/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "MinMaxSizePartitionAnalysis.hpp"

using namespace llvm;
using namespace llvm::noelle;

void MinMaxSizePartitionAnalysis::checkIfShouldMerge (SCCSet *sA, SCCSet *sB) {

  /*
   * Hard stop merging once we have fewer partitions than cores
   */
  if (partitioner.getPartitionGraph()->numNodes() <= numCores) return ;

  /*
   * Compute all sets that have to be merged if the two target sets are merged
   */
  std::unordered_set<SCCSet *> setsInMerge = partitioner.getCycleIntroducedByMerging(sA, sB);

  /*
   * Compute the cost of running all these sets on one core
   */
  SCCSet potentialMerge;
  uint64_t instCountOfMerge = 0;
  for (auto set : setsInMerge) {
    potentialMerge.sccs.insert(set->sccs.begin(), set->sccs.end());
    for (auto scc : set->sccs) {
      instCountOfMerge += this->sccToInstructionCountMap.at(scc);
    }
  }
  std::unordered_set<SCCSet *> singleSet = { &potentialMerge };
  uint64_t costOnceMerged = IL.latencyPerInvocation(&dagAttrs, singleSet);

  /*
   * Only merge if it is the cheapest of the merges
   */
  if (costOnceMerged > this->costOfMergedSet) return ;

  /*
   * Only merge if it is the smallest of equally cost effective merges
   */
  if (costOnceMerged == this->costOfMergedSet
    && instCountOfMerge > this->numInstructionsInSetsBeingMerged) return ;

  /*
   * Save merge candidate
   * TODO: Compute saved cost by merging
   */
  this->minSetsToMerge = setsInMerge;
  this->costOfMergedSet = costOnceMerged;
  this->numInstructionsInSetsBeingMerged = instCountOfMerge;

  if (verbose >= Verbosity::Maximal) {
    errs() << prefix << "Lowered cost: " << savedCostByMerging
      << " Merged cost: " << costOfMergedSet
      << " Instruction count: " << instCountOfMerge << "\n";
  }

};
