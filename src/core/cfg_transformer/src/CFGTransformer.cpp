/*
 * Copyright 2021 - 2024  Simone Campanoni
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
#include "arcana/noelle/core/CFGTransformer.hpp"

namespace arcana::noelle {

CFGTransformer::CFGTransformer() {
  return;
}

BasicBlock *CFGTransformer::branchToANewBasicBlockAndBack(
    Instruction *splitPoint,
    std::string newBasicBlockName,
    std::string joinBasicBlockName,
    std::function<void(BasicBlock *newBB, BasicBlock *newJoinBB)>
        addConditionalBranch) {

  /*
   * Fetch the function
   */
  auto f = splitPoint->getFunction();

  /*
   * Create a new basic block
   */
  auto newBB = BasicBlock::Create(f->getContext(), newBasicBlockName, f);

  /*
   * Perform the transformation.
   */
  this->branchToTargetBasicBlockAndBack(splitPoint,
                                        *newBB,
                                        joinBasicBlockName,
                                        addConditionalBranch);

  return newBB;
}

void CFGTransformer::branchToTargetBasicBlockAndBack(
    Instruction *splitPoint,
    BasicBlock &targetBB,
    std::string joinBasicBlockName,
    std::function<void(BasicBlock *newBB, BasicBlock *newJoinBB)>
        addConditionalBranch) {

  /*
   * Split the last basic block to inject the condition to jump to the new last
   * basic block
   */
  IRBuilder<> newBBBuilder(&targetBB);
  auto bb = splitPoint->getParent();
  auto newLastBB = bb->splitBasicBlock(splitPoint, joinBasicBlockName);
  assert(newLastBB != nullptr);
  newBBBuilder.CreateBr(newLastBB);
  auto newTerm = bb->getTerminator();
  assert(newTerm != nullptr);
  newTerm->eraseFromParent();

  /*
   * Add the conditional branch
   */
  addConditionalBranch(&targetBB, newLastBB);

  return;
}

} // namespace arcana::noelle
