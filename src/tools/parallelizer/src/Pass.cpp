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
#include "Parallelizer.hpp"

namespace arcana::noelle {

/*
 * Options of the Parallelizer pass.
 */
static cl::opt<bool> ForceParallelization(
    "noelle-parallelizer-force",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Force the parallelization"));
static cl::opt<bool> ForceNoSCCPartition(
    "dswp-no-scc-merge",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Force no SCC merging when parallelizing"));
static cl::list<int> LoopIndexesWhiteList(
    "noelle-loops-white-list",
    cl::ZeroOrMore,
    cl::CommaSeparated,
    cl::desc("Parallelize only a subset of loops"));
static cl::list<int> LoopIndexesBlackList(
    "noelle-loops-black-list",
    cl::ZeroOrMore,
    cl::CommaSeparated,
    cl::desc("Don't parallelize a subset of loops"));

Parallelizer::Parallelizer()
  : ModulePass{ ID },
    forceParallelization{ false },
    forceNoSCCPartition{ false } {

  return;
}

bool Parallelizer::doInitialization(Module &M) {
  this->forceParallelization = (ForceParallelization.getNumOccurrences() > 0);
  this->forceNoSCCPartition = (ForceNoSCCPartition.getNumOccurrences() > 0);
  this->loopIndexesWhiteList = LoopIndexesWhiteList;
  this->loopIndexesBlackList = LoopIndexesBlackList;

  return false;
}

bool Parallelizer::runOnModule(Module &M) {
  errs() << "Parallelizer: Start\n";

  /*
   * Fetch the outputs of the passes we rely on.
   */
  auto &noelle = getAnalysis<Noelle>();
  auto heuristics = getAnalysis<HeuristicsPass>().getHeuristics(noelle);

  /*
   * Parallelize the loops of the target program.
   */
  auto modified = this->parallelizeLoops(noelle, heuristics);

  return modified;
}

void Parallelizer::getAnalysisUsage(AnalysisUsage &AU) const {

  /*
   * Noelle.
   */
  AU.addRequired<Noelle>();
  AU.addRequired<HeuristicsPass>();
}

} // namespace arcana::noelle

// Next there is code to register your pass to "opt"
char arcana::noelle::Parallelizer::ID = 0;
static RegisterPass<arcana::noelle::Parallelizer> X(
    "parallelizer",
    "Automatic parallelization of sequential code");

// Next there is code to register your pass to "clang"
static arcana::noelle::Parallelizer *_PassMaker = NULL;
static RegisterStandardPasses _RegPass1(
    PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
      if (!_PassMaker) {
        PM.add(_PassMaker = new arcana::noelle::Parallelizer());
      }
    }); // ** for -Ox
static RegisterStandardPasses _RegPass2(
    PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
      if (!_PassMaker) {
        PM.add(_PassMaker = new arcana::noelle::Parallelizer());
      }
    }); // ** for -O0
