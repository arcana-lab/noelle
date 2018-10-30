#include "MinMaxSizePartitionAnalysis.hpp"

void llvm::MinMaxSizePartitionAnalysis::checkIfShouldMerge (int sA, int sB) {
  errs() << "Checking to merge: " << sA << " " << sB << "\n";
  if (!partition.canMergeSubsets(sA, sB)) return ;
  errs() << "Is possible\n";

  auto current = subsetCost[sA] + subsetCost[sB];
  auto insts = subsetInstCount[sA] + subsetInstCount[sB];
  std::set<std::set<SCC *> *> subsets = {
    &(partition.subsetOfID(sA)->SCCs),
    &(partition.subsetOfID(sB)->SCCs)
  };
  uint64_t merge = IL.latencyPerInvocation(subsets);
  uint64_t lowered = current - merge;

  if (partition.subsets.size() == numCores) return ;

  errs() << "Current cost of both: " << current << "\n";
  errs() << "Inst count of both versus current min: " << insts << " " << instCount << "\n";
  errs() << "Merged together cost versus min: " << merge << " " << mergedSubsetCost << "\n";

  /*
   * Only merge if it is the cheapest of the merges
   */
  if (merge > mergedSubsetCost) return ;

  /*
   * Only merge if it is the smallest of equally cost effective merges
   */
  if (merge == mergedSubsetCost && insts > instCount) return ;

  minSubsetA = sA;
  minSubsetB = sB;
  loweredCost = lowered;
  instCount = insts;
  mergedSubsetCost = merge;
};
