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

  class SmallestSizePartitionAnalysis : public PartitionCostAnalysis {
   public:
    SmallestSizePartitionAnalysis (InvocationLatency &IL, SCCDAGPartition &p)
      : PartitionCostAnalysis{IL, p} {};

    void checkIfShouldMerge (int sA, int sB);
  };
}
