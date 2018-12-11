#include "PartitionCostAnalysis.hpp"

const std::string llvm::PartitionCostAnalysis::prefix = "Heuristic:   PCA: ";

llvm::PartitionCostAnalysis::PartitionCostAnalysis (
  InvocationLatency &il,
  SCCDAGPartition &p,
  SCCDAGAttrs &attrs,
  int cores,
  Verbosity v
) : totalCost{0}, totalInstCount{0}, IL{il},
    partition{p}, dagAttrs{attrs}, numCores{cores}, verbose{verbose} {

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
