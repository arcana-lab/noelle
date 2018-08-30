#include "DOALL.hpp"

bool DOALL::canBeAppliedToLoop (LoopDependenceInfo *LDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) const {

  if (LDI->loopExitBlocks.size() > 1) return false;

  errs() << "DOALL CHECKS --------- IS DOALL (loop exit blocks == 1) \n";
  if (!LDI->sccdagAttrs.allPostLoopEnvValuesAreReducable(LDI->environment)) return false;
  errs() << "DOALL CHECKS --------- IS DOALL (post env reducable) \n";

  if (!LDI->sccdagAttrs.loopHasInductionVariable(SE)) return false;
  errs() << "DOALL CHECKS --------- IS DOALL (has IV) \n";

  auto nonDOALLSCCs = LDI->sccdagAttrs.getSCCsWithLoopCarriedDataDependencies();
  bool allSCCsDOALL = true;
  for (auto scc : nonDOALLSCCs) {
    scc->print(errs() << "Loop carried dep scc:\n") << "\n";
    auto &sccInfo = LDI->sccdagAttrs.getSCCAttrs(scc);
    allSCCsDOALL &= scc->getType() == SCC::SCCType::COMMUTATIVE
      || sccInfo->isClonable
      || LDI->sccdagAttrs.isSCCContainedInSubloop(LDI->liSummary, scc);
    // errs() << "DOALL CHECKS --------- IS DOALL (scc): " << isDOALL << "\n";
  }
  return allSCCsDOALL;
}
