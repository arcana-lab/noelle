#include "../include/Heuristics.hpp"

using namespace llvm;

void Heuristics::adjustParallelizationPartitionForDSWP (
  SCCDAGPartition *partition,
  SCCDAGAttrs &attrs,
  uint64_t numThreads
) {
  // smallestSizeMergePartition(*partition, attrs, idealThreads);
  minMaxMergePartition(*partition, attrs, numThreads);
}

void Heuristics::minMaxMergePartition (
  SCCDAGPartition &partition,
  SCCDAGAttrs &attrs,
  uint64_t numThreads
) {
  auto modified = false;
  MinMaxSizePartitionAnalysis PCA(invocationLatency, partition, attrs, numThreads);
  do {
    modified = false;

    PCA.resetCandidateSubsetInfo();
    PCA.traverseAllPartitionSubsets();
    PCA.printCandidate(errs());
    modified = PCA.mergeCandidateSubsets();
  } while (modified);
}

void Heuristics::smallestSizeMergePartition (
  SCCDAGPartition &partition,
  SCCDAGAttrs &attrs,
  uint64_t numThreads
) {
  auto modified = false;
  SmallestSizePartitionAnalysis PCA(invocationLatency, partition, attrs, numThreads);
  do {
    modified = false;

    PCA.resetCandidateSubsetInfo();
    PCA.traverseAllPartitionSubsets();
    modified = PCA.mergeCandidateSubsets();
  } while (modified);
}
