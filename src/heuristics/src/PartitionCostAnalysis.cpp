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
  SCCDAGPartition &p,
  SCCDAGAttrs &attrs,
  int cores,
  Verbosity v
) : totalCost{0}, totalInstCount{0}, IL{il},
    partition{p}, dagAttrs{attrs}, numCores{cores}, verbose{v} {

  /*
   * Estimate the current latency for executing the pipeline of the current SCCDAG partition once.
   */
  for (auto subset : *partition.getSubsets()) {
    uint64_t instCount = 0;
    for (auto scc : *subset) instCount += scc->numInternalNodes();
    std::set<SCCset *> single = { subset };
    uint64_t cost = IL.latencyPerInvocation(&dagAttrs, single);

    subsetCost[subset] = cost;
    subsetInstCount[subset] = instCount;

    totalInstCount += instCount;
    totalCost += cost;
  }
}

void llvm::PartitionCostAnalysis::traverseAllPartitionSubsets () {

  /*
   * Collect all subsets of the current SCCDAG partition.
   */
  std::queue<SCCset *> subToCheck;
  std::set<SCCset *> alreadyChecked;
  for (auto root : *partition.getRoots()) {
    subToCheck.push(root);
    alreadyChecked.insert(root);
  }

  while (!subToCheck.empty()) {
    auto sub = subToCheck.front();
    subToCheck.pop();
    auto children = partition.getChildren(sub);
    if (!children) continue;

    /*
     * Check merge criteria on children
     * Traverse them in turn
     */
    for (auto child : *children) {
      checkIfShouldMerge(sub, child);
      if (alreadyChecked.find(child) == alreadyChecked.end()){
        subToCheck.push(child);
        alreadyChecked.insert(child);
      }
    }
  }
}

void llvm::PartitionCostAnalysis::resetCandidateSubsetInfo () {
  minSubsetA = minSubsetB = nullptr;
  loweredCost = 0;
  mergedSubsetCost = totalCost;
  instCount = totalInstCount;
}

bool llvm::PartitionCostAnalysis::mergeCandidateSubsets () {
  if (!minSubsetA) return false;

  auto mergedSub = partition.mergePair(minSubsetA, minSubsetB);

  /*
   * Readjust subset cost tracking
   */
  subsetCost[mergedSub] = mergedSubsetCost;
  subsetInstCount[mergedSub] = instCount;
  totalCost -= loweredCost;
  return true;
}

void llvm::PartitionCostAnalysis::printCandidate (raw_ostream &stream) {
  if (verbose == Verbosity::Disabled) return;

  if (!minSubsetA || !minSubsetB) {
    stream << prefix << "No candidates\n";
    return;
  }
  stream << prefix << "Min subsets:\n";
  stream << prefix << partition.subsetStr(minSubsetA)
    << " " << partition.subsetStr(minSubsetB) << "\n";
  stream << prefix << "Lowered cost: " << loweredCost
    << " Merged subset cost: " << mergedSubsetCost
    << " Instruction count: " << instCount << "\n";
}
