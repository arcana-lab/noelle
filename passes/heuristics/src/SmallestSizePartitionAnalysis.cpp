#include "SmallestSizePartitionAnalysis.hpp"

/*
 * Prioritize merge that best lowers overall cost without yielding a too costly partition
 */
void llvm::SmallestSizePartitionAnalysis::checkIfShouldMerge (SCCset *sA, SCCset *sB) {
  if (partition.mergeYieldsCycle(sA, sB)) return ;

  /*
   * Determine cost of merge
   */
  auto current = subsetCost[sA] + subsetCost[sB];
  auto insts = subsetInstCount[sA] + subsetInstCount[sB];
  std::set<SCCset *> subsets = { sA, sB };
  uint64_t merge = IL.latencyPerInvocation(&dagAttrs, subsets);
  uint64_t lowered = current - merge;

  if (merge > totalCost / 1 || partition.getSubsets()->size() == numCores) return ;

  /*
   * Only merge if it best lowers cost
   */
  if (lowered < loweredCost) return ;

  /*
   * Only merge if it is the smallest of equally cost effective merges
   */
  if (lowered == loweredCost && insts > instCount) return ;

  minSubsetA = sA;
  minSubsetB = sB;
  loweredCost = lowered;
  instCount = insts;
  mergedSubsetCost = merge;
};

