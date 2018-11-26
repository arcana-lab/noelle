#include "../include/Heuristics.hpp"

using namespace llvm;

Heuristics::Heuristics (int cores) : numCores{cores} {}

void Heuristics::adjustParallelizationPartitionForDSWP (
  SCCDAGPartition *partition,
  SCCDAGAttrs &attrs,
  uint64_t idealThreads
) {
  // smallestSizeMergePartition(*partition, attrs, idealThreads);
  minMaxMergePartition(*partition, attrs, idealThreads);
}

void Heuristics::minMaxMergePartition (
  SCCDAGPartition &partition,
  SCCDAGAttrs &attrs,
  uint64_t idealThreads
) {
  auto modified = false;
  MinMaxSizePartitionAnalysis PCA(invocationLatency, partition, attrs, numCores);
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
  uint64_t idealThreads
) {
  auto modified = false;
  SmallestSizePartitionAnalysis PCA(invocationLatency, partition, attrs, numCores);
  do {
    modified = false;

    PCA.resetCandidateSubsetInfo();
    PCA.traverseAllPartitionSubsets();
    modified = PCA.mergeCandidateSubsets();
  } while (modified);
}
