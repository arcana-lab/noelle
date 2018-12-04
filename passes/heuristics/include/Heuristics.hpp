#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "SCCDAGPartition.hpp"

#include "../src/InvocationLatency.hpp"
#include "../src/PartitionCostAnalysis.hpp"
#include "../src/SmallestSizePartitionAnalysis.hpp"
#include "../src/MinMaxSizePartitionAnalysis.hpp"

using namespace std;

namespace llvm {

  class Heuristics {
    public:

      /*
       * Methods
       */
      void adjustParallelizationPartitionForDSWP (
        SCCDAGPartition *partition,
        SCCDAGAttrs &attrs,
        uint64_t numThreads
      );

     private:

      void minMaxMergePartition (
        SCCDAGPartition &partition,
        SCCDAGAttrs &attrs,
        uint64_t numThreads
      );

      void smallestSizeMergePartition (
        SCCDAGPartition &partition,
        SCCDAGAttrs &attrs,
        uint64_t numThreads
      );

      InvocationLatency invocationLatency;
  };

}
