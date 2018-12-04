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

      PDG * getFunctionPDG (Function &F) ;

      PDG * getPDG () ;

    private:
      PDG *programDependenceGraph;
      std::set<std::string> memorylessFunctionNames;
      std::set<Function *> CGUnderMain;
      std::set<GlobalValue *> primitiveArrayGlobals;

      template <class InstI, class InstJ>
      void addEdgeFromMemoryAlias(PDG *, Function &, AAResults &, InstI *, InstJ *, bool WAW);

      void addEdgeFromFunctionModRef(PDG *, Function &, AAResults &, StoreInst *, CallInst *);
      void addEdgeFromFunctionModRef(PDG *, Function &, AAResults &, LoadInst *, CallInst *);
      void addEdgeFromFunctionModRef(PDG *, Function &, AAResults &, CallInst *, CallInst *);

      void iterateInstForStoreAliases(PDG *, Function &, AAResults &, StoreInst *);
      void iterateInstForLoadAliases(PDG *, Function &, AAResults &, LoadInst *);
      void iterateInstForModRef(PDG *, Function &, AAResults &, CallInst &);

      void constructEdgesFromUseDefs (PDG *pdg);
      void constructEdgesFromAliases (PDG *pdg, Module &M);
      void constructEdgesFromControl (PDG *pdg, Module &M);
      void constructEdgesFromAliasesForFunction (PDG *pdg, Function &F, AAResults &AA);
      void constructEdgesFromControlForFunction (PDG *pdg, Function &F, PostDominatorTree &postDomTree);

      void collectCGUnderFunctionMain (Module &M);
      void removeEdgesNotUsedByParSchemes (PDG *pdg);

      void collectPrimitiveArrayGlobalValues (Module &M);
      bool isOnlyUsedByNonAddrValues (std::set<Instruction *> checked, Instruction *I);

      void collectMemorylessFunctions (Module &M);
      bool edgeIsNotLoopCarriedMemoryDependency (DGEdge<Value> *edge);
      bool edgeIsOnKnownMemorylessFunction (DGEdge<Value> *edge);
      bool areGEPIndicesConstantOrIV (GetElementPtrInst *gep);
      bool canPrecedeInCurrentIteration (Instruction *from, Instruction *to);
  };
}
