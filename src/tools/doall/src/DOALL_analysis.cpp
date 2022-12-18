/*
 * Copyright 2021 - 2022  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "DOALL.hpp"
#include "noelle/core/Reduction.hpp"

namespace llvm::noelle {

std::set<SCC *> DOALL::getSCCsThatBlockDOALLToBeApplicable(
    LoopDependenceInfo *LDI,
    Noelle &par) {
  std::set<SCC *> sccs;

  /*
   * Fetch the SCC manager of the loop given as input
   */
  auto sccManager = LDI->getSCCManager();

  /*
   * Iterate over SCCs with loop-carried data dependences
   */
  auto nonDOALLSCCs = sccManager->getSCCsWithLoopCarriedDataDependencies();
  for (auto sccInfo : nonDOALLSCCs) {

    /*
     * If the SCC is reducable, then it does not block the loop to be a DOALL.
     */
    if (isa<Reduction>(sccInfo)) {
      continue;
    }

    /*
     * If the SCC can be cloned, then it does not block the loop to be a DOALL.
     */
    if (sccInfo->canBeCloned()) {
      continue;
    }

    /*
     * If the SCC can be removed by cloning objects, then we can ignore it.
     */
    if (sccInfo->canBeClonedUsingLocalMemoryLocations()) {
      continue;
    }

    /*
     * If all loop carried data dependencies within the SCC do not overlap
     * between iterations, then DOALL can ignore them
     */
    auto areAllDataLCDsFromDisjointMemoryAccesses = true;
    auto domainSpaceAnalysis = LDI->getLoopIterationDomainSpaceAnalysis();
    for (auto dep : sccInfo->getLoopCarriedDependences()) {
      if (dep->isControlDependence()) {
        continue;
      }
      if (!dep->isMemoryDependence()) {
        areAllDataLCDsFromDisjointMemoryAccesses = false;
        break;
      }

      auto fromInst = dyn_cast<Instruction>(dep->getOutgoingT());
      auto toInst = dyn_cast<Instruction>(dep->getIncomingT());
      areAllDataLCDsFromDisjointMemoryAccesses &=
          fromInst && toInst
          && domainSpaceAnalysis
                 ->areInstructionsAccessingDisjointMemoryLocationsBetweenIterations(
                     fromInst,
                     toInst);
      if (!areAllDataLCDsFromDisjointMemoryAccesses) {
        break;
      }
    }
    if (areAllDataLCDsFromDisjointMemoryAccesses) {
      continue;
    }

    /*
     * We found an SCC that blocks DOALL to be applicable.
     */
    auto scc = sccInfo->getSCC();
    sccs.insert(scc);
  }

  return sccs;
}

} // namespace llvm::noelle
