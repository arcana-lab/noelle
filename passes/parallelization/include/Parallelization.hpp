#pragma once

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/AssumptionCache.h"

using namespace llvm;

namespace llvm {
  struct Parallelization : public ModulePass {
    public:
      static char ID;
      IntegerType *int1, *int8, *int16, *int32, *int64;

      Parallelization();

      virtual ~Parallelization();

      bool doInitialization (Module &M) override ;

      void getAnalysisUsage(AnalysisUsage &AU) const override ;

      bool runOnModule (Module &M) override ;

      void cacheInformation (
        Module *module,
        std::unordered_map<Function *, LoopInfo *> &loopInfo,
        std::unordered_map<Function *, DominatorTree *> &domTree,
        std::unordered_map<Function *, PostDominatorTree *> &postDomTree,
        std::unordered_map<Function *, ScalarEvolution *> &scalarEvolution
        );

      std::vector<Function *> * getModuleFunctionsReachableFrom (Module *module, Function *startingPoint);

      std::vector<Loop *> * getModuleLoops (Module *module, std::unordered_map<Function *, LoopInfo *> &loopsInformation);

      void linkParallelizedLoopToOriginalFunction (Module *module, BasicBlock *originalPreHeader, BasicBlock *startOfParallelizedLoopWithinOriginalFunction);

      // TODO
      Function * createFunctionForTheLoopBody ();
  };
}
