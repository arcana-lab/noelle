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
    auto stripped = strip(ptr);
    if (isa<Instruction>(stripped)) {
      return dyn_cast<Instruction>(stripped)->getFunction();
    } else if (isa<Argument>(stripped)) {
      return dyn_cast<Argument>(stripped)->getParent();
    } else {
      return nullptr;
    }
  };

  auto func1 = getOwnerFunction(ptr1);
  auto func2 = getOwnerFunction(ptr2);

  if (func1 == nullptr && func2 == nullptr) {
    if (isa<GlobalVariable>(ptr1) && isa<GlobalVariable>(ptr2)
        && (ptr1 != ptr2)) {
      return false;
    } else {
      return true;
    }
  } else if (func1 != nullptr && func2 == nullptr) {
    auto funcSum = getFunctionSummary(func1);
    return funcSum->getPointees(ptr1).count(nullptr) > 0;
  } else if (func1 == nullptr && func2 != nullptr) {
    auto funcSum = getFunctionSummary(func2);
    return funcSum->getPointees(ptr2).count(nullptr) > 0;
  } else if (func1 == func2) {
    auto funcSum = getFunctionSummary(func1);
    auto ptes1 = funcSum->getPointees(ptr1);
    auto ptes2 = funcSum->getPointees(ptr2);
    ptes1.erase(ptes2.begin(), ptes2.end());
    return !ptes1.empty();
  } else {
    auto funcSum1 = getFunctionSummary(func1);
    auto funcSum2 = getFunctionSummary(func2);
    return funcSum1->getPointees(ptr1).count(nullptr) > 0
           || funcSum2->getPointees(ptr2).count(nullptr) > 0;
  }
}

bool MayPointsToAnalysis::mayEscape(Instruction *inst) {
  auto currentF = inst->getFunction();
  auto funcSum = getFunctionSummary(currentF);
  return funcSum->mayBePointedByUnknown(inst);
}

bool MayPointsToAnalysis::notPrivatizable(GlobalVariable *globalVar,
                                          Function *currentF) {
  auto funcSum = getFunctionSummary(currentF);
  return funcSum->mayBePointedByUnknown(globalVar);
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
