#include "SmallestSizePartitionAnalysis.hpp"

/*
 * Prioritize merge that best lowers overall cost without yielding a too costly partition
 */
void llvm::SmallestSizePartitionAnalysis::checkIfShouldMerge (int sA, int sB) {
  // errs() << "Checking to see if can merge " << sA << " with " << sB << "\n";
  if (!partition.canMergeSubsets(sA, sB)) return ;
  // errs() << "Trying to merge " << sA << " with " << sB << "\n";

  /*
   * Determine cost of merge
   */
  auto current = subsetCost[sA] + subsetCost[sB];
  auto insts = subsetInstCount[sA] + subsetInstCount[sB];
  std::set<std::set<SCC *> *> subsets = {
    &(partition.subsetOfID(sA)->SCCs),
    &(partition.subsetOfID(sB)->SCCs)
  };
  uint64_t merge = IL.latencyPerInvocation(subsets);
  uint64_t lowered = current - merge;

  if (merge > totalCost / 1 || partition.subsets.size() == numCores) return ;

  /*
   * Only merge if it best lowers cost
   */
  if (lowered < loweredCost) return ;

  /*
   * Only merge if it is the smallest of equally cost effective merges
   */
  if (lowered == loweredCost && insts > instCount) return ;

  // errs() << "\twill lower merge by " << loweredCost << "\n";
  minSubsetA = sA;
  minSubsetB = sB;
  loweredCost = lowered;
  instCount = insts;
  mergedSubsetCost = merge;
};

