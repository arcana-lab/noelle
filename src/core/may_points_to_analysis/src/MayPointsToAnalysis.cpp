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
#include "noelle/core/MayPointsToAnalysis.hpp"
#include "MpaUtils.hpp"

namespace llvm::noelle {

MayPointsToAnalysis::MayPointsToAnalysis() {}

bool MayPointsToAnalysis::mayAlias(Value *ptr1, Value *ptr2) {
  assert(ptr1->getType()->isPointerTy() && ptr2->getType()->isPointerTy());

  auto getOwnerFunction = [](Value *ptr) -> Function * {
    if (isa<Instruction>(ptr)) {
      return dyn_cast<Instruction>(ptr)->getFunction();
    } else if (isa<Argument>(ptr)) {
      return dyn_cast<Argument>(ptr)->getParent();
    } else {
      return nullptr;
    }
  };

  auto stripped1 = strip(ptr1);
  auto stripped2 = strip(ptr2);

  auto func1 = getOwnerFunction(stripped1);
  auto func2 = getOwnerFunction(stripped2);

  if (func1 == nullptr && func2 == nullptr) {
    if (isa<GlobalVariable>(stripped1) && isa<GlobalVariable>(stripped2)
        && (stripped1 != stripped2)) {
      return false;
    } else {
      return true;
    }
  } else if (func1 != nullptr && func2 == nullptr) {
    auto funcSum = getFunctionSummary(func1);
    funcSum->doMayPointsToAnalysis();
    return funcSum->getPointeeMemobjs(stripped1).count(nullptr) > 0;
  } else if (func1 == nullptr && func2 != nullptr) {
    auto funcSum = getFunctionSummary(func2);
    funcSum->doMayPointsToAnalysis();
    return funcSum->getPointeeMemobjs(stripped2).count(nullptr) > 0;
  } else if (func1 == func2) {
    auto funcSum = getFunctionSummary(func1);
    funcSum->doMayPointsToAnalysis();
    auto ptes1 = funcSum->getPointeeMemobjs(stripped1);
    auto ptes2 = funcSum->getPointeeMemobjs(stripped2);
    for (auto pte1 : ptes1) {
      for (auto pte2 : ptes2) {
        if (pte1 == pte2) {
          return true;
        }
      }
    }
    return false;
  } else {
    return true;
  }
}

bool MayPointsToAnalysis::mayEscape(Instruction *inst) {
  assert(isAllocation(inst));
  auto currentF = inst->getFunction();
  auto funcSum = getFunctionSummary(currentF);
  funcSum->doMayPointsToAnalysis();
  return funcSum->mayBePointedByUnknown(inst)
         || funcSum->mayBePointedByReturnValue(inst);
}

bool MayPointsToAnalysis::notPrivatizable(GlobalVariable *globalVar,
                                          Function *currentF) {
  auto funcSum = getFunctionSummary(currentF);
  funcSum->doMayPointsToAnalysisFor(globalVar);

  auto result = funcSum->mayBePointedByUnknown(globalVar)
                || funcSum->mayBePointedByReturnValue(globalVar);
  funcSum->clearPointsToSummary();
  return result;
}

bool MayPointsToAnalysis::mayAccessEscapedMemobj(Instruction *inst) {
  if (!isa<LoadInst>(inst) && !isa<StoreInst>(inst)) {
    return true;
  }

  auto currentF = inst->getFunction();
  auto funcSum = getFunctionSummary(currentF);

  Value *ptr;
  if (isa<LoadInst>(inst)) {
    ptr = dyn_cast<LoadInst>(inst)->getPointerOperand();
  } else if (isa<StoreInst>(inst)) {
    ptr = dyn_cast<StoreInst>(inst)->getPointerOperand();
  }

  funcSum->doMayPointsToAnalysis();
  auto pointees = funcSum->getReachableMemobjs(ptr);
  for (auto memobj : pointees) {
    if (memobj == nullptr || funcSum->mayBePointedByUnknown(memobj)) {
      return true;
    }
  }

  return false;
}

std::unordered_set<Value *> MayPointsToAnalysis::getPointees(
    Value *ptr,
    Function *currentF) {
  assert(ptr->getType()->isPointerTy());
  auto funcSum = getFunctionSummary(currentF);
  funcSum->doMayPointsToAnalysis();
  return funcSum->getPointeeMemobjs(ptr);
}

MayPointsToAnalysis::~MayPointsToAnalysis() {
  for (auto &[f, funcSum] : functionSummaries) {
    delete funcSum;
  }
  functionSummaries.clear();
}

MpaSummary *MayPointsToAnalysis::getFunctionSummary(Function *currentF) {
  if (functionSummaries.find(currentF) == functionSummaries.end()) {
    functionSummaries[currentF] = new MpaSummary(currentF);
  }
  return functionSummaries[currentF];
}

} // namespace llvm::noelle
