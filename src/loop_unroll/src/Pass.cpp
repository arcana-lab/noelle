/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopUnroll.hpp"
#include "PDGAnalysis.hpp"
#include "HotProfiler.hpp"

using namespace llvm;

LoopUnroll::LoopUnroll()
  :
  ModulePass{ID}
  {
  return ;
}

bool LoopUnroll::doInitialization (Module &M) {
  return true;
}

bool LoopUnroll::runOnModule (Module &M) {
  return false;
}

void LoopUnroll::getAnalysisUsage (AnalysisUsage &AU) const {
  AU.setPreservesAll();
  return ;
}

// Next there is code to register your pass to "opt"
char llvm::LoopUnroll::ID = 0;
static RegisterPass<LoopUnroll> X("LoopUnroll", "Loop distribution");

// Next there is code to register your pass to "clang"
static LoopUnroll * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_ModuleOptimizerEarly,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new LoopUnroll());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new LoopUnroll());}});// ** for -O0
