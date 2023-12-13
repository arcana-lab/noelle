/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef NOELLE_SRC_TOOLS_DSWP_H_
#define NOELLE_SRC_TOOLS_DSWP_H_

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/PDG.hpp"
#include "noelle/core/SCC.hpp"
#include "noelle/core/SCCDAG.hpp"
#include "noelle/core/PDGAnalysis.hpp"
#include "noelle/core/Noelle.hpp"
#include "HeuristicsPass.hpp"
#include "noelle/tools/ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences.hpp"
#include "DSWPTask.hpp"
#include "noelle/core/LoopContent.hpp"

namespace arcana::noelle {

class DSWP
  : public ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences {
public:
  /*
   * Methods
   */
  DSWP(Noelle &par, bool forceParallelization, bool enableSCCMerging);

  bool apply(LoopContent *LDI, Heuristics *h) override;

  bool canBeAppliedToLoop(LoopContent *LDI, Heuristics *h) const override;

  uint32_t getMinimumNumberOfIdleCores(void) const override;

  std::set<GenericSCC *> getClonableSCCs(SCCDAGAttrs *sccManager,
                                         LoopTree *loopNode) const;

  std::string getName(void) const override;

  Transformation getParallelizationID(void) const override;

protected:
  BasicBlock *getBasicBlockExecutedOnlyByLastIterationBeforeExitingTask(
      LoopContent *LDI,
      uint32_t taskIndex,
      BasicBlock &bb) override;

private:
  uint32_t minCores;

  /*
   * CLI Options
   */
  bool enableMergingSCC;

  /*
   * Stores new pipeline execution
   */
  std::unordered_map<SCC *, DSWPTask *> sccToStage;
  std::vector<std::unique_ptr<QueueInfo>> queues;

  /*
   * Types for arrays storing dependencies and stages
   */
  ArrayType *queueArrayType;
  ArrayType *stageArrayType;
  Value *zeroIndexForBaseArray;

  /*
   * TODO: Dominator information to be fetched from LDI
   */
  DominatorSummary *originalFunctionDS;

  /*
   * Dispatcher
   */
  Function *taskDispatcher;

  std::set<GenericSCC *> clonableSCCs;

  /*
   * Pipeline
   */
  void partitionSCCDAG(LoopContent *LDI, Heuristics *h);
  void clusterSubloops(LoopContent *LDI);
  void generateStagesFromPartitionedSCCs(LoopContent *LDI);
  void addClonableSCCsToStages(LoopContent *LDI);
  bool isCompleteAndValidStageStructure(LoopContent *LDI) const;
  void generateLoopSubsetForStage(LoopContent *LDI, int taskIndex);
  void generateLoadsOfQueuePointers(Noelle &par, int taskIndex);
  void popValueQueues(LoopContent *LDI, Noelle &par, int taskIndex);
  void pushValueQueues(LoopContent *LDI, Noelle &par, int taskIndex);
  void createPipelineFromStages(LoopContent *LDI, Noelle &par);
  Value *createStagesArrayFromStages(LoopContent *LDI,
                                     IRBuilder<> funcBuilder,
                                     Noelle &par);
  Value *createQueueSizesArrayFromStages(LoopContent *LDI,
                                         IRBuilder<> funcBuilder,
                                         Noelle &par);

  bool canBeCloned(GenericSCC *scc) const;

  /*
   * Recursively inline queue push/pop functions in DSWP Utils and ThreadPool
   * API
   */
  void inlineQueueCalls(int taskIndex);

  /*
   * Information collection helpers
   */
  void collectDataAndMemoryQueueInfo(LoopContent *LDI, Noelle &par);
  void collectControlQueueInfo(LoopContent *LDI, Noelle &par);
  std::set<Task *> collectTransitivelyControlledTasks(
      LoopContent *LDI,
      DGNode<Value> *conditionalBranchNodei);
  void registerQueue(Noelle &par,
                     LoopContent *LDI,
                     DSWPTask *fromStage,
                     DSWPTask *toStage,
                     Instruction *producer,
                     Instruction *consumer,
                     bool isMemoryDependence);
  void collectLiveInEnvInfo(LoopContent *LDI);
  void collectLiveOutEnvInfo(LoopContent *LDI);
  bool areQueuesAcyclical() const;

  /*
   * Debug utilities
   */
  void printStageSCCs(LoopContent *LDI) const;
  void printStageQueues(LoopContent *LDI) const;
  void printEnv(LoopContent *LDI) const;
  void writeStageGraphsAsDot(LoopContent &LDI) const;
  void writeStageQueuesAsDot(const LoopContent &LDI) const;
  void printStageClonedValues(const LoopContent &LDI, int taskIndex) const;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_TOOLS_DSWP_H_
