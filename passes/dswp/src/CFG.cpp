#include "DSWP.hpp"

using namespace llvm;

void DSWP::remapControlFlow (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
{
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

void DSWP::trimCFGOfStages (DSWPLoopDependenceInfo *LDI)
{
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
  auto &envMap = LDI->environment->producerIndexMap;
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
          op.set(stageInfo->envLoadMap[envMap[opV]]);
        } else if (queueMap.find(opI) != queueMap.end()) {
          op.set(stageInfo->queueInstrMap[queueMap[opI]]->load);
        } else {
          ignoreOperandAbort(opV, cloneInstruction);
        }
        continue;
      } else if (auto opA = dyn_cast<Argument>(opV)) {
        if (LDI->environment->isPreLoopEnv(opV)) {
          op.set(stageInfo->envLoadMap[envMap[opV]]);
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
