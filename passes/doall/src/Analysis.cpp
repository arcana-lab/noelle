#include "DOALL.hpp"

bool DOALL::canBeAppliedToLoop (LoopDependenceInfo *LDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) const {

  /*
   * Check the number of exits of the loop.
   */
  if (LDI->loopExitBlocks.size() > 1){
    return false;
  }
  assert(LDI->loopExitBlocks.size() == 1);

  /*
   * Check the loop induction variables.
   *
   * Check we have at least one induction variable: we need at least one induction variable to know the number of iterations.
   */
  if (!LDI->sccdagAttrs.loopHasInductionVariable(SE)){
    return false;
  }
  auto nonDOALLSCCs = LDI->sccdagAttrs.getSCCsWithLoopCarriedDataDependencies();
  for (auto scc : nonDOALLSCCs) {
    auto &sccInfo = LDI->sccdagAttrs.getSCCAttrs(scc);
    auto isSCCOK = 
         scc->getType() == SCC::SCCType::COMMUTATIVE
      || sccInfo->isClonable
      || LDI->sccdagAttrs.isSCCContainedInSubloop(LDI->liSummary, scc);
    if (!isSCCOK){
      return false;
    }
  }

  return true;
}
