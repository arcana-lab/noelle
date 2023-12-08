/*
 * Copyright 2016 - 2022  Kevin McAfee, Angelo Matni, Simone Campanoni
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
#ifndef NOELLE_SRC_TOOLS_PARALLELIZATION_PLANNER_PLANNER_H_
#define NOELLE_SRC_TOOLS_PARALLELIZATION_PLANNER_PLANNER_H_
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/LoopDependenceInfo.hpp"
#include "noelle/core/PDG.hpp"
#include "noelle/core/SCC.hpp"
#include "noelle/core/SCCDAG.hpp"
#include "noelle/core/Noelle.hpp"
#include "noelle/core/MetadataManager.hpp"
#include "noelle/tools/DOALL.hpp"

namespace arcana::noelle {

class Planner : public ModulePass {
public:
  Planner();

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

  std::vector<LoopDependenceInfo *> getLoopsToParallelize(Module &M,
                                                          Noelle &par);

  void removeLoopsNotWorthParallelizing(Noelle &noelle,
                                        Hot *profiles,
                                        LoopForest *f);

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

#endif // NOELLE_SRC_TOOLS_PARALLELIZATION_PLANNER_PLANNER_H_
