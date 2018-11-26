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

#include "DSWPLoopDependenceInfo.hpp"
#include "PDG.hpp"
#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "PDGAnalysis.hpp"
#include "Parallelization.hpp"
#include "HeuristicsPass.hpp"
#include "ParallelizationTechnique.hpp"

#include "TaskExecutionDSWP.hpp"

#include <unordered_map>
#include <set>
#include <queue>
#include <deque>

namespace llvm {

  class DSWP : public ParallelizationTechnique {
    public:

      /*
       * Methods
       */
      DSWP (Module &module, bool forceParallelization, bool enableSCCMerging, Verbosity v);
      bool apply (
        LoopDependenceInfoForParallelizer *LDI,
        Parallelization &par,
        Heuristics *h,
        ScalarEvolution &SE
      ) override ;
      bool canBeAppliedToLoop (
        LoopDependenceInfoForParallelizer *baseLDI,
        Parallelization &par,
        Heuristics *h,
        ScalarEvolution &SE
      ) const override ;

    private:

      /*
       * Fields
       */
      bool forceParallelization;
      bool enableMergingSCC;

      /*
       * Pipeline
       */
      void partitionSCCDAG (DSWPLoopDependenceInfo *LDI, Heuristics *h) const ;
      void clusterSubloops (DSWPLoopDependenceInfo *LDI);
      void generateStagesFromPartitionedSCCs (DSWPLoopDependenceInfo *LDI);
      void addRemovableSCCsToStages (DSWPLoopDependenceInfo *LDI);
      void generateLoopSubsetForStage (DSWPLoopDependenceInfo *LDI, int taskIndex);
      void generateLoadsOfQueuePointers (
        DSWPLoopDependenceInfo *LDI,
        Parallelization &par,
        int taskIndex
      );
      void popValueQueues (
        DSWPLoopDependenceInfo *LDI,
        Parallelization &par,
        int taskIndex
      );
      void pushValueQueues (
        DSWPLoopDependenceInfo *LDI,
        Parallelization &par,
        int taskIndex
      );
      void createPipelineFromStages (DSWPLoopDependenceInfo *LDI, Parallelization &par);
      Value * createStagesArrayFromStages (
        DSWPLoopDependenceInfo *LDI,
        IRBuilder<> funcBuilder,
        Parallelization &par
      );
      Value * createQueueSizesArrayFromStages (
        DSWPLoopDependenceInfo *LDI,
        IRBuilder<> funcBuilder,
        Parallelization &par
      );

      /*
       * Recursively inline queue push/pop functions in DSWP Utils and ThreadPool API
       */
      void inlineQueueCalls (DSWPLoopDependenceInfo *LDI, int taskIndex);

      /*
       * Information collection helpers
       */
      void collectDataQueueInfo (DSWPLoopDependenceInfo *LDI, Parallelization &par);
      void collectControlQueueInfo (DSWPLoopDependenceInfo *LDI, Parallelization &par);
      void registerQueue (
        Parallelization &par,
        DSWPLoopDependenceInfo *LDI,
        DSWPTaskExecution *fromStage,
        DSWPTaskExecution *toStage,
        Instruction *producer,
        Instruction *consumer
      );
      void trimCFGOfStages (DSWPLoopDependenceInfo *LDI);
      void collectTransitiveCondBrs (
        DSWPLoopDependenceInfo *LDI,
        std::set<TerminatorInst *> &bottomLevelBrs,
        std::set<TerminatorInst *> &descendantCondBrs
      );
      void collectLiveInEnvInfo (DSWPLoopDependenceInfo *LDI);
      void collectLiveOutEnvInfo (DSWPLoopDependenceInfo *LDI);

      /*
       * Debug utilities
       */
      void printStageSCCs (DSWPLoopDependenceInfo *LDI) const ;
      void printStageQueues (DSWPLoopDependenceInfo *LDI) const ;
      void printEnv (DSWPLoopDependenceInfo *LDI) const ;
      void printPartition (DSWPLoopDependenceInfo *LDI) const ;
  };

}
