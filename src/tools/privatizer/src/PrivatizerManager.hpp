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
#pragma once

#include "noelle/core/Noelle.hpp"
#include <unordered_set>

namespace llvm::noelle {

class LiveMemorySummary {
public:
  std::unordered_set<CallBase *> allocable;
  std::unordered_set<CallBase *> removable;
};

class PrivatizerManager : public ModulePass {
public:
  static char ID;

  // PrivatizerManager.cpp
  PrivatizerManager();

  bool doInitialization(Module &M) override;

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  bool enablePrivatizer;

  const uint64_t STACK_SIZE_THRESHOLD = 8 * 1024 * 1024;

  const string prefix = "PrivatizerManager: ";

  const string emptyPrefix = "                   ";

  unordered_map<Function *, uint64_t> stackMemoryUsages;

  // PrivatizerUtils.cpp
  bool isFixedSizedHeapAllocation(CallBase *heapAllocInst);

  uint64_t getAllocationSize(Value *allocationSource);

  void setStackMemoryUsage(PointToSummary *ptSum);

  bool stackHasEnoughSpaceForNewAllocaInst(uint64_t allocationSize,
                                           Function *currentF);

  // HeapToStack.cpp
  unordered_map<Function *, LiveMemorySummary> collectHeapToStack(
      Noelle &noelle,
      PointToSummary *ptSum);

  bool applyHeapToStack(Noelle &noelle, LiveMemorySummary liveMemSum);

  LiveMemorySummary getLiveMemorySummary(Noelle &noelle,
                                         PointToSummary *ptSum,
                                         FunctionSummary *funcSum);

  // GlobalToStack.cpp
  unordered_map<GlobalVariable *, unordered_set<Function *>>
  collectGlobalToStack(Noelle &noelle, PointToSummary *ptSum);

  bool applyGlobalToStack(Noelle &noelle,
                          GlobalVariable *globalVar,
                          unordered_set<Function *> privatizableFunctions);

  unordered_set<Function *> getPrivatizableFunctions(Noelle &noelle,
                                                     PointToSummary *ptSum,
                                                     GlobalVariable *globalVar);

  Instruction *getProgramPointOfInitilization(Noelle &noelle,
                                              GlobalVariable *globalVar,
                                              StoreInst *storeInst);

  bool globalVariableInitializedInFunction(Noelle &noelle,
                                           PointToSummary *ptSum,
                                           GlobalVariable *globalVar,
                                           Function *currentF);
};

} // namespace llvm::noelle
