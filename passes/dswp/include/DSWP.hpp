/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"
#include "DSWPLoopDependenceInfo.hpp"
#include "PDG.hpp"
#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "PDGAnalysis.hpp"
#include "Parallelization.hpp"
#include "HeuristicsPass.hpp"
#include "ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences.hpp"
#include "TaskExecutionDSWP.hpp"

namespace llvm {

  class DSWP : public ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences {
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

      int coresPerLoopOverride;
  };

}
