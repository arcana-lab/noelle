/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
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
#ifndef NOELLE_SRC_TOOLS_PARALLELIZATION_TECHNIQUE_PARALLELIZATIONTECHNIQUE_H_
#define NOELLE_SRC_TOOLS_PARALLELIZATION_TECHNIQUE_PARALLELIZATIONTECHNIQUE_H_
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "noelle/core/Noelle.hpp"
#include "noelle/core/LoopDependenceInfo.hpp"
#include "Heuristics.hpp"
#include "noelle/core/Task.hpp"
#include "noelle/core/Hot.hpp"
#include "noelle/core/PDGPrinter.hpp"
#include "noelle/core/SubCFGs.hpp"

namespace arcana::noelle {

class ParallelizationTechnique {
public:
  /*
   * Constructor.
   */
  ParallelizationTechnique(Noelle &n);

  Value *getEnvArray(void) const;

  uint32_t getIndexOfEnvironmentVariable(uint32_t id) const;

  BasicBlock *getParLoopEntryPoint(void) const;

  BasicBlock *getParLoopExitPoint(void) const;

  /*
   * Apply the parallelization technique to the loop LDI.
   */
  virtual bool apply(LoopDependenceInfo *LDI, Heuristics *h) = 0;

  /*
   * Can the current parallelization technique be applied to parallelize loop
   * LDI?
   */
  virtual bool canBeAppliedToLoop(LoopDependenceInfo *LDI,
                                  Heuristics *h) const = 0;

  virtual uint32_t getMinimumNumberOfIdleCores(void) const = 0;

  virtual std::string getName(void) const = 0;

  virtual Transformation getParallelizationID(void) const = 0;

  /*
   * Destructor.
   */
  virtual ~ParallelizationTechnique();

protected:
  /*
   * Generate empty tasks.
   *
   * Each task will be composed by the following empty basic blocks:
   * - an entry basic block, which is mapped to the pre-header of the loop LDI
   * - an exit block, which is the only basic block that will exit the task
   * - one basic block per loop exit, which will jump to the exit block
   */
  virtual void addPredecessorAndSuccessorsBasicBlocksToTasks(
      LoopDependenceInfo *LDI,
      std::vector<Task *> taskStructs);

  /*
   * Loop's environment
   */
  void initializeEnvironmentBuilder(LoopDependenceInfo *LDI,
                                    std::set<uint32_t> nonReducableVars);

  void initializeEnvironmentBuilder(LoopDependenceInfo *LDI,
                                    std::set<uint32_t> simpleVars,
                                    std::set<uint32_t> reducableVars);

  void initializeEnvironmentBuilder(
      LoopDependenceInfo *LDI,
      std::function<bool(uint32_t variableID, bool isLiveOut)>
          shouldThisVariableBeReduced);

  void initializeEnvironmentBuilder(
      LoopDependenceInfo *LDI,
      std::function<bool(uint32_t variableID, bool isLiveOut)>
          shouldThisVariableBeReduced,
      std::function<bool(uint32_t variableID, bool isLiveOut)>
          shouldThisVariableBeSkipped);

  void initializeLoopEnvironmentUsers(void);

  void allocateEnvironmentArray(LoopDependenceInfo *LDI);

  void populateLiveInEnvironment(LoopDependenceInfo *LDI);

  virtual BasicBlock *performReductionToAllReducableLiveOutVariables(
      LoopDependenceInfo *LDI,
      Value *numberOfThreadsExecuted);

  /*
   * Task helpers for manipulating loop body clones
   */
  virtual void cloneSequentialLoop(LoopDependenceInfo *LDI, int taskIndex);
  virtual void cloneSequentialLoopSubset(LoopDependenceInfo *LDI,
                                         int taskIndex,
                                         std::set<Instruction *> subset);

  void cloneMemoryLocationsLocallyAndRewireLoop(LoopDependenceInfo *LDI,
                                                int taskIndex);

  std::unordered_map<InductionVariable *, Value *> cloneIVStepValueComputation(
      LoopDependenceInfo *LDI,
      int taskIndex,
      IRBuilder<> &insertBlock);

  void adjustStepValueOfPointerTypeIVToReflectPointerArithmetic(
      std::unordered_map<InductionVariable *, Value *> clonedStepValueMap,
      IRBuilder<> &insertBlock);

  /*
   * Task helpers for environment usage
   */
  virtual void generateCodeToLoadLiveInVariables(LoopDependenceInfo *LDI,
                                                 int taskIndex);

  virtual void generateCodeToStoreLiveOutVariables(LoopDependenceInfo *LDI,
                                                   int taskIndex);

  Instruction *
  fetchOrCreatePHIForIntermediateProducerValueOfReducibleLiveOutVariable(
      LoopDependenceInfo *LDI,
      int taskIndex,
      int envID,
      BasicBlock *insertBasicBlock,
      DominatorSummary &taskDS);

  void generateCodeToStoreExitBlockIndex(LoopDependenceInfo *LDI,
                                         int taskIndex);

  std::set<BasicBlock *> determineLatestPointsToInsertLiveOutStore(
      LoopDependenceInfo *LDI,
      int taskIndex,
      Instruction *liveOut,
      bool isReduced,
      DominatorSummary &taskDS);

  void setReducableVariablesToBeginAtIdentityValue(LoopDependenceInfo *LDI,
                                                   int taskIndex);

  Value *castToCorrectReducibleType(IRBuilder<> &builder,
                                    Value *value,
                                    Type *targetType);

  virtual BasicBlock *getBasicBlockExecutedOnlyByLastIterationBeforeExitingTask(
      LoopDependenceInfo *LDI,
      uint32_t taskIndex,
      BasicBlock &bb) = 0;

  /*
   * Partition SCCDAG.
   */
  void partitionSCCDAG(LoopDependenceInfo *LDI);

  /*
   * General purpose helpers (that should be moved to parallelization_utils)
   */
  void doNestedInlineOfCalls(Function *F, std::set<CallInst *> &calls);

  float computeSequentialFractionOfExecution(LoopDependenceInfo *LDI) const;

  float computeSequentialFractionOfExecution(
      LoopDependenceInfo *LDI,
      std::function<bool(GenericSCC *scc)> doesItRunSequentially) const;

  virtual void makePRVGsReentrant(void);

  Value *fetchCloneInTask(Task *t, Value *original);

  /*
   * Fields
   */
  Noelle &noelle;
  Verbosity verbose;
  LoopEnvironmentBuilder *envBuilder;

  /*
   * Parallel task related information.
   */
  BasicBlock *entryPointOfParallelizedLoop, *exitPointOfParallelizedLoop;
  std::vector<Task *> tasks;
  uint32_t numTaskInstances;
  std::map<uint64_t, uint64_t> fromTaskIDToUserID;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_TOOLS_PARALLELIZATION_TECHNIQUE_PARALLELIZATIONTECHNIQUE_H_
