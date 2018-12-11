#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "SCC.hpp"
#include "SCCDAGPartition.hpp"
#include "SCCDAGAttrs.hpp"

#include "PartitionCostAnalysis.hpp"

using namespace std;

namespace llvm {

  class MinMaxSizePartitionAnalysis : public PartitionCostAnalysis {
   public:  
    MinMaxSizePartitionAnalysis (
      InvocationLatency &IL,
      SCCDAGPartition &p,
      SCCDAGAttrs &attrs,
      int cores,
      Verbosity v
    ) : PartitionCostAnalysis{IL, p, attrs, cores, v} {};

    void checkIfShouldMerge (SCCset *sA, SCCset *sB);
  };
}
