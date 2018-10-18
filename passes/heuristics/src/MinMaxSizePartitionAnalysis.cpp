#include "MinMaxSizePartitionAnalysis.hpp"

void llvm::MinMaxSizePartitionAnalysis::checkIfShouldMerge (int sA, int sB) {
  if (!partition.canMergeSubsets(sA, sB)) return ;
};
