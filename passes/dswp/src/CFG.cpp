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

