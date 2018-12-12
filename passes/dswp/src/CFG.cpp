#include "DSWP.hpp"

using namespace llvm;

void DSWP::collectTransitiveCondBrs (DSWPLoopDependenceInfo *LDI,
  std::set<TerminatorInst *> &bottomLevelBrs,
  std::set<TerminatorInst *> &descendantCondBrs
) {
  std::queue<DGNode<Value> *> queuedBrs;
  std::set<TerminatorInst *> visitedBrs;
  for (auto br : bottomLevelBrs) {
    queuedBrs.push(LDI->loopInternalDG->fetchNode(cast<Value>(br)));
    visitedBrs.insert(br);
  }

  while (!queuedBrs.empty()) {
    auto brNode = queuedBrs.front();
    auto term = cast<TerminatorInst>(brNode->getT());
    queuedBrs.pop();
    if (term->getNumSuccessors() > 1) {
      descendantCondBrs.insert(term);
    }

    for (auto edge : brNode->getIncomingEdges()) {
      if (auto termI = dyn_cast<TerminatorInst>(edge->getOutgoingT())) {
        if (visitedBrs.find(termI) == visitedBrs.end()) {
          queuedBrs.push(edge->getOutgoingNode());
          visitedBrs.insert(termI);
        }
      }
    }
  }
}

void DSWP::trimCFGOfStages (DSWPLoopDependenceInfo *LDI) {
  std::set<BasicBlock *> iterEndBBs;
  iterEndBBs.insert(LDI->header);
  for (auto bb : LDI->loopExitBlocks) iterEndBBs.insert(bb);

  /*
   * Collect branches at end of loop iteration
   */
  std::set<TerminatorInst *> iterEndBrs;
  for (auto bb : LDI->loopBBs) {
    auto term = bb->getTerminator();
    for (auto termBB : term->successors()) {
      if (iterEndBBs.find(termBB) != iterEndBBs.end()) {
        iterEndBrs.insert(term);
        break;
      }
    }
  }

  /*
   * Collect conditional branches necessary to capture loop iteration tail branches
   */
  std::set<TerminatorInst *> minNecessaryCondBrs;
  collectTransitiveCondBrs(LDI, iterEndBrs, minNecessaryCondBrs);

  /*
   * Collect conditional branches necessary to capture stage execution
   */
  for (auto techniqueTask : this->tasks) {
    auto task = (DSWPTask *)techniqueTask;
    for (auto br : minNecessaryCondBrs) task->usedCondBrs.insert(br);

    std::set<TerminatorInst *> stageBrs;
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
      stageBrs.insert(LDI->queues[queueIndex]->producer->getParent()->getTerminator());
    }

    collectTransitiveCondBrs(LDI, stageBrs, task->usedCondBrs);
  }
}

void DSWP::generateLoopSubsetForStage (DSWPLoopDependenceInfo *LDI, int taskIndex) {
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
