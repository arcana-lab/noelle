#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "SCC.hpp"
#include "SCCDAGPartition.hpp"
#include "InvocationLatency.hpp"

using namespace std;

namespace llvm {

  class PartitionCostAnalysis {
   public:
    PartitionCostAnalysis (InvocationLatency &IL, SCCDAGPartition &p, SCCDAGAttrs &, int numCores);

    void traverseAllPartitionSubsets ();

    virtual void checkIfShouldMerge (SCCset *sA, SCCset *sB) = 0;

    void resetCandidateSubsetInfo ();

    bool mergeCandidateSubsets ();

    void printCandidate (raw_ostream &stream);

    const static std::string prefix;
   protected:

    InvocationLatency &IL;
    SCCDAGPartition &partition;
    SCCDAGAttrs &dagAttrs;
    int numCores;

    uint64_t totalCost;
    uint64_t totalInstCount;
    std::unordered_map<SCCset *, uint64_t> subsetCost;
    std::unordered_map<SCCset *, uint64_t> subsetInstCount;

    SCCset *minSubsetA;
    SCCset *minSubsetB;
    uint64_t loweredCost;
    uint64_t instCount;
    uint64_t mergedSubsetCost;
  };

}
