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

#include "LoopDependenceInfo.hpp"
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
      bool apply (LoopDependenceInfo *LDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) override ;
      bool canBeAppliedToLoop (LoopDependenceInfo *LDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) const override ;

    private:

      /*
       * Fields
       */
      Function *dispatcher;

      /*
       * Methods
       */
      std::unique_ptr<ChunkerInfo> createChunkingFuncAndArgs (LoopDependenceInfo *LDI, Parallelization &par);
      void cloneSequentialLoop(LoopDependenceInfo *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker);
      void reproducePreEnv(LoopDependenceInfo *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker);
      void mapOriginLoopValueUses(LoopDependenceInfo *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker);
      void reduceOriginIV(LoopDependenceInfo *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker, ScalarEvolution &SE);
      void createOuterLoop(LoopDependenceInfo *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker);
      void alterInnerLoopToIterateChunks(LoopDependenceInfo *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker);
      void storePostEnvironment(LoopDependenceInfo *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker);

      void addChunkFunctionExecutionAsideOriginalLoop (LoopDependenceInfo *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker);
      void reducePostEnvironment (LoopDependenceInfo *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker);
      Value *createEnvArray (LoopDependenceInfo *LDI, Parallelization &par, std::unique_ptr<ChunkerInfo> &chunker, IRBuilder<> entryBuilder, IRBuilder<> parBuilder);
  };

}
