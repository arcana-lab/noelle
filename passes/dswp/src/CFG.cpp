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
  for (auto techniqueWorker : this->workers) {
    auto worker = (DSWPTechniqueWorker *)techniqueWorker;
    for (auto br : minNecessaryCondBrs) worker->usedCondBrs.insert(br);

    std::set<TerminatorInst *> stageBrs;
    std::set<SCC *> workerSCCs(worker->stageSCCs.begin(), worker->stageSCCs.end());
    workerSCCs.insert(worker->removableSCCs.begin(), worker->removableSCCs.end());
    for (auto scc : workerSCCs) {
      for (auto nodePair : scc->internalNodePairs()) {
        stageBrs.insert(cast<Instruction>(nodePair.first)->getParent()->getTerminator());
      }
    }

    // NOTE: This is because queue loads are done in the basic block of the producer,
    //  hence portions of the CFG where the producer would be contained must be preserved
    for (auto queueIndex : worker->popValueQueues) {
      stageBrs.insert(LDI->queues[queueIndex]->producer->getParent()->getTerminator());
    }

    collectTransitiveCondBrs(LDI, stageBrs, worker->usedCondBrs);
  }
}

void DSWP::generateLoopSubsetForStage (DSWPLoopDependenceInfo *LDI, int workerIndex) {
  auto worker = (DSWPTechniqueWorker *)this->workers[workerIndex];

  /*
   * Clone the portion of the loop within the stage's normal, and clonable, SCCs
   * TODO(angelo): Rename removable to clonable. The name removable stemed from
   * its irrelevance when partitioning stages as it gets duplicated
   */
  std::set<Instruction *> subset;
  for (auto scc : worker->removableSCCs) {
    for (auto nodePair : scc->internalNodePairs()) {
      subset.insert(cast<Instruction>(nodePair.first));
    }
  }
  for (auto scc : worker->stageSCCs) {
    for (auto nodePair : scc->internalNodePairs()) {
      subset.insert(cast<Instruction>(nodePair.first));
    }
  }
  this->cloneSequentialLoopSubset(LDI, worker->order, subset);

  /*
   * Determine the needed basic block terminators outside of the stage's SCCs
   * to capture control flow through the loop body to either loop latch or loop
   * exiting basic blocks
   */
  auto &iClones = worker->instructionClones;
  auto &bbClones = worker->basicBlockClones;
  auto &cxt = worker->F->getContext();
  for (auto B : LDI->loopBBs) {
    auto terminator = cast<Instruction>(B->getTerminator());
    if (iClones.find(terminator) == iClones.end()) {
      Instruction *termClone = nullptr;
      if (worker->usedCondBrs.find(B->getTerminator()) != worker->usedCondBrs.end()) {
        termClone = terminator->clone();
      } else {
        termClone = BranchInst::Create(LDI->loopBBtoPD[B]);
      }
      iClones[terminator] = termClone;

      if (bbClones.find(B) == bbClones.end()) {
        bbClones[B] = BasicBlock::Create(cxt, "", worker->F);
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
    worker->basicBlockClones[LDI->loopExitBlocks[i]] = worker->loopExitBlocks[i];
  }
}
