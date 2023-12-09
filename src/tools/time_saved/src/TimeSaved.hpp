/*
 * Copyright 2022  Kevin McAfee, Simone Campanoni
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
#ifndef NOELLE_SRC_TOOLS_TIME_SAVED_TIMESAVED_H_
#define NOELLE_SRC_TOOLS_TIME_SAVED_TIMESAVED_H_
#pragma once

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DerivedUser.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/IRBuilder.h"

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/LoopDependenceInfo.hpp"
#include "noelle/core/PDG.hpp"
#include "noelle/core/SCC.hpp"
#include "noelle/core/SCCDAG.hpp"
#include "noelle/core/Noelle.hpp"
#include "noelle/core/MetadataManager.hpp"
#include "noelle/tools/DOALL.hpp"

namespace arcana::noelle {

class TimeSaved : public ModulePass {
public:
  TimeSaved();

  bool doInitialization(Module &M) override;

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  /*
   * Class fields
   */
  static char ID;

private:
  /*
   * Fields
   */
  bool forceParallelization;

  /*
   * Methods
   */

  std::vector<LoopDependenceInfo *> selectTheOrderOfLoopsToParallelize(
      Noelle &noelle,
      Hot *profiles,
      noelle::LoopTree *tree,
      uint64_t &maxTimeSaved,
      uint64_t &maxTimeSavedWithDOALLOnly);

  std::pair<uint64_t, uint64_t> evaluateSavings(
      Noelle &noelle,
      noelle::LoopTree *tree,
      const std::map<LoopStructure *, uint64_t> &timeSaved,
      const std::map<LoopStructure *, bool> &doallLoops);

  uint64_t evaluateSavings(Noelle &noelle,
                           noelle::LoopTree *tree,
                           const std::map<LoopStructure *, uint64_t> &timeSaved,
                           std::function<bool(LoopStructure *)> considerLoop);
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_TOOLS_TIME_SAVED_TIMESAVED_H_
