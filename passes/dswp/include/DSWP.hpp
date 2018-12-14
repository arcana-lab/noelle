#pragma once

#include "SystemHeaders.hpp"
#include "DSWPLoopDependenceInfo.hpp"
#include "PDG.hpp"
#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "PDGAnalysis.hpp"
#include "Parallelization.hpp"
#include "HeuristicsPass.hpp"
#include "ParallelizationTechnique.hpp"
#include "TaskExecutionDSWP.hpp"

namespace llvm {

  class DSWP : public ParallelizationTechnique {
    public:

      /*
       * Methods
       */
      DSWP (
        Module &module,
        bool forceParallelization,
        bool enableSCCMerging,
        Verbosity v,
        int coresPerOverride
      );

      bool apply (
        LoopDependenceInfo *LDI,
        Parallelization &par,
        Heuristics *h,
        ScalarEvolution &SE
      ) override ;
      bool canBeAppliedToLoop (
        LoopDependenceInfo *baseLDI,
        Parallelization &par,
        Heuristics *h,
        ScalarEvolution &SE
      ) const override ;
      void initialize (
        LoopDependenceInfo *baseLDI,
        Heuristics *h
      );

    private:

      /*
       * Fields
       */
      bool forceParallelization;
      bool enableMergingSCC;

      /*
       * Pipeline
       */
      void partitionSCCDAG (DSWPLoopDependenceInfo *LDI, Heuristics *h) ;
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
        DSWPTask *fromStage,
        DSWPTask *toStage,
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

      /*
       * DSWP specific structures used to determine workers
       */
      std::set<std::set<SCC *> *> *subsets;

      int coresPerLoopOverride;
  };

}
