/*
 * Copyright 2019 - 2020 Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "EnablersManager.hpp"
#include "HotProfiler.hpp"
#include "LoopDistribution.hpp"
#include "LoopUnroll.hpp"
#include <unordered_map>

using namespace llvm;

static cl::opt<bool> DisableEnablers("noelle-disable-enablers", cl::ZeroOrMore, cl::Hidden, cl::desc("Disable all enablers"));

namespace llvm::noelle {
  bool EnablersManager::doInitialization (Module &M) {
    this->enableEnablers = (DisableEnablers.getNumOccurrences() == 0) ? true : false;

    return false; 
  }

  void EnablersManager::getAnalysisUsage (AnalysisUsage &AU) const {

    /*
    * Analysis needed by this pass.
    */
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<AssumptionCacheTracker>();

    /*
    * Noelle framework.
    */
    AU.addRequired<Noelle>();

    return ;
  }
}

// Next there is code to register your pass to "opt"
char llvm::noelle::EnablersManager::ID = 0;
static RegisterPass<EnablersManager> X("enablers", "Transformations designed to enable automatic parallelization of sequential code", false, false);

// Next there is code to register your pass to "clang"
static EnablersManager * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new EnablersManager());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new EnablersManager());}});// ** for -O0
