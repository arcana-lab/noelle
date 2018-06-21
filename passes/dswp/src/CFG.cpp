#include "DSWP.hpp"

using namespace llvm;

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

