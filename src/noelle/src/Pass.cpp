/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Noelle.hpp"
#include "PDGAnalysis.hpp"
#include "HotProfiler.hpp"

using namespace llvm;

static cl::opt<int> Verbose("noelle-verbose", cl::ZeroOrMore, cl::Hidden, cl::desc("Verbose output (0: disabled, 1: minimal, 2: maximal)"));
static cl::opt<int> MinimumHotness("noelle-min-hot", cl::ZeroOrMore, cl::Hidden, cl::desc("Minimum hotness of code to be parallelized"));
static cl::opt<int> MaximumCores("noelle-max-cores", cl::ZeroOrMore, cl::Hidden, cl::desc("Maximum number of logical cores that Noelle can use"));
static cl::opt<bool> DisableDSWP("noelle-disable-dswp", cl::ZeroOrMore, cl::Hidden, cl::desc("Disable DSWP"));
static cl::opt<bool> DisableHELIX("noelle-disable-helix", cl::ZeroOrMore, cl::Hidden, cl::desc("Disable HELIX"));
static cl::opt<bool> DisableDOALL("noelle-disable-doall", cl::ZeroOrMore, cl::Hidden, cl::desc("Disable DOALL"));
static cl::opt<bool> DisableInliner("noelle-disable-inliner", cl::ZeroOrMore, cl::Hidden, cl::desc("Disable the function inliner"));

bool Noelle::doInitialization (Module &M) {

  /*
   * Fetch the command line options.
   */
  this->verbose = static_cast<Verbosity>(Verbose.getValue());
  this->minHot = ((double)(MinimumHotness.getValue())) / 100;
  auto optMaxCores = MaximumCores.getValue();
  if (optMaxCores > 0){
    this->maxCores = optMaxCores;
  }
  if (DisableDOALL.getNumOccurrences() == 0){
    this->enabledTransformations.insert(DOALL_ID);
  }
  if (DisableDSWP.getNumOccurrences() == 0){
    this->enabledTransformations.insert(DSWP_ID);
  }
  if (DisableHELIX.getNumOccurrences() == 0){
    this->enabledTransformations.insert(HELIX_ID);
  }
  if (DisableInliner.getNumOccurrences() == 0){
    this->enabledTransformations.insert(INLINER_ID);
  }

  /*
   * Store the module.
   */
  this->program = &M;

  int1 = IntegerType::get(M.getContext(), 1);
  int8 = IntegerType::get(M.getContext(), 8);
  int16 = IntegerType::get(M.getContext(), 16);
  int32 = IntegerType::get(M.getContext(), 32);
  int64 = IntegerType::get(M.getContext(), 64);

  return false;
}

void Noelle::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<CallGraphWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<AssumptionCacheTracker>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<PDGAnalysis>();
  AU.addRequired<HotProfiler>();

  return ;
}

bool Noelle::runOnModule (Module &M){
  return false;
}

// Next there is code to register your pass to "opt"
char Noelle::ID = 0;
static RegisterPass<Noelle> X("parallelization", "Computing the Program Dependence Graph");

// Next there is code to register your pass to "clang"
static Noelle * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new Noelle());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new Noelle());}});// ** for -O0
