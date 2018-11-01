#pragma once

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DerivedUser.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/IRBuilder.h"

#include "LoopDependenceInfoForParallelizer.hpp"
#include "PDG.hpp"
#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "PDGAnalysis.hpp"
#include "Parallelization.hpp"
#include "HeuristicsPass.hpp"
#include "ParallelizationTechnique.hpp"

#include "ChunkerInfo.hpp"

#include <unordered_map>
#include <set>
#include <queue>
#include <deque>

// TODO(angelo): replace with values passed into this library
#define NUM_CORES 4
#define CHUNK_SIZE 8

namespace llvm {

  class DOALL : public ParallelizationTechnique {
    public:

      /*
       * Methods
       */
      DOALL (Module &module, Verbosity v);
      bool apply (LoopDependenceInfoForParallelizer *LDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) override ;
      bool canBeAppliedToLoop (LoopDependenceInfoForParallelizer *LDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) const override ;

    protected:

      /*
       * Fields
       */
      Function *dispatcher;

      /*
       * Environment overrides
       */
      void createEnvironment (LoopDependenceInfoForParallelizer *LDI) override ;
      void propagateLiveOutEnvironment (LoopDependenceInfoForParallelizer *LDI) override ;

      /*
       * Import methods of the base class that we overload.
       */
      using ParallelizationTechnique::cloneSequentialLoop ;

      /*
       * Methods
       */
      std::unique_ptr<ChunkerInfo> createChunkingFuncAndArgs (LoopDependenceInfoForParallelizer *LDI, Parallelization &par);

      void cloneSequentialLoop (
          LoopDependenceInfoForParallelizer *LDI, 
          std::unique_ptr<ChunkerInfo> &chunker
          );

      void reproducePreEnv(LoopDependenceInfoForParallelizer *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker);
      void mapOriginLoopValueUses(LoopDependenceInfoForParallelizer *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker);
      void reduceOriginIV(LoopDependenceInfoForParallelizer *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker, ScalarEvolution &SE);
      void createOuterLoop(LoopDependenceInfoForParallelizer *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker);
      void alterInnerLoopToIterateChunks(LoopDependenceInfoForParallelizer *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker);
      void storePostEnvironment(LoopDependenceInfoForParallelizer *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker);

      void addChunkFunctionExecutionAsideOriginalLoop (LoopDependenceInfoForParallelizer *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker);
      Value *createEnvArray (LoopDependenceInfoForParallelizer *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker, IRBuilder<> entryBuilder, IRBuilder<> parBuilder);
  };

}
