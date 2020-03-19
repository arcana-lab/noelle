/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "EnablersManager.hpp"
#include "HotProfiler.hpp"
#include "LoopDistribution.hpp"
#include <unordered_map>

using namespace llvm;

//extern cl::opt<int> Verbose("noelle-verbose", cl::ZeroOrMore, cl::Hidden, cl::desc("Verbose output (0: disabled, 1: minimal, 2: maximal)"));
//extern cl::opt<int> MinimumHotness("noelle-min-hot", cl::ZeroOrMore, cl::Hidden, cl::desc("Minimum hotness of code to be parallelized"));

bool EnablersManager::doInitialization (Module &M) {
  //this->minHot = ((double)(MinimumHotness.getValue())) / 100;
  //this->verbose = static_cast<Verbosity>(Verbose.getValue());
  this->minHot = 0;
  this->verbose = static_cast<Verbosity>(3);

  return false; 
}

void EnablersManager::getAnalysisUsage (AnalysisUsage &AU) const {

  /*
   * Analyses.
   */
  AU.addRequired<PDGAnalysis>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();

  /*
   * Parallelizations.
   */
  AU.addRequired<Parallelization>();

  /*
   * Parallelization enablers.
   */
  AU.addRequired<LoopDistribution>();

  /*
   * Profilers.
   */
  AU.addRequired<HotProfiler>();

  return ;
}

// Next there is code to register your pass to "opt"
char llvm::EnablersManager::ID = 0;
static RegisterPass<EnablersManager> X("enablers", "Transformations designed to enable automatic parallelization of sequential code");

// Next there is code to register your pass to "clang"
static EnablersManager * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new EnablersManager());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new EnablersManager());}});// ** for -O0
