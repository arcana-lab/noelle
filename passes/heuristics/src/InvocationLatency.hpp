#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "SCC.hpp"

using namespace std;

namespace llvm {

  class InvocationLatency {
    public:
      uint64_t latencyPerInvocation (SCC *scc);

      uint64_t latencyPerInvocation (std::set<std::set<SCC *> *> &subsets);

      uint64_t latencyPerInvocation (Instruction *inst);

      uint64_t queueLatency (Value *queueVal);

    private:
      std::unordered_map<Function *, uint64_t> funcToCost;
      std::unordered_map<Value *, uint64_t> queueValToCost;
      std::unordered_map<SCC *, uint64_t> sccToCost;
      std::unordered_map<SCC *, std::set<Value *>> sccToExternals;
  };
}
