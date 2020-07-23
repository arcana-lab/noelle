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

using namespace llvm ;
      
uint64_t Hot::getInvocations (LoopStructure *l) const {

  /*
   * Fetch the pre-header.
   */
  auto preH = l->getPreHeader();

  /*
   * Fetch the number of invocations of the preheader. 
   * This is the same as the invocations of the loop.
   */
  auto preHInvocations = this->getInvocations(preH);

  return preHInvocations;
}

uint64_t Hot::getSelfInstructions (LoopStructure *loop) const {
  uint64_t insts = 0;

  for (auto bb : loop->getBasicBlocks()){
    insts += this->getStaticInstructions(bb);
  }

  return insts;
}

uint64_t Hot::getTotalInstructions (LoopStructure *loop) const {
  uint64_t insts = 0;

  for (auto bb : loop->getBasicBlocks()){
    insts += this->getTotalInstructions(bb);
  }

  return insts;
}

double Hot::getDynamicTotalInstructionCoverage (LoopStructure *loop) const {
  auto mInsts = this->getTotalInstructions();
  auto lInsts = this->getTotalInstructions(loop);
  auto hotness = ((double)lInsts) / ((double)mInsts);

  return hotness;
}
