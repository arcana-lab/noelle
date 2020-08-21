/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "PartitionCostAnalysis.hpp"

const std::string llvm::PartitionCostAnalysis::prefix = "Heuristic:   PCA: ";

llvm::PartitionCostAnalysis::PartitionCostAnalysis (
  InvocationLatency &il,
  SCCDAGPartitioner &p,
  SCCDAGAttrs &attrs,
  int cores,
  Verbosity v
) : costIfAllSetsRunOnSeparateCores{0}, totalInstructionCount{0}, IL{il}, sccToInstructionCountMap{},
    partitioner{p}, dagAttrs{attrs}, numCores{cores}, verbose{v} {

  /*
   * Estimate the current latency for executing the pipeline of the current SCCDAG partitioner once.
   */
  auto allSets = partitioner.getSets();
  for (auto set : allSets) {
    for (auto scc : set->sccs) {
      auto instCount = scc->numInternalNodes();
      totalInstructionCount += instCount;
      sccToInstructionCountMap.insert(std::make_pair(scc, instCount));
    }
  }
  costIfAllSetsRunOnSeparateCores = IL.latencyPerInvocation(&dagAttrs, allSets);
}

void llvm::PartitionCostAnalysis::traverseAllPartitionSubsets () {

  /*
   * Collect all subsets of the current SCCDAG partitioner.
   */
  std::queue<SCCSet *> subToCheck;
  std::set<SCCSet *> alreadyChecked;
  for (auto root : partitioner.getRoots()) {
    subToCheck.push(root);
    alreadyChecked.insert(root);
  }

  while (!subToCheck.empty()) {
    auto sub = subToCheck.front();
    subToCheck.pop();
    auto children = partitioner.getChildren(sub);

    /*
     * Check merge criteria on children
     * Traverse them in turn
     */
    for (auto child : children) {
      checkIfShouldMerge(sub, child);
      if (alreadyChecked.find(child) == alreadyChecked.end()){
        subToCheck.push(child);
        alreadyChecked.insert(child);
      }
    }
  }
}

void llvm::PartitionCostAnalysis::resetCandidateSubsetInfo () {
  minSetsToMerge.clear();
  savedCostByMerging = 0;
  costOfMergedSet = UINT64_MAX;
  numInstructionsInSetsBeingMerged = UINT64_MAX;
}

bool llvm::PartitionCostAnalysis::mergeCandidateSubsets () {
  if (minSetsToMerge.size() == 0) return false;

  partitioner.getPartitionGraph()->mergeSetsAndCollapseResultingCycles(minSetsToMerge);
  auto allSets = partitioner.getSets();
  costIfAllSetsRunOnSeparateCores = IL.latencyPerInvocation(&dagAttrs, allSets);
  return true;
}

void llvm::PartitionCostAnalysis::printCandidate (raw_ostream &stream) {
  if (verbose == Verbosity::Disabled) return;

  if (minSetsToMerge.size() == 0) {
    stream << prefix << "No candidates\n";
    return;
  }

  // stream << prefix << "Min subsets:\n";
  // stream << prefix << partitioner.subsetStr(minSubsetA)
  //   << " " << partitioner.subsetStr(minSubsetB) << "\n";
  stream << prefix << "Saved cost: " << savedCostByMerging
    << " Merged set cost: " << costOfMergedSet
    << " Instruction count: " << numInstructionsInSetsBeingMerged << "\n";
}
