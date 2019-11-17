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

void DSWP::collectTransitiveCondBrs (LoopDependenceInfo *LDI,
  std::set<Instruction *> &bottomLevelBrs,
  std::set<Instruction *> &descendantCondBrs
) {

  /*
   * Fetch the loop dependence graph
   */
  auto loopDG = LDI->getLoopDG();

  /*
   * Collect the branches
   */
  std::queue<DGNode<Value> *> queuedBrs;
  std::set<Instruction *> visitedBrs;
  for (auto br : bottomLevelBrs) {
    queuedBrs.push(loopDG->fetchNode(cast<Value>(br)));
    visitedBrs.insert(br);
  }

  while (!queuedBrs.empty()) {
    auto brNode = queuedBrs.front();
    auto term = cast<Instruction>(brNode->getT());
    queuedBrs.pop();
    if (term->getNumSuccessors() > 1) {
      descendantCondBrs.insert(term);
    }

    for (auto edge : brNode->getIncomingEdges()) {
      if (auto termI = dyn_cast<Instruction>(edge->getOutgoingT())) {
        if (  true
              && termI->isTerminator()
              && (visitedBrs.find(termI) == visitedBrs.end()) 
            ){
          queuedBrs.push(edge->getOutgoingNode());
          visitedBrs.insert(termI);
        }
      }
    }
  }
}

void DSWP::trimCFGOfStages (LoopDependenceInfo *LDI) {
  std::set<BasicBlock *> iterEndBBs;
  iterEndBBs.insert(LDI->header);
  for (auto bb : LDI->loopExitBlocks) iterEndBBs.insert(bb);

  /*
   * Collect branches at end of loop iteration
   */
  std::set<Instruction *> iterEndBrs;
  for (auto bb : LDI->loopBBs) {
    auto term = bb->getTerminator();
    for (auto termBB : successors(bb)){
      if (iterEndBBs.find(termBB) != iterEndBBs.end()) {
        iterEndBrs.insert(term);
        break;
      }
    }
  }

  /*
   * Collect conditional branches necessary to capture loop iteration tail branches
   */
  std::set<Instruction *> minNecessaryCondBrs;
  collectTransitiveCondBrs(LDI, iterEndBrs, minNecessaryCondBrs);

  /*
   * Collect conditional branches necessary to capture stage execution
   */
  for (auto techniqueTask : this->tasks) {
    auto task = (DSWPTask *)techniqueTask;
    for (auto br : minNecessaryCondBrs) task->usedCondBrs.insert(br);

    std::set<Instruction *> stageBrs;
    std::set<SCC *> taskSCCs(task->stageSCCs.begin(), task->stageSCCs.end());
    taskSCCs.insert(task->removableSCCs.begin(), task->removableSCCs.end());
    for (auto scc : taskSCCs) {
      for (auto nodePair : scc->internalNodePairs()) {
        stageBrs.insert(cast<Instruction>(nodePair.first)->getParent()->getTerminator());
      }
    }

    // NOTE: This is because queue loads are done in the basic block of the producer,
    //  hence portions of the CFG where the producer would be contained must be preserved
    for (auto queueIndex : task->popValueQueues) {
      stageBrs.insert(this->queues[queueIndex]->producer->getParent()->getTerminator());
    }

    collectTransitiveCondBrs(LDI, stageBrs, task->usedCondBrs);
  }
}

void DSWP::generateLoopSubsetForStage (LoopDependenceInfo *LDI, int taskIndex) {
  auto task = (DSWPTask *)this->tasks[taskIndex];

  /*
   * Clone the portion of the loop within the stage's normal, and clonable, SCCs
   * TODO(angelo): Rename removable to clonable. The name removable stemed from
   * its irrelevance when partitioning stages as it gets duplicated
   */
  std::set<Instruction *> subset;
  for (auto scc : task->removableSCCs) {
    for (auto nodePair : scc->internalNodePairs()) {
      subset.insert(cast<Instruction>(nodePair.first));
    }
  }
  for (auto scc : task->stageSCCs) {
    for (auto nodePair : scc->internalNodePairs()) {
      subset.insert(cast<Instruction>(nodePair.first));
    }
  }
  this->cloneSequentialLoopSubset(LDI, task->order, subset);

  /*
   * Determine the needed basic block terminators outside of the stage's SCCs
   * to capture control flow through the loop body to either loop latch or loop
   * exiting basic blocks
   */
  auto &iClones = task->instructionClones;
  auto &bbClones = task->basicBlockClones;
  auto &cxt = task->F->getContext();
  for (auto B : LDI->loopBBs) {
    auto terminator = cast<Instruction>(B->getTerminator());
    if (iClones.find(terminator) == iClones.end()) {
      Instruction *termClone = nullptr;
      if (task->usedCondBrs.find(B->getTerminator()) != task->usedCondBrs.end()) {
        termClone = terminator->clone();
      } else {
        termClone = BranchInst::Create(LDI->loopBBtoPD[B]);
      }
      iClones[terminator] = termClone;

      if (bbClones.find(B) == bbClones.end()) {
        bbClones[B] = BasicBlock::Create(cxt, "", task->F);
      }
      IRBuilder<> builder(bbClones[B]);
      builder.Insert(termClone);
    }
  }

  /*
   * Map loop exit block clones
   * TODO(angelo): Have ParallelizationTechnique expose an API to do this more generally
   */
  for (int i = 0; i < LDI->loopExitBlocks.size(); ++i) {
    task->basicBlockClones[LDI->loopExitBlocks[i]] = task->loopExitBlocks[i];
  }
}
