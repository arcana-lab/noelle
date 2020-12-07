/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"
#include "Hot.hpp"

using namespace llvm;
using namespace llvm::noelle;

void Hot::setBasicBlockInvocations (BasicBlock *bb, uint64_t invocations){

  /*
   * Check if bb is the entry point of a function.
   */
  auto f = bb->getParent();
  auto& entryBB = f->getEntryBlock();
  if (&entryBB == bb){

    /*
     * Insert the number of invocations of a function.
     */
    this->functionInvocations[f] = invocations;
  }

  /*
   * Insert the number of invocations of the basic block bb.
   */
  this->bbInvocations[bb] = invocations;

  return ;
}
      
bool Hot::hasBeenExecuted (BasicBlock *bb) const {
  if (this->getInvocations(bb) == 0){
    return false;
  }

  return true;
}

uint64_t Hot::getInvocations (BasicBlock *bb) const {
  auto inv = this->bbInvocations.at(bb);

  return inv;
}
      
uint64_t Hot::getSelfInstructions (BasicBlock *bb) const {
  auto invs = this->getInvocations(bb);

  return invs * this->getStaticInstructions(bb);
}

uint64_t Hot::getTotalInstructions (BasicBlock *bb) const {
  uint64_t t = 0;

  for (auto &inst: *bb){
    t += this->getTotalInstructions(&inst);
  }

  return t;
}
      
uint64_t Hot::getStaticInstructions (BasicBlock *bb) const {
  auto inv = this->getInvocations(bb);
  
  auto bbLength = std::distance(bb->begin(), bb->end());

  return inv * bbLength;
}

double Hot::getBranchFrequency (BasicBlock *sourceBB, BasicBlock *targetBB) const {
  auto &branchSuccessors = this->branchProbability.at(sourceBB);
  
  /*
   * Check if we have information about the branch.
   */
  if (branchSuccessors.find(targetBB) == branchSuccessors.end()){

    /* 
     * We do not have information about the branch.
     */
    return 0;
  }

  /*
   * We have information about the branch.
   *
   * Fetch the frequency.
   */
  auto f = branchSuccessors.at(targetBB);

  return f;
}
      
void Hot::setBranchFrequency (BasicBlock *src, BasicBlock *dst, double branchFrequency){
  auto &branchSuccessors = this->branchProbability[src];
  branchSuccessors[dst] = branchFrequency;

  return ;
}
