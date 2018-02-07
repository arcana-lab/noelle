#pragma once

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "PDG.hpp"

using namespace llvm;

namespace llvm {
  struct PDGAnalysis : public ModulePass {
    public:
      static char ID;
      PDGAnalysis();

      bool doInitialization (Module &M) override ;

      void getAnalysisUsage(AnalysisUsage &AU) const override ;

      bool runOnModule (Module &M) override ;

      PDG & getPDG () ;

      void constructEdgesFromUseDefs (Module &M);
      
      void constructEdgesFromAliases (Module &M);

    private:
      std::unique_ptr<PDG> programDependenceGraph;
      std::map<Function *, AAResults *> aaResults;

      void addEdgeFromMemoryAlias(Function &, Instruction &, Instruction &);
      void addEdgeFromFunctionModRef(Function &, Instruction &, CallInst &);

      template <class iType>
      void iterateInstForAliases(Function &, Instruction &);

      void iterateInstForModRef(Function &, CallInst &);

  };
}
