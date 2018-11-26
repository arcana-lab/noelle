#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "SCC.hpp"
#include "SCCDAGAttrs.hpp"

using namespace std;

namespace llvm {

  class InvocationLatency {
    public:
      uint64_t latencyPerInvocation (SCC *scc);
      uint64_t latencyPerInvocation (SCCDAGAttrs *, std::set<std::set<SCC *> *> &subsets);
      uint64_t latencyPerInvocation (Instruction *inst);
      uint64_t queueLatency (Value *queueVal);

      std::set<Value *> &memoizeExternals (SCCDAGAttrs *, SCC *);
      std::set<SCC *> &memoizeParents (SCCDAGAttrs *, SCC *);

    private:
      std::unordered_map<Function *, uint64_t> funcToCost;
      std::unordered_map<Value *, uint64_t> queueValToCost;
      std::unordered_map<SCC *, uint64_t> sccToCost;
      std::unordered_map<SCC *, std::set<Value *>> incomingExternals;
      std::unordered_map<SCC *, std::set<SCC *>> clonableParents;
  };
}
