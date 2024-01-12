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

#include "noelle/core/SystemHeaders.hpp"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "noelle/core/Noelle.hpp"
#include "noelle/core/LoopContent.hpp"
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

  virtual Value *getEnvArray(void) const;

  virtual uint32_t getIndexOfEnvironmentVariable(uint32_t id) const;

  virtual BasicBlock *getParLoopEntryPoint(void) const;

  virtual BasicBlock *getParLoopExitPoint(void) const;

  /*
   * Apply the parallelization technique to the loop @loopContent.
   */
  virtual bool apply(LoopContent *loopContent, Heuristics *h) = 0;

  /*
   * Can the current parallelization technique be applied to parallelize loop
   * @loopContent?
   */
  virtual bool canBeAppliedToLoop(LoopContent *loopContent,
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
   * - an entry basic block, which is mapped to the pre-header of the loop
   * @loopContent
   * - an exit block, which is the only basic block that will exit the task
   * - one basic block per loop exit, which will jump to the exit block
   */
  virtual void addPredecessorAndSuccessorsBasicBlocksToTasks(
      LoopContent *loopContent,
      std::vector<Task *> taskStructs);

  /*
   * Loop's environment
   */
  virtual void initializeEnvironmentBuilder(
      LoopContent *loopContent,
      std::set<uint32_t> nonReducableVars);

  virtual void initializeEnvironmentBuilder(LoopContent *loopContent,
                                            std::set<uint32_t> simpleVars,
                                            std::set<uint32_t> reducableVars);

  virtual void initializeEnvironmentBuilder(
      LoopContent *loopContent,
      std::function<bool(uint32_t variableID, bool isLiveOut)>
          shouldThisVariableBeReduced);

  virtual void initializeEnvironmentBuilder(
      LoopContent *loopContent,
      std::function<bool(uint32_t variableID, bool isLiveOut)>
          shouldThisVariableBeReduced,
      std::function<bool(uint32_t variableID, bool isLiveOut)>
          shouldThisVariableBeSkipped);

  virtual void initializeLoopEnvironmentUsers(void);

  virtual void allocateEnvironmentArray(LoopContent *loopContent);

  virtual void populateLiveInEnvironment(LoopContent *loopContent);

  virtual BasicBlock *performReductionToAllReducableLiveOutVariables(
      LoopContent *loopContent,
      Value *numberOfThreadsExecuted);

  /*
   * Task helpers for manipulating loop body clones
   */
  virtual void cloneSequentialLoop(LoopContent *loopContent, int taskIndex);
  virtual void cloneSequentialLoopSubset(LoopContent *loopContent,
                                         int taskIndex,
                                         std::set<Instruction *> subset);

  virtual void cloneMemoryLocationsLocallyAndRewireLoop(
      LoopContent *loopContent,
      int taskIndex);

  virtual std::unordered_map<InductionVariable *, Value *>
  cloneIVStepValueComputation(LoopContent *loopContent,
                              int taskIndex,
                              IRBuilder<> &insertBlock);

  virtual void adjustStepValueOfPointerTypeIVToReflectPointerArithmetic(
      std::unordered_map<InductionVariable *, Value *> clonedStepValueMap,
      IRBuilder<> &insertBlock);

  /*
   * Task helpers for environment usage
   */
  virtual void generateCodeToLoadLiveInVariables(LoopContent *loopContent,
                                                 int taskIndex);

  virtual void generateCodeToStoreLiveOutVariables(LoopContent *loopContent,
                                                   int taskIndex);

  virtual Instruction *
  fetchOrCreatePHIForIntermediateProducerValueOfReducibleLiveOutVariable(
      LoopContent *loopContent,
      int taskIndex,
      int envID,
      BasicBlock *insertBasicBlock,
      DominatorSummary &taskDS);

  virtual void generateCodeToStoreExitBlockIndex(LoopContent *loopContent,
                                                 int taskIndex);

  virtual std::set<BasicBlock *> determineLatestPointsToInsertLiveOutStore(
      LoopContent *loopContent,
      int taskIndex,
      Instruction *liveOut,
      bool isReduced,
      DominatorSummary &taskDS);

  virtual void setReducableVariablesToBeginAtIdentityValue(
      LoopContent *loopContent,
      int taskIndex);

  virtual Value *castToCorrectReducibleType(IRBuilder<> &builder,
                                            Value *value,
                                            Type *targetType);

  virtual BasicBlock *getBasicBlockExecutedOnlyByLastIterationBeforeExitingTask(
      LoopContent *loopContent,
      uint32_t taskIndex,
      BasicBlock &bb) = 0;

  /*
   * General purpose helpers (that should be moved to parallelization_utils)
   */
  virtual void doNestedInlineOfCalls(Function *F, std::set<CallInst *> &calls);

  virtual float computeSequentialFractionOfExecution(
      LoopContent *loopContent) const;

  virtual float computeSequentialFractionOfExecution(
      LoopContent *loopContent,
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
