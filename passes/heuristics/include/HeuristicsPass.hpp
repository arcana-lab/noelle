#pragma once

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "Heuristics.hpp"

using namespace llvm;

namespace llvm {
  struct HeuristicsPass : public ModulePass {
    public:
      static char ID;

      HeuristicsPass();

      bool doInitialization (Module &M) override ;

      void getAnalysisUsage(AnalysisUsage &AU) const override ;

      bool runOnModule (Module &M) override ;

      Heuristics * getHeuristics () ;
  };
}
