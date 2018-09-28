#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "SCC.hpp"
#include "SCCDAGPartition.hpp"
#include "SCCDAGAttrs.hpp"

using namespace std;

namespace llvm {

  class Heuristics {
    public:

      /*
       * Methods
       */
      uint64_t latencyPerInvocation (SCC *scc);

      uint64_t latencyPerInvocation (SCCDAGAttrs &sccdagAttrs, std::set<std::set<SCC *> *> &subsets);

      uint64_t latencyPerInvocation (Instruction *inst);

      uint64_t queueLatency (Value *queueVal);

      void adjustParallelizationPartitionForDSWP (SCCDAGPartition &partition, SCCDAGAttrs &sccdagAttrs, uint64_t idealThreads);

    private:
      std::unordered_map<Function *, uint64_t> funcToCost;
      std::unordered_map<Value *, uint64_t> queueValToCost;
  };

}
