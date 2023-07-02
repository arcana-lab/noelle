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
#include "PrivatizerManager.hpp"
#include "llvm/IR/GlobalVariable.h"
#include <numeric>

namespace llvm::noelle {

bool PrivatizerManager::isFixedSizedHeapAllocation(CallBase *heapAllocInst) {
  if (getCalledFuncName(heapAllocInst) == "malloc") {
    if (dyn_cast<ConstantInt>(heapAllocInst->getOperand(0))) {
      return true;
    }
  } else if (getCalledFuncName(heapAllocInst) == "calloc") {
    if (dyn_cast<ConstantInt>(heapAllocInst->getOperand(0))
        && dyn_cast<ConstantInt>(heapAllocInst->getOperand(1))) {
      return true;
    }
  }
  return false;
}

uint64_t PrivatizerManager::getAllocationSize(Value *allocationSource) {
  if (isa<CallBase>(allocationSource)) {
    auto heapAllocInst = dyn_cast<CallBase>(allocationSource);
    if (isFixedSizedHeapAllocation(heapAllocInst)) {
      auto dl = heapAllocInst->getModule()->getDataLayout();
      if (getCalledFuncName(heapAllocInst) == "malloc") {
        return dyn_cast<ConstantInt>(heapAllocInst->getOperand(0))
            ->getZExtValue();
      } else if (getCalledFuncName(heapAllocInst) == "calloc") {
        auto elementCount =
            dyn_cast<ConstantInt>(heapAllocInst->getOperand(0))->getZExtValue();
        auto elementSizeInBytes =
            dyn_cast<ConstantInt>(heapAllocInst->getOperand(1))->getZExtValue();
        return elementCount * elementSizeInBytes;
      }
    }
  } else if (isa<GlobalVariable>(allocationSource)) {
    auto globalVar = dyn_cast<GlobalVariable>(allocationSource);
    auto globalVarType = globalVar->getValueType();
    auto dl = globalVar->getParent()->getDataLayout();
    return dl.getTypeAllocSize(globalVarType);
  }
  assert(
      false
      && "Unsupported allocation source: not a fixed-sized heap allocation or a global variable.");
};

void PrivatizerManager::setStackMemoryUsage(PointToSummary *ptSum) {
  stackMemoryUsages.clear();
  for (auto &[_, funcSum] : ptSum->functionSummaries) {
    auto dl = funcSum->currentF->getParent()->getDataLayout();
    auto stackMemoryUsage = std::accumulate(
        funcSum->allocaInsts.begin(),
        funcSum->allocaInsts.end(),
        0,
        [&](uint64_t previous, AllocaInst *item) -> uint64_t {
          return previous + (item->getAllocationSizeInBits(dl).getValue() / 8);
        });
    stackMemoryUsages[funcSum->currentF] = stackMemoryUsage;
  }
}

bool PrivatizerManager::stackHasEnoughSpaceForNewAllocaInst(
    uint64_t allocationSize,
    Function *currentF) {
  auto stackMemoryUsage = stackMemoryUsages.at(currentF);
  if ((stackMemoryUsage + allocationSize) < STACK_SIZE_THRESHOLD) {
    stackMemoryUsages[currentF] = stackMemoryUsage + allocationSize;
    return true;
  } else {
    return false;
  }
}

} // namespace llvm::noelle
