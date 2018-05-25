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

#include "LoopDependenceInfo.hpp"

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

      std::vector<Function *> * getModuleFunctionsReachableFrom (
        Module *module, 
        Function *startingPoint
        );

      std::vector<LoopDependenceInfo *> * getModuleLoops (
        Module *module, 
        std::function<LoopDependenceInfo * (Function *, PDG *, Loop *, LoopInfo &)> allocationFunction
        );

      void linkParallelizedLoopToOriginalFunction (
        Module *module, 
        BasicBlock *originalPreHeader, 
        BasicBlock *startOfParallelizedLoopWithinOriginalFunction
        );
  };
}
