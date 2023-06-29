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

  PrivatizerManager();

  bool doInitialization(Module &M) override;

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  bool enablePrivatizer;

  const uint64_t STACK_SIZE_THRESHOLD = 8 * 1024 * 1024;

  LiveMemorySummary getLiveMemorySummary(PointToSummary *ptSum,
                                         FunctionSummary *funcSum);

  bool applyHeapToStack(PointToSummary *ptSum, FunctionSummary *funcSum);

  bool applyGlobalToStack(FunctionSummary *funcSum);

  bool canBeClonedToStack(GlobalVariable *globalVar, FunctionSummary *funcSum);
};

} // namespace llvm::noelle