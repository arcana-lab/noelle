/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "HELIX.hpp"

using namespace llvm ;

void HELIX::spillLoopCarriedDataDependencies (LoopDependenceInfo *LDI) {
  std::set<PHINode *> loopCarriedPHIs;

  /*
   * Collect all PHIs in the loop header; they are local variables
   * with loop carried data dependencies and need to be spilled
   */
  Instruction *headerPHI = &*LDI->header->begin();
  while (isa<PHINode>(headerPHI)) {
    loopCarriedPHIs.insert((PHINode *)headerPHI);
    headerPHI = headerPHI->getNextNode();
  }
  assert(loopCarriedPHIs.size() > 0
    && "There should be loop carried data dependencies for a HELIX loop");

  /*
   * Spill each loop carried PHI
   */
  IRBuilder<> entryBuilder(&*LDI->function->begin()->begin());
  IRBuilder<> headerBuilder(headerPHI->getPrevNode());
  for (auto phi : loopCarriedPHIs) {
    auto alloca = entryBuilder.CreateAlloca(phi->getType());

    /*
     * Store loop carried and initial values onto the stack
     */
    for (auto i = 0; i < phi->getNumIncomingValues(); ++i) {
      auto terminator = phi->getIncomingBlock(i)->getTerminator();
      IRBuilder<> builder(terminator);
      builder.CreateStore(phi->getIncomingValue(i), alloca);
      terminator->removeFromParent();
      builder.Insert(terminator);
    }

    /*
     * Load from the stack; replace uses of the PHI with the load
     */
    auto load = headerBuilder.CreateLoad(alloca);
    for (auto user : phi->users()) {
      user->replaceUsesOfWith(phi, load);
    }
    phi->removeFromParent();
  }
}
