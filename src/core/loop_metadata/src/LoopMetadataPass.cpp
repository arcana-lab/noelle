/*
 * Copyright 2019 - 2024  Simone Campanoni
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
#include "llvm/Analysis/LoopInfo.h"
#include "LoopMetadataPass.hpp"

namespace arcana::noelle {

LoopMetadataPass::LoopMetadataPass() : ModulePass(ID) {

  return;
}

bool LoopMetadataPass::doInitialization(Module &M) {
  return false;
}

bool LoopMetadataPass::runOnModule(Module &M) {
  bool modified = false;

  /*
   * Fetch all the loops of the program.
   */
  auto loopStructures = this->getLoopStructuresWithoutNoelle(M);

  /*
   * Set loop ID metadata
   */
  modified |= this->setIDs(loopStructures);

  return modified;
}

void LoopMetadataPass::getAnalysisUsage(AnalysisUsage &AU) const {

  /*
   * Analyses.
   */
  AU.addRequired<LoopInfoWrapperPass>();

  return;
}

// Next there is code to register your pass to "opt"
char LoopMetadataPass::ID = 0;
static RegisterPass<LoopMetadataPass> X("LoopMetadata",
                                        "Adding metadata to loops");

} // namespace arcana::noelle
