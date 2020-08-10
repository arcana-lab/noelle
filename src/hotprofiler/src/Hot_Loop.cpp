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
      
double Hot::getAverageLoopIterationsPerInvocation (LoopStructure *loop) const {

  /*
   * Fetch the number of times the loop is invoked.
   */
  auto loopInvocations = this->getInvocations(loop);
  if (loopInvocations == 0){
    return 0;
  }

  /*
   * Fetch the total number of iterations executed.
   */
  auto loopIterations = this->getIterations(loop);

  /*
   * Compute the stats.
   */
  auto stats = ((double)loopIterations) / ((double)loopInvocations);

  return stats;
}

double Hot::getAverageTotalInstructionsPerInvocation (LoopStructure *loop) const {

  /*
   * Fetch the number of times the loop is invoked.
   */
  auto loopInvocations = this->getInvocations(loop);
  if (loopInvocations == 0){
    return 0;
  }

  /*
   * Compute the stats.
   */
  auto loopTotal = this->getTotalInstructions(loop);
  auto averageInstsPerInvocation = ((double)loopTotal) / ((double)loopInvocations);

  return averageInstsPerInvocation;
}

double Hot::getAverageTotalInstructionsPerIteration (LoopStructure *loop) const {

  /*
   * Fetch the average number of instructions per invocation.
   */
  auto instsPerInvocation = this->getAverageTotalInstructionsPerInvocation(loop);
  if (instsPerInvocation == 0){
    return 0;
  }

  /*
   * Fetch the average number of iterations per invocation.
   */
  auto itersPerInvocation = this->getAverageLoopIterationsPerInvocation(loop);

  /*
   * Compute the average instructions per iteration.
   */
  auto instsPerIteration = instsPerInvocation / itersPerInvocation;

  return instsPerIteration;
}

uint64_t Hot::getIterations (LoopStructure *l) const {

  /*
   * Fetch the header.
   */
  auto loopHeader = l->getHeader();

  /*
   * Fetch the invocations of the header and its successors within the loop.
   */
  auto headerInvocations = this->getInvocations(loopHeader);
  uint64_t succInvocations = 0;
  for (auto succBB : successors(loopHeader)){
    if (!l->isIncluded(succBB)){
      continue ;
    }
    succInvocations += this->getInvocations(succBB);
  }

  /*
   * Compute the total number of iterations executed.
   */
  uint64_t loopIterations = 0;
  if (headerInvocations == succInvocations){
    loopIterations = headerInvocations;

  } else {
    loopIterations = headerInvocations - 1;
  }

  return loopIterations;
}
