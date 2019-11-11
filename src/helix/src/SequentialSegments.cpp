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

std::vector<SequentialSegment *> HELIX::identifySequentialSegments (LoopDependenceInfo *LDI){
  std::vector<SequentialSegment *> sss;

  /*
   * Prepare the initial partition.
   */
  ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::partitionSCCDAG(LDI);

  /*
   * Fetch the subsets.
   */
  auto& subsets = this->partition->getDepthOrderedSubsets();

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
       * Fetch the type of the SCC.
       */
      auto sccType = sccInfo->getType();

      /*
       * Only sequential SCC can generate a sequential segment.
       * FIXME: A reducible SCC should not be sequential in nature
       */
      if (  (sccType == SCCAttrs::SEQUENTIAL)  ||
            (sccType == SCCAttrs::REDUCIBLE) ){
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
    auto ss = new SequentialSegment(LDI, subset, ssID, this->verbose);

    /*
     * Insert the new sequential segment to the list.
     */
    ssID++;
    sss.push_back(ss);
  }

  return sss;
}
