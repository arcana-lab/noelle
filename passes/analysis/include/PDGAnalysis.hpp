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

    private:
      PDG *programDependenceGraph;
      std::map<Function *, AAResults *> aaResults;

      template <class InstI, class InstJ>
      void addEdgeFromMemoryAlias(Function &, AAResults *, InstI *, InstJ *, bool storePair);

      void addEdgeFromFunctionModRef(Function &, AAResults *, StoreInst *, CallInst *);
      void addEdgeFromFunctionModRef(Function &, AAResults *, LoadInst *, CallInst *);
      void addEdgeFromFunctionModRef(Function &, AAResults *, CallInst *, CallInst *);

      void iterateInstForStoreAliases(Function &, AAResults *, StoreInst *);
      void iterateInstForLoadAliases(Function &, AAResults *, LoadInst *);

      void iterateInstForModRef(Function &, AAResults *, CallInst &);

      void constructEdgesFromUseDefs (Module &M);

      void constructEdgesFromAliases (Module &M);

      void constructEdgesFromControl (Module &M);
      void constructControlEdgesForFunction(Function &F, PostDominatorTree &postDomTree);

      void removeEdgesFromApparentIntraIterationDependencies (Module &M);
      bool checkLoadStoreAliasOnSameGEP (GetElementPtrInst *gep);
      bool instMayPrecede (Value *from, Value *to);
  };
}
