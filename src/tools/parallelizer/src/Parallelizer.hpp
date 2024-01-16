/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
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
#ifndef NOELLE_SRC_TOOLS_PARALLELIZER_H_
#define NOELLE_SRC_TOOLS_PARALLELIZER_H_

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/LoopContent.hpp"
#include "noelle/core/PDG.hpp"
#include "noelle/core/SCC.hpp"
#include "noelle/core/SCCDAG.hpp"
#include "noelle/core/Noelle.hpp"
#include "noelle/core/MetadataManager.hpp"
#include "noelle/tools/DOALL.hpp"
#include "noelle/tools/HELIX.hpp"
#include "HeuristicsPass.hpp"
#include "noelle/tools/DSWP.hpp"

namespace arcana::noelle {

class Parallelizer : public ModulePass {
public:
  Parallelizer();

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
  bool forceNoSCCPartition;
  std::vector<int> loopIndexesWhiteList;
  std::vector<int> loopIndexesBlackList;

  /*
   * Methods
   */
  bool parallelizeLoop(LoopContent *loopContent, Noelle &par, Heuristics *h);

  bool parallelizeLoops(Noelle &noelle, Heuristics *heuristics);

  std::vector<LoopContent *> getLoopsToParallelize(Module &M, Noelle &par);

  bool collectThreadPoolHelperFunctionsAndTypes(Module &M, Noelle &par);
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_TOOLS_PARALLELIZER_H_
