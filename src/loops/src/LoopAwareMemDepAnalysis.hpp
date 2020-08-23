// TODO: add copyright

#pragma once

#include "SystemHeaders.hpp"

#include "PDG.hpp"
#include "MemoryAnalysisModules/LoopAA.h"
#include "LoopCarriedDependencies.hpp"
#include "LoopIterationDomainSpaceAnalysis.hpp"
#include "LoopsSummary.hpp"

namespace llvm {

// Perform loop-aware memory dependence analysis to refine the loop PDG
void refinePDGWithLoopAwareMemDepAnalysis(
  PDG *loopDG,
  Loop *l,
  LoopStructure *loopStructure,
  LoopCarriedDependencies &LCD,
  liberty::LoopAA *loopAA,
  LoopIterationDomainSpaceAnalysis *LIDS
);

// Refine the loop PDG with SCAF
void refinePDGWithSCAF(PDG *loopDG, Loop *l, liberty::LoopAA *loopAA);

void refinePDGWithLIDS(
  PDG *loopDG,
  LoopStructure *loopStructure,
  LoopCarriedDependencies &LCD,
  LoopIterationDomainSpaceAnalysis *LIDS
);

} // namespace llvm
