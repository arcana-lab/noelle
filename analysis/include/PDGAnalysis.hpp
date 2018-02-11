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
      virtual ~PDGAnalysis();

      bool doInitialization (Module &M) override ;

      void getAnalysisUsage(AnalysisUsage &AU) const override ;

      bool runOnModule (Module &M) override ;

      PDG * getPDG () ;

      void constructEdgesFromUseDefs (Module &M);
      
      void constructEdgesFromAliases (Module &M);

    private:
      PDG *programDependenceGraph;
      std::map<Function *, AAResults *> aaResults;

      void addEdgeFromMemoryAlias(Function &, AAResults *, Instruction *, Instruction *, bool storePair);
      void addEdgeFromFunctionModRef(Function &, AAResults *, StoreInst *, CallInst *);
      void addEdgeFromFunctionModRef(Function &, AAResults *, LoadInst *, CallInst *);

      void iterateInstForStoreAliases(Function &, AAResults *, Instruction &);
      void iterateInstForLoadAliases(Function &, AAResults *, Instruction &);

      void iterateInstForModRef(Function &, AAResults *, CallInst &);

  };
}
