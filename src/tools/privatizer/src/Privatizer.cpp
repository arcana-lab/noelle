/*
 * Copyright 2023 Xiao Chen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Privatizer.hpp"
#include "Utils.hpp"

namespace arcana::noelle {

FunctionSummary::FunctionSummary(Function *currentF) : currentF(currentF) {
  for (auto &bb : *currentF) {
    for (auto &inst : bb) {
      if (isa<StoreInst>(inst)) {
        auto storeInst = dyn_cast<StoreInst>(&inst);
        storeInsts.insert(storeInst);
      } else if (isa<AllocaInst>(inst)) {
        auto allocaInst = dyn_cast<AllocaInst>(&inst);
        allocaInsts.insert(allocaInst);
        stackMemoryUsage += getAllocationSize(allocaInst);
      } else if (isa<CallBase>(inst)) {
        auto callInst = dyn_cast<CallBase>(&inst);
        auto calleeFunc = callInst->getCalledFunction();
        if (calleeFunc && calleeFunc->getName() == "malloc") {
          mallocInsts.insert(callInst);
        } else if (calleeFunc && calleeFunc->getName() == "calloc") {
          callocInsts.insert(callInst);
        } else if (calleeFunc && calleeFunc->getName() == "free") {
          freeInsts.insert(callInst);
        } else if (isa<MemCpyInst>(callInst)) {
          destsOfMemcpy.insert(callInst->getArgOperand(0));
        }
      }
    }
  }
}

bool FunctionSummary::stackCanHoldNewAlloca(uint64_t allocationSize) {
  if ((stackMemoryUsage + allocationSize) < STACK_SIZE_THRESHOLD) {
    stackMemoryUsage += allocationSize;
    return true;
  } else {
    return false;
  }
}

bool FunctionSummary::isDestOfMemcpy(Value *ptr) {
  return destsOfMemcpy.find(ptr) != destsOfMemcpy.end();
}

Privatizer::Privatizer() : ModulePass{ ID } {
  return;
}

bool Privatizer::runOnModule(Module &M) {

  /*
   * Check if enablers have been enabled.
   */
  if (!this->enablePrivatizer) {
    return false;
  }

  /*
   * Fetch NOELLE.
   */
  auto &noelle = getAnalysis<NoellePass>().getNoelle();
  mpa = noelle.getMayPointsToAnalysis();

  auto modified = false;

  auto h2s = collectH2S(noelle);
  auto g2s = collectG2S(noelle);

  for (auto &[f, liveMemSum] : h2s) {
    modified |= transformH2S(noelle, liveMemSum);
  }
  for (auto &[globalVar, privariableFunctions] : g2s) {
    modified |= transformG2S(noelle, globalVar, privariableFunctions);
  }

  return modified;
}

FunctionSummary *Privatizer::getFunctionSummary(Function *f) {
  if (functionSummaries.find(f) == functionSummaries.end()) {
    functionSummaries[f] = new FunctionSummary(f);
  }
  return functionSummaries[f];
}

void Privatizer::clearFunctionSummaries() {
  for (auto &[f, summary] : functionSummaries) {
    delete summary;
  }
  functionSummaries.clear();
}

} // namespace arcana::noelle
