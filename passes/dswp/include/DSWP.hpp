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
#include "PipelineInfo.hpp"
#include "PDG.hpp"
#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "PDGAnalysis.hpp"
#include "Parallelization.hpp"
#include "HeuristicsPass.hpp"
#include "ParallelizationTechnique.hpp"

#include <unordered_map>
#include <set>
#include <queue>
#include <deque>

namespace llvm {

  class DSWP : public ParallelizationTechnique {
    public:

      /*
       * Object fields
       */
      Function *stageDispatcher;
      FunctionType *stageType;

      /*
       * Methods
       */
      DSWP (Module &module, bool forceParallelization, bool enableSCCMerging, Verbosity v);
      bool apply (LoopDependenceInfo *LDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) override ;
      bool canBeAppliedToLoop (LoopDependenceInfo *baseLDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) const override ;

    private:

      /*
       * Fields
       */
      bool forceParallelization;
      bool enableMergingSCC;

      /*
       * Environment overrides
       */
      void createEnvironment (LoopDependenceInfo *LDI);
      void propagateLiveOutEnvironment (LoopDependenceInfo *LDI);

      /*
       * Methods
       */
      void clusterSubloops (DSWPLoopDependenceInfo *LDI);
      void partitionSCCDAG (DSWPLoopDependenceInfo *LDI, Heuristics *h);
      bool isWorthParallelizing (DSWPLoopDependenceInfo *LDI);
      void addRemovableSCCsToStages (DSWPLoopDependenceInfo *LDI);
      void registerQueue (Parallelization &par, DSWPLoopDependenceInfo *LDI, StageInfo *fromStage, StageInfo *toStage, Instruction *producer, Instruction *consumer);
      void collectPartitionedSCCQueueInfo (Parallelization &par, DSWPLoopDependenceInfo *LDI);
      void collectTransitiveCondBrs (DSWPLoopDependenceInfo *LDI, std::set<TerminatorInst *> &bottomLevelBrs, std::set<TerminatorInst *> &descendantCondBrs);
      void trimCFGOfStages (DSWPLoopDependenceInfo *LDI);
      void collectControlQueueInfo (Parallelization &par, DSWPLoopDependenceInfo *LDI);
      void collectRemovableSCCQueueInfo (Parallelization &par, DSWPLoopDependenceInfo *LDI);
      void collectPreLoopEnvInfo (DSWPLoopDependenceInfo *LDI);
      void collectPostLoopEnvInfo (DSWPLoopDependenceInfo *LDI);
      void configureDependencyStorage (DSWPLoopDependenceInfo *LDI, Parallelization &par);
      void collectStageAndQueueInfo (DSWPLoopDependenceInfo *LDI, Parallelization &par);
      void createInstAndBBForSCC (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo);
      void loadAndStoreEnv (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo, Parallelization &par);
      void loadAllQueuePointersInEntry (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo, Parallelization &par) ;
      void popValueQueues (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo, Parallelization &par);
      void pushValueQueues (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo, Parallelization &par);
      void remapOperandsOfInstClones (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo);
      void remapControlFlow (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo);
      Value * createEnvArrayFromStages (DSWPLoopDependenceInfo *LDI, IRBuilder<> funcBuilder, IRBuilder<> builder, Parallelization &par) ;
      Value * createStagesArrayFromStages (DSWPLoopDependenceInfo *LDI, IRBuilder<> funcBuilder, Parallelization &par);
      Value * createQueueSizesArrayFromStages (DSWPLoopDependenceInfo *LDI, IRBuilder<> funcBuilder, Parallelization &par);

      /*
       * Pipeline
       */
      void createPipelineStageFromSCCDAGPartition (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo, Parallelization &par);
      void createStagesFromPartitionedSCCs (DSWPLoopDependenceInfo *LDI);
      void createPipelineFromStages (DSWPLoopDependenceInfo *LDI, Parallelization &par);

      /*
       * Recursively inline queue push/pop functions in DSWP Utils and ThreadPool API
       */
      void inlineQueueCalls (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo);

      /*
       * Debug utilities
       */
      void printStageSCCs (DSWPLoopDependenceInfo *LDI);
      void printStageQueues (DSWPLoopDependenceInfo *LDI);
      void printEnv (DSWPLoopDependenceInfo *LDI);
      void printPartition (DSWPLoopDependenceInfo *LDI);
  };

}
