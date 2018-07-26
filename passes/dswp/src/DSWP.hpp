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

#include <unordered_map>
#include <set>
#include <queue>
#include <deque>

namespace llvm {

  struct DSWP : public ModulePass {
    public:

      /*
       * Class fields
       */
      static char ID;

      /*
       * Object fields
       */
      Function *stageDispatcher;
      Function *printReachedI, *printPushedP, *printPulledP;
      std::vector<Function *> queuePushes;
      std::vector<Function *> queuePops;
      std::vector<Type *> queueTypes;
      std::vector<Type *> queueElementTypes;
      unordered_map<int, int> queueSizeToIndex;
      FunctionType *stageType;

      /*
       * Methods
       */
      DSWP();
      bool doInitialization (Module &M) override ;
      bool runOnModule (Module &M) override ;
      void getAnalysisUsage (AnalysisUsage &AU) const override ;

    private:

      /*
       * Fields
       */
      bool forceParallelization;
      bool forceNoSCCPartition;
      enum class Verbosity { Disabled, Minimal, Pipeline, Maximal };
      Verbosity verbose;

      /*
       * Methods
       */
      std::vector<DSWPLoopDependenceInfo *> getLoopsToParallelize (Module &M, Parallelization &par);
      bool collectThreadPoolHelperFunctionsAndTypes (Module &M, Parallelization &par) ;
      bool applyDSWP (DSWPLoopDependenceInfo *LDI, Parallelization &par, Heuristics *h) ;
      void mergeSingleSyntacticSugarInstrs (DSWPLoopDependenceInfo *LDI);
      void clusterSubloops (DSWPLoopDependenceInfo *LDI);
      void mergeBranchesWithoutOutgoingEdges (DSWPLoopDependenceInfo *LDI);
      void mergeTrivialNodesInSCCDAG (DSWPLoopDependenceInfo *LDI);
      void collectSCCDAGInfo (DSWPLoopDependenceInfo *LDI, Heuristics *h);
      void partitionSCCDAG (DSWPLoopDependenceInfo *LDI);
      void estimateCostAndExtentOfParallelismOfSCCs (DSWPLoopDependenceInfo *LDI, Heuristics *h);
      void collectRemovableSCCsByInductionVars (DSWPLoopDependenceInfo *LDI);
      void collectRemovableSCCsBySyntacticSugarInstrs (DSWPLoopDependenceInfo *LDI);
      void collectParallelizableSingleInstrNodes (DSWPLoopDependenceInfo *LDI);
      bool isWorthParallelizing (DSWPLoopDependenceInfo *LDI);
      void addRemovableSCCsToStages (DSWPLoopDependenceInfo *LDI);
      void registerQueue (DSWPLoopDependenceInfo *LDI, StageInfo *fromStage, StageInfo *toStage, Instruction *producer, Instruction *consumer);
      void collectPartitionedSCCQueueInfo (DSWPLoopDependenceInfo *LDI);
      void collectTransitiveCondBrs (DSWPLoopDependenceInfo *LDI, std::set<TerminatorInst *> &bottomLevelBrs, std::set<TerminatorInst *> &descendantCondBrs);
      void trimCFGOfStages (DSWPLoopDependenceInfo *LDI);
      void collectControlQueueInfo (DSWPLoopDependenceInfo *LDI);
      void collectRemovableSCCQueueInfo (DSWPLoopDependenceInfo *LDI);
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
      void storeOutgoingDependentsIntoExternalValues (DSWPLoopDependenceInfo *LDI, IRBuilder<> builder, Parallelization &par);

      /*
       * Pipeline
       */
      void createPipelineStageFromSCCDAGPartition (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo, Parallelization &par);
      void createStagesfromPartitionedSCCs (DSWPLoopDependenceInfo *LDI);
      void createPipelineFromStages (DSWPLoopDependenceInfo *LDI, Parallelization &par);

      /*
       * Recursively inline queue push/pop functions in DSWP Utils and ThreadPool API
       */
      void inlineQueueCalls (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo);

      /*
       * Debug utilities
       */
      void printSCCs (SCCDAG *sccSubgraph);
      void printLoop (Loop *loop);
      void printPartitions (DSWPLoopDependenceInfo *LDI);
      void printStageSCCs (DSWPLoopDependenceInfo *LDI);
      void printStageQueues (DSWPLoopDependenceInfo *LDI);
      void printEnv (DSWPLoopDependenceInfo *LDI);
  };

}
