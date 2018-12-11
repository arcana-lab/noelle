#include "../include/Heuristics.hpp"

using namespace llvm;

void Heuristics::adjustParallelizationPartitionForDSWP (
  SCCDAGPartition *partition,
  SCCDAGAttrs &attrs,
  uint64_t numThreads,
  Verbosity verbose
) {
  // smallestSizeMergePartition(*partition, attrs, idealThreads, verbose);
  minMaxMergePartition(*partition, attrs, numThreads, verbose);
}

void Heuristics::minMaxMergePartition (
  SCCDAGPartition &partition,
  SCCDAGAttrs &attrs,
  uint64_t numThreads,
  Verbosity verbose
) {
  auto modified = false;
  MinMaxSizePartitionAnalysis PCA(invocationLatency, partition, attrs, numThreads, verbose);
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
  uint64_t numThreads,
  Verbosity verbose
) {
  auto modified = false;
  SmallestSizePartitionAnalysis PCA(invocationLatency, partition, attrs, numThreads, verbose);
  do {
    modified = false;

    PCA.resetCandidateSubsetInfo();
    PCA.traverseAllPartitionSubsets();
    PCA.printCandidate(errs());
    modified = PCA.mergeCandidateSubsets();
  } while (modified);
}
