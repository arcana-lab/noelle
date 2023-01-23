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
#include "DSWP.hpp"
#include "noelle/core/Architecture.hpp"
#include "noelle/core/LoopIterationSCC.hpp"

namespace llvm::noelle {

std::set<GenericSCC *> DSWP::getClonableSCCs(SCCDAGAttrs *sccManager) const {
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
    if (currentSCCInfo->canBeCloned()) {
      set.insert(currentSCCInfo);
      continue;
    }
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
    if ((SCCDAG->fetchNode(currentSCC)->numOutgoingEdges() > 0)
        && (currentSCC->numInternalNodes() == 1)) {
      auto I = currentSCC->begin_internal_node_map()->first;
      if (isa<PHINode>(I) || isa<GetElementPtrInst>(I) || isa<CastInst>(I)) {
        set.insert(currentSCCInfo);
        continue;
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

} // namespace llvm::noelle
