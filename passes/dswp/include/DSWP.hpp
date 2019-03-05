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
#include "PDG.hpp"
#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "PDGAnalysis.hpp"
#include "Parallelization.hpp"
#include "HeuristicsPass.hpp"
#include "ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences.hpp"
#include "TaskExecutionDSWP.hpp"
#include "LoopDependenceInfo.hpp"

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
        Verbosity v
      );

      bool apply (
        LoopDependenceInfo *LDI,
        Parallelization &par,
        Heuristics *h
      ) override ;

      bool canBeAppliedToLoop (
        LoopDependenceInfo *LDI,
        Parallelization &par,
        Heuristics *h
      ) const override ;

      void initialize (
        LoopDependenceInfo *LDI,
        Heuristics *h
      );

    private:

      /*
       * CLI Options
       */
      bool forceParallelization;
      bool enableMergingSCC;

      /*
       * Stores new pipeline execution
       */
      unordered_map<SCC *, DSWPTask *> sccToStage;
      std::vector<std::unique_ptr<QueueInfo>> queues;

      /*
       * Types for arrays storing dependencies and stages
       */
      ArrayType *queueArrayType;
      ArrayType *stageArrayType;
      Value *zeroIndexForBaseArray;

      /*
       * Pipeline
       */
      void partitionSCCDAG (LoopDependenceInfo *LDI, Heuristics *h) ;
      void clusterSubloops (LoopDependenceInfo *LDI);
      void generateStagesFromPartitionedSCCs (LoopDependenceInfo *LDI);
      void addRemovableSCCsToStages (LoopDependenceInfo *LDI);
      void generateLoopSubsetForStage (LoopDependenceInfo *LDI, int taskIndex);
      void generateLoadsOfQueuePointers (Parallelization &par, int taskIndex);
      void popValueQueues (Parallelization &par, int taskIndex);
      void pushValueQueues (Parallelization &par, int taskIndex);
      void createPipelineFromStages (LoopDependenceInfo *LDI, Parallelization &par);
      Value * createStagesArrayFromStages (
        LoopDependenceInfo *LDI,
        IRBuilder<> funcBuilder,
        Parallelization &par
      );
      Value * createQueueSizesArrayFromStages (
        LoopDependenceInfo *LDI,
        IRBuilder<> funcBuilder,
        Parallelization &par
      );

      /*
       * Recursively inline queue push/pop functions in DSWP Utils and ThreadPool API
       */
      void inlineQueueCalls (int taskIndex);

      /*
       * Information collection helpers
       */
      void collectDataQueueInfo (LoopDependenceInfo *LDI, Parallelization &par);
      void collectControlQueueInfo (LoopDependenceInfo *LDI, Parallelization &par);
      void registerQueue (
        Parallelization &par,
        LoopDependenceInfo *LDI,
        DSWPTask *fromStage,
        DSWPTask *toStage,
        Instruction *producer,
        Instruction *consumer
      );
      void trimCFGOfStages (LoopDependenceInfo *LDI);
      void collectTransitiveCondBrs (
        LoopDependenceInfo *LDI,
        std::set<TerminatorInst *> &bottomLevelBrs,
        std::set<TerminatorInst *> &descendantCondBrs
      );
      void collectLiveInEnvInfo (LoopDependenceInfo *LDI);
      void collectLiveOutEnvInfo (LoopDependenceInfo *LDI);

      /*
       * Debug utilities
       */
      void printStageSCCs (LoopDependenceInfo *LDI) const ;
      void printStageQueues (LoopDependenceInfo *LDI) const ;
      void printEnv (LoopDependenceInfo *LDI) const ;
  };

}
