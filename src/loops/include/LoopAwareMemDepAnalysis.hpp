// TODO: add copyright

#pragma once

#include "SystemHeaders.hpp"

#include "PDG.hpp"
#include "MemoryAnalysisModules/LoopAA.h"

namespace llvm {

// Perform loop-aware memory dependence analysis to refine the loop PDG
void refinePDGWithLoopAwareMemDepAnalysis(PDG *loopDG, Loop *l,
                                          liberty::LoopAA *loopAA);

// Refine the loop PDG with SCAF
void refinePDGWithSCAF(PDG *loopDG, Loop *l, liberty::LoopAA *loopAA);

} // namespace llvm
