/*
 * Copyright 2019 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopStats.hpp"

using namespace llvm;
using namespace llvm::noelle;

bool LoopStats::doInitialization(Module &M) {
  return false;
}

void LoopStats::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<Noelle>();
  return;
}

bool LoopStats::runOnModule(Module &M) {

  /*
   * Fetch noelle.
   */
  auto& noelle = getAnalysis<Noelle>();
  if (noelle.getVerbosity() > Verbosity::Disabled) {
    errs() << "LoopStats: Start\n";
  }

  /*
   * Fetch all program loops.
   */
  auto programLoops = noelle.getLoops();

  /*
   * Analyze the loops.
   */
  this->collectStatsForLoops(noelle, *programLoops);

  /*
   * Free the memory.
   */
  delete programLoops ;

  if (noelle.getVerbosity() > Verbosity::Disabled) {
    errs() << "LoopStats: Exit\n";
  }

  return false;
}

// Next there is code to register your pass to "opt"
char LoopStats::ID = 0;
static RegisterPass<LoopStats> X("LoopStats", "Generate statistics output for loops using LDI");

// Next there is code to register your pass to "clang"
static LoopStats * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new LoopStats());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new LoopStats()); }}); // ** for -O0
