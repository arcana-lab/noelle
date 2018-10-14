#include "PartitionCostAnalysis.hpp"

llvm::PartitionCostAnalysis::PartitionCostAnalysis (
  InvocationLatency &il,
  SCCDAGPartition &p
) : totalCost{0}, totalInstCount{0}, IL{il}, partition{p} {

  /*
   * Estimate the current latency for executing the pipeline of the current SCCDAG partition once.
   */
  for (auto &subset : partition.subsets) {
    auto subsetID = partition.getSubsetID(subset);

    uint64_t instCount = 0;
    for (auto scc : subset->SCCs) instCount += scc->numInternalNodes();
    std::set<std::set<SCC *> *> subsets = { &subset->SCCs };
    uint64_t cost = IL.latencyPerInvocation(subsets);

    subsetInstCount[subsetID] = instCount;
    subsetCost[subsetID] = cost;

    totalInstCount += instCount;
    totalCost += cost;
  }
}

void llvm::PartitionCostAnalysis::traverseAllPartitionSubsets () {

  /*
   * Collect all subsets of the current SCCDAG partition.
   */
  std::queue<int> subIDToCheck;
  std::set<int> alreadyChecked;
  auto topLevelSubIDs = partition.getSubsetIDsWithNoIncomingEdges();
  for (auto subID : topLevelSubIDs) {
    subIDToCheck.push(subID);
    alreadyChecked.insert(subID);
  }

  while (!subIDToCheck.empty()) {

    auto subID = subIDToCheck.front();
    subIDToCheck.pop();

    /*
     * Check if the current subset has been already tagged to be removed (i.e., merged).
     */
    if (!partition.isValidSubset(subID)) continue ;
    // errs() << "\nTraversing " << subID << "\n";

    /*
     * Check merge criteria on dependents and depth-1 neighbors
     */
    auto dependentIDs = partition.getDependentIDs(subID);
    auto siblingIDs = partition.getSiblingIDs(subID);
    for (auto s : dependentIDs) checkIfShouldMerge(subID, s);
    for (auto s : siblingIDs) checkIfShouldMerge(subID, s);

    /*
     * Add dependent SCCs as well.
     */
    for (auto s : dependentIDs) {
      if (alreadyChecked.find(s) == alreadyChecked.end()){
        subIDToCheck.push(s);
        alreadyChecked.insert(s);
      }
    }
  }
}

void llvm::PartitionCostAnalysis::resetCandidateSubsetInfo () {
  minSubsetA = minSubsetB = -1;
  loweredCost = mergedSubsetCost = 0;
  instCount = totalInstCount;
}

bool llvm::PartitionCostAnalysis::mergeCandidateSubsets () {
  /*
   * Merge partition if one is found; reiterate the merge check on it
   */
  if (minSubsetA != -1) {
    // errs() << "Merging " << minSubsetAID << " with " << minSubsetBID << "\n";
    auto mergedSub = partition.mergeSubsets(minSubsetA, minSubsetB);

    /*
     * Readjust subset cost tracking
     */
    subsetCost[mergedSub] = mergedSubsetCost;
    subsetInstCount[mergedSub] = instCount;
    totalCost -= loweredCost;
    return true;
  }
  return false;
}
