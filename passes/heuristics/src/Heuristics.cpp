#include "../include/Heuristics.hpp"

using namespace llvm;

void Heuristics::adjustParallelizationPartitionForDSWP (
  SCCDAGPartition &partition,
  uint64_t idealThreads
) {
  smallestSizeMergePartition(partition, idealThreads);
}

void Heuristics::minMaxMergePartition (
  SCCDAGPartition &partition,
  uint64_t idealThreads
) {
  auto modified = false;
  do {
    modified = false;

    MinMaxSizePartitionAnalysis PCA(invocationLatency, partition);
    PCA.resetCandidateSubsetInfo();
    PCA.traverseAllPartitionSubsets();

    modified = PCA.mergeCandidateSubsets();
  } while (modified);
}

void Heuristics::smallestSizeMergePartition (
  SCCDAGPartition &partition,
  uint64_t idealThreads
) {
  auto modified = false;
  do {
    modified = false;

    SmallestSizePartitionAnalysis PCA(invocationLatency, partition);
    PCA.resetCandidateSubsetInfo();
    PCA.traverseAllPartitionSubsets();

    modified = PCA.mergeCandidateSubsets();
  } while (modified);
}
