/*
 * Copyright 2022 - 2023  Simone Campanoni
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
#include "noelle/tools/DSWP.hpp"
#include "noelle/core/Architecture.hpp"
#include "noelle/core/LoopIterationSCC.hpp"

namespace arcana::noelle {

std::set<GenericSCC *> DSWP::getClonableSCCs(SCCDAGAttrs *sccManager,
                                             LoopTree *loopNode) const {
  std::set<GenericSCC *> set;

  auto SCCDAG = sccManager->getSCCDAG();
  for (auto nodePair : SCCDAG->internalNodePairs()) {

    /*
     * Fetch the current SCC.
     */
    auto currentSCC = nodePair.first;
    auto currentSCCInfo = sccManager->getSCCAttrs(currentSCC);

    /*
     * Check if there is no loop-carried dependence.
     */
    if (isa<LoopIterationSCC>(currentSCCInfo)) {
      // set.insert(currentSCCInfo);
      // continue ;
    }

    /*
     * Check if the current SCC can be removed (e.g., because it is due to
     * induction variables). If it is, then this SCC has already been assigned
     * to every dependent partition.
     */
    auto onlyTerminators = true;
    for (auto iNodePair : currentSCC->internalNodePairs()) {
      auto V = iNodePair.first;
      if (auto inst = dyn_cast<Instruction>(V)) {
        if (!isa<CmpInst>(inst) && !inst->isTerminator()) {
          onlyTerminators = false;
          break;
        }
      }
    }
    if (onlyTerminators) {
      set.insert(currentSCCInfo);
      continue;
    }

    /*
     * Check if the SCC can be trivially cloned on all DSWP stages.
     */
    auto currentSCCNode = SCCDAG->fetchNode(currentSCC);
    if (currentSCCNode->outDegree() > 0) {

      /*
       * First case: the SCC contains only instructions that do not represent
       * actual computation. These instructions can always be replicated
       * anywhere to any DSWP stage.
       */
      if (currentSCC->numInternalNodes() == 1) {
        auto I = currentSCC->begin_internal_node_map()->first;
        if (isa<PHINode>(I) || isa<GetElementPtrInst>(I) || isa<CastInst>(I)) {
          set.insert(currentSCCInfo);
          continue;
        }
      }

      /*
       * Second case: the SCC does not have memory dependences.
       */
      if (!currentSCCInfo->doesHaveMemoryDependencesWithin()) {

        /*
         * The SCC has no memory dependences.
         *
         * Check if there is no loop-carried dependence.
         */
        auto hasNoLoopCarriedDependence = isa<LoopIterationSCC>(currentSCCInfo);
        if (hasNoLoopCarriedDependence) {
          set.insert(currentSCCInfo);
          continue;
        }

        /*
         * The SCC has loop-carried dependences.
         *
         * Check if the loop-carried dependences are fully contained within
         * sub-loops.
         */
        auto lcSCC = cast<LoopCarriedSCC>(currentSCCInfo);
        auto topLoop = loopNode->getLoop();
        auto lcFullyContained = true;
        for (auto loopCarriedDependency : lcSCC->getLoopCarriedDependences()) {
          auto valueFrom = loopCarriedDependency->getSrc();
          auto valueTo = loopCarriedDependency->getDst();
          assert(isa<Instruction>(valueFrom) && isa<Instruction>(valueTo));
          if (loopNode->getInnermostLoopThatContains(
                  cast<Instruction>(valueFrom))
                  == topLoop
              || loopNode->getInnermostLoopThatContains(
                     cast<Instruction>(valueTo))
                     == topLoop) {
            lcFullyContained = false;
            break;
          }
        }
        if (lcFullyContained) {
          set.insert(currentSCCInfo);
          continue;
        }
      }
    }
  }

  return set;
}

bool DSWP::canBeCloned(GenericSCC *scc) const {
  if (this->clonableSCCs.find(scc) != this->clonableSCCs.end()) {
    return true;
  }

  return false;
}

} // namespace arcana::noelle
