#include "MinMaxSizePartitionAnalysis.hpp"

void llvm::MinMaxSizePartitionAnalysis::checkIfShouldMerge (SCCset *sA, SCCset *sB) {
  std::string subsetStrs = partition.subsetStr(sA) + " " + partition.subsetStr(sB);
  errs() << prefix << "Checking: " << subsetStrs;
  if (partition.mergeYieldsCycle(sA, sB)) {
    errs() << "\n";
    return;
  }
  errs() << " Is possible\n";

  auto current = subsetCost[sA] + subsetCost[sB];
  auto insts = subsetInstCount[sA] + subsetInstCount[sB];
  std::set<SCCset *> subsets = { sA, sB };
  uint64_t merge = IL.latencyPerInvocation(&dagAttrs, subsets);
  uint64_t lowered = current - merge;

  if (partition.getSubsets()->size() == numCores) return ;

  // errs() << prefix << subsetStrs << "\n";
  errs() << prefix << "Lowered cost: " << lowered
    << " Merged cost: " << merge
    << " Instr count: " << insts << "\n";

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
