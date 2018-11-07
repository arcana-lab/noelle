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

void DSWP::remapControlFlow (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo) {
  auto &context = LDI->function->getContext();
  auto stageF = stageInfo->sccStage;

  for (auto bb : LDI->loopBBs)
  {
    auto originalT = bb->getTerminator();
    if (stageInfo->iCloneMap.find(originalT) == stageInfo->iCloneMap.end()) continue;
    auto terminator = cast<TerminatorInst>(stageInfo->iCloneMap[originalT]);
    for (int i = 0; i < terminator->getNumSuccessors(); ++i)
    {
      auto succBB = terminator->getSuccessor(i);
      assert(stageInfo->sccBBCloneMap.find(succBB) != stageInfo->sccBBCloneMap.end());
      terminator->setSuccessor(i, stageInfo->sccBBCloneMap[succBB]);
    }
  }

  for (auto bbPair : stageInfo->sccBBCloneMap)
  {
    if (bbPair.second->size() == 0) continue;
    auto iIter = bbPair.second->begin();
    while (auto phi = dyn_cast<PHINode>(&*iIter))
    {
      for (auto bb : phi->blocks())
      {
        phi->setIncomingBlock(phi->getBasicBlockIndex(bb), stageInfo->sccBBCloneMap[bb]);
      }
      ++iIter;
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
  for (auto bb : LDI->loopBBs)
  {
    auto term = bb->getTerminator();
    for (auto termBB : term->successors())
    {
      if (iterEndBBs.find(termBB) != iterEndBBs.end())
      {
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
  for (auto &stage : LDI->stages)
  {
    for (auto br : minNecessaryCondBrs) stage->usedCondBrs.insert(br);

    std::set<TerminatorInst *> stageBrs;
    for (auto stageSCC : stage->stageSCCs)
    {
      for (auto nodePair : stageSCC->internalNodePairs())
      {
        stageBrs.insert(cast<Instruction>(nodePair.first)->getParent()->getTerminator());
      }
    }
    for (auto queueIndex : stage->popValueQueues)
    {
      stageBrs.insert(LDI->queues[queueIndex]->producer->getParent()->getTerminator());
    }

    collectTransitiveCondBrs(LDI, stageBrs, stage->usedCondBrs);
  }
}

void DSWP::remapOperandsOfInstClones (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
{
  auto &iCloneMap = stageInfo->iCloneMap;
  auto &queueMap = stageInfo->producedPopQueue;

  auto ignoreOperandAbort = [&](Value *opV, Instruction *cloneInstruction) -> void {
    opV->print(errs() << "Ignore operand\t"); cloneInstruction->print(errs() << "\nInstr:\t"); errs() << "\n";
    stageInfo->sccStage->print(errs() << "Current function state:\n"); errs() << "\n";
    abort();
  };

  for (auto ii = iCloneMap.begin(); ii != iCloneMap.end(); ++ii) {
    auto cloneInstruction = ii->second;

    for (auto &op : cloneInstruction->operands()) {
      auto opV = op.get();
      if (auto opI = dyn_cast<Instruction>(opV)) {
        if (iCloneMap.find(opI) != iCloneMap.end()) {
          op.set(iCloneMap[opI]);
        } else if (LDI->environment->isPreLoopEnv(opV)) {
          op.set(stageInfo->envLoadMap[LDI->environment->indexOfProducer(opV)]);
        } else if (queueMap.find(opI) != queueMap.end()) {
          op.set(stageInfo->queueInstrMap[queueMap[opI]]->load);
        } else {
          ignoreOperandAbort(opV, cloneInstruction);
        }
        continue;
      } else if (auto opA = dyn_cast<Argument>(opV)) {
        if (LDI->environment->isPreLoopEnv(opV)) {
          op.set(stageInfo->envLoadMap[LDI->environment->indexOfProducer(opV)]);
        } else {
          ignoreOperandAbort(opV, cloneInstruction);
        }
      } else if (isa<Constant>(opV) || isa<BasicBlock>(opV) || isa<Function>(opV)) {
        continue;
      } else if (isa<MetadataAsValue>(opV) || isa<InlineAsm>(opV) || isa<DerivedUser>(opV) || isa<Operator>(opV)) {
        continue;
      } else {
        opV->print(errs() << "Unknown what to do with operand\n"); opV->getType()->print(errs() << "\tType:\t");
        cloneInstruction->print(errs() << "\nInstr:\t"); errs() << "\n";
        stageInfo->sccStage->print(errs() << "Current function state:\n"); errs() << "\n";
        abort();
      }
    }
  }
}

void DSWP::createInstAndBBForSCC (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo) {
  auto &context = LDI->function->getParent()->getContext();

  /*
   * Clone the portion of the loop within the stage's normal, and clonable, SCCs
   * TODO(angelo): Rename removable to clonable. The name removable stemed from
   * its irrelevance when partitioning stages as it gets duplicated
   */
  std::set<SCC *> subset;
  for (auto scc : stageInfo->stageSCCs) subset.insert(scc);
  for (auto scc : stageInfo->removableSCCs) subset.insert(scc);
  this->cloneSequentialLoopSubset(LDI, stageInfo->order, subset);

  /*
   * Determine the needed basic block terminators outside of the stage's SCCs
   * to capture control flow through the loop body to either loop latch or loop
   * exiting basic blocks
   */
  auto worker = this->workers[stageInfo->order];
  for (auto B : LDI->loopBBs) {
    auto terminator = cast<Instruction>(B->getTerminator());
    if (worker->instructionClones.find(terminator) == worker->instructionClones.end()) {
      if (stageInfo->usedCondBrs.find(B->getTerminator()) != stageInfo->usedCondBrs.end())
      {
        worker->instructionClones[terminator] = terminator->clone();
        continue;
      }
      worker->instructionClones[terminator] = BranchInst::Create(LDI->loopBBtoPD[B]);
    }
  }

  /*
   * Map loop exit block clones
   * TODO(angelo): Have ParallelizationTechnique expose an API to do this more generally
   */
  for (int i = 0; i < LDI->loopExitBlocks.size(); ++i) {
    worker->basicBlockClones[LDI->loopExitBlocks[i]] = stageInfo->loopExitBlocks[i];
  }
}
