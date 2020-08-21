/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "HELIX.hpp"

using namespace llvm ;

std::vector<SequentialSegment *> HELIX::identifySequentialSegments (
  LoopDependenceInfo *originalLDI,
  LoopDependenceInfo *LDI
){

  std::vector<SequentialSegment *> sss;

  /*
   * Prepare the initial partition.
   */
  ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::partitionSCCDAG(LDI);

  /*
   * Compute reachability analysis
   */
  auto reachabilityDFR = this->computeReachabilityFromInstructions(LDI);

  /*
   * Identify the loop's preamble, and whether the original loop was IV governed
   */
  auto loopSCCDAG = LDI->sccdagAttrs.getSCCDAG();
  auto preambleSCCNodes = loopSCCDAG->getTopLevelNodes();
  assert(preambleSCCNodes.size() == 1 && "The loop internal SCCDAG should only have one preamble");
  auto preambleSCC = (*preambleSCCNodes.begin())->getT();
  bool wasOriginalLoopIVGoverned = originalLDI->getLoopGoverningIVAttribution() != nullptr;

  /*
   * Fetch the subsets.
   */
  auto& subsets = this->partition->getDepthOrderedSubsets();

  /*
   * Fetch the set of SCCs that have loop-carried data dependences.
   */
  auto depsSCCs = LDI->sccdagAttrs.getSCCsWithLoopCarriedDataDependencies();

  /*
   * Allocate the sequential segments, one per partition.
   */
  int32_t ssID = 0;
  for (auto subset : subsets){

    /*
     * Check if the current set of SCCs require a sequential segments.
     */
    auto requireSS = false;
    for (auto scc : *subset){

      /*
       * Fetch the SCC metadata.
       */
      auto sccInfo = LDI->sccdagAttrs.getSCCAttrs(scc);

      /*
       * Do not synchronize induction variables
       */
      if (sccInfo->isInductionVariableSCC()) {
        continue;
      }

      /*
       * HACK: Our loop governing IV attribution class is not powerful
       * enough to understand our manipulation of the loop governing IV,
       * so we ignore the preamble SCC if the original LDI's attribution was compute-able
       */
      if (wasOriginalLoopIVGoverned && scc == preambleSCC) {
        errs() << "HELIX:   Skipping preamble synchronization\n";
        continue;
      }

      /*
       * If the SCC is due to a control dependence, but the number of iterations can be computed just before executing the loop, then we can skip it.
       */
      if (  true
            && wasOriginalLoopIVGoverned
            && (depsSCCs.find(scc) == depsSCCs.end())
         ){
        continue ;
      }

      /*
       * Fetch the type of the SCC.
       */
      auto sccType = sccInfo->getType();

      /*
       * Only sequential SCC can generate a sequential segment.
       * FIXME: A reducible SCC should not be sequential in nature
       */
      if (sccType == SCCAttrs::SEQUENTIAL) {
        requireSS = true;
        break ;
      }
    }
    if (!requireSS){
      continue ;
    }

    /*
     * Allocate a sequential segment.
     */
    auto ss = new SequentialSegment(LDI, reachabilityDFR, subset, ssID, this->verbose);

    /*
     * Insert the new sequential segment to the list.
     */
    ssID++;
    sss.push_back(ss);
  }

  delete reachabilityDFR;

  return sss;
}
