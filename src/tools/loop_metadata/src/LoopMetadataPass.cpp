/*
 * Copyright 2019 - 2020 Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SystemHeaders.hpp"
#include "LoopMetadataPass.hpp"

using namespace llvm;
using namespace llvm::noelle;

LoopMetadataPass::LoopMetadataPass()
  :
  ModulePass(ID)
  {

  return ;
}

bool LoopMetadataPass::doInitialization (Module &M) {
  return false;
}

bool LoopMetadataPass::runOnModule (Module &M) {

  /*
   * Fetch the outputs of the passes we rely on.
   */
  auto& parallelizationFramework = getAnalysis<Noelle>();

  /*
   * Fetch the context
   */
  auto& context = M.getContext();

  /*
   * Tag all loops of the function given as input.
   *
   * Fetch the result of loop identification analysis.
   */
  auto modified = false;

  /*
   * Tag the loops of the current function.
   */
  modified |= this->tagLoops(context, M, parallelizationFramework);

  return modified;
}

void LoopMetadataPass::getAnalysisUsage (AnalysisUsage &AU) const {

  /*
   * Analyses.
   */
  AU.addRequired<Noelle>();

  return ;
}

// Next there is code to register your pass to "opt"
char LoopMetadataPass::ID = 0;
static RegisterPass<LoopMetadataPass> X("LoopMetadata", "Adding metadata to loops");

// Next there is code to register your pass to "clang"
static LoopMetadataPass * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new LoopMetadataPass());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new LoopMetadataPass()); }}); // ** for -O0
