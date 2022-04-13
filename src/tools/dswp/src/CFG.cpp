/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "DSWP.hpp"

using namespace llvm;
using namespace llvm::noelle;

void DSWP::generateLoopSubsetForStage (LoopDependenceInfo *LDI, int taskIndex) {
  auto task = (DSWPTask *)this->tasks[taskIndex];

  /*
   * Fetch the header.
   */
  auto loopSummary = LDI->getLoopStructure();
  auto loopHeader = loopSummary->getHeader();

  /*
   * Clone the portion of the loop within the stage's normal, and clonable, SCCs
   */
  std::set<Instruction *> subset;
  for (auto scc : task->clonableSCCs) {
    for (auto nodePair : scc->internalNodePairs()) {
      subset.insert(cast<Instruction>(nodePair.first));
    }
  }
  for (auto scc : task->stageSCCs) {
    for (auto nodePair : scc->internalNodePairs()) {
      subset.insert(cast<Instruction>(nodePair.first));
    }
  }

  /*
   * Include control queue consumers (conditional branches) explicitly.
   * They would be left out IF they don't belong to this task, rather some parent task
   */
  for (auto queueIndex : task->popValueQueues) {
    for (auto conditionalBranch : this->queues.at(queueIndex)->consumers) {
      if (conditionalBranch->isTerminator()) {
        subset.insert(conditionalBranch);
      }
    }
  }

  this->cloneSequentialLoopSubset(LDI, task->getID(), subset);

  /*
   * Create an empty basic block for all basic blocks in the loop to be potentially used in the task
   */
  auto &cxt = task->getTaskBody()->getContext();
  for (auto B : loopSummary->getBasicBlocks()) {
    if (!task->isAnOriginalBasicBlock(B)){
      task->addBasicBlockStub(B);
    }
  }

  auto loopExitsVector = LDI->getLoopStructure()->getLoopExitBasicBlocks();
  std::set<BasicBlock *> loopExits(loopExitsVector.begin(), loopExitsVector.end());
  std::queue<BasicBlock *> queueToFindMissingBBs;
  std::set<BasicBlock *> visitedBBs(loopExits.begin(), loopExits.end());
  queueToFindMissingBBs.push(loopHeader);

  /*
   * Traverse basic blocks from the header to all loop exits.
   * Determine if the basic block has a terminator instruction created for this task
   * If it does not, add a branch instruction to the basic block's post dominating basic block
   * 
   * This is to avoid handling loops or other complex control structures uniquely which
   * do not concern this task. Trimming of the resulting control flow graph can be done later
   */
  while (!queueToFindMissingBBs.empty()) {
    auto originalB = queueToFindMissingBBs.front();
    queueToFindMissingBBs.pop();
    if (visitedBBs.find(originalB) != visitedBBs.end()) continue;
    visitedBBs.insert(originalB);

    assert(task->isAnOriginalBasicBlock(originalB) && "Basic block not cloned!");
    auto clonedB = task->getCloneOfOriginalBasicBlock(originalB);

    if (!clonedB->getTerminator() || !clonedB->getTerminator()->isTerminator()) {
      auto postDominatingBB = this->originalFunctionDS->PDT.getNode(originalB)->getIDom()->getBlock();
      assert(loopExits.find(postDominatingBB) == loopExits.end() && "Loop exiting terminator not cloned by task!");

      IRBuilder<> builder(clonedB);
      builder.Insert(BranchInst::Create(task->getCloneOfOriginalBasicBlock(postDominatingBB)));
      queueToFindMissingBBs.push(postDominatingBB);

    } else {
      for (auto successorBB = succ_begin(originalB); successorBB != succ_end(originalB); ++successorBB) {
        queueToFindMissingBBs.push(*successorBB);
      }
    }
  }

  /*
   * Remove remaining basic blocks created that have no terminator
   * TODO: This should not be necessary. Investigate the contents of loopBBs to determine the issue
   */
  for (auto B : loopSummary->getBasicBlocks()) {
    auto clonedB = task->getCloneOfOriginalBasicBlock(B);
    if (clonedB->getTerminator()) continue;
    clonedB->eraseFromParent();
    task->removeOriginalBasicBlock(B);
  }
}
