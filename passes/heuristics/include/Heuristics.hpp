#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "SCC.hpp"

using namespace std;

namespace llvm {

  class Heuristics {
    public:

      /*
       * Methods
       */
      uint64_t latencyPerInvocation (SCC *scc);

      uint64_t latencyPerInvocation (Instruction *inst);

    private:
      std::unordered_map<Function *, uint64_t> funcToCost;
  };

}
