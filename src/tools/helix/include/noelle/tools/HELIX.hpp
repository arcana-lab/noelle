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
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/LoopDependenceInfo.hpp"
#include "noelle/core/SubCFGs.hpp"
#include "noelle/core/PDG.hpp"
#include "noelle/core/SCC.hpp"
#include "noelle/core/SCCDAG.hpp"
#include "noelle/core/PDGAnalysis.hpp"
#include "noelle/core/Noelle.hpp"
#include "noelle/core/ControlFlowEquivalence.hpp"
#include "noelle/tools/ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences.hpp"
#include "noelle/tools/SequentialSegment.hpp"
#include "noelle/tools/HELIXTask.hpp"
#include "HeuristicsPass.hpp"

namespace llvm::noelle {

class SpilledLoopCarriedDependency;

class HELIX
  : public ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences {
public:
  /*
   * Methods
   */
  HELIX(Noelle &n, bool forceParallelization);

  bool apply(LoopDependenceInfo *LDI, Heuristics *h) override;

  bool canBeAppliedToLoop(LoopDependenceInfo *LDI,
                          Heuristics *h) const override;

  PDG *constructTaskInternalDependenceGraphFromOriginalLoopDG(
      LoopDependenceInfo *LDI,
      PostDominatorTree &postDomTreeOfTaskFunction);

  Function *getTaskFunction(void) const;

  SCC *getTheSequentialSCCThatCreatesTheSequentialPrologue(
      LoopDependenceInfo *LDI) const;

  bool doesHaveASequentialPrologue(LoopDependenceInfo *LDI) const;

  uint32_t getMinimumNumberOfIdleCores(void) const override;

  std::string getName(void) const override;

  virtual ~HELIX();

protected:
  virtual void createParallelizableTask(LoopDependenceInfo *LDI, Heuristics *h);

  virtual bool synchronizeTask(LoopDependenceInfo *LDI,
                               Heuristics *h,
                               HELIXTask *helixTask);

  virtual void invokeParallelizedLoop(LoopDependenceInfo *LDI,
                                      uint64_t numberOfSequentialSegments);

  void spillLoopCarriedDataDependencies(LoopDependenceInfo *LDI,
                                        DataFlowResult *reachabilityDFR,
                                        HELIXTask *helixTask);

  void createLoadsAndStoresToSpilledLCD(
      LoopDependenceInfo *LDI,
      DataFlowResult *reachabilityDFR,
      std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
      SpilledLoopCarriedDependency *spill,
      Value *spillEnvPtr);

  void insertStoresToSpilledLCD(
      LoopDependenceInfo *LDI,
      std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
      SpilledLoopCarriedDependency *spill,
      Value *spillEnvPtr);

  void defineFrontierForLoadsToSpilledLCD(
      LoopDependenceInfo *LDI,
      DataFlowResult *reachabilityDFR,
      std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
      SpilledLoopCarriedDependency *spill,
      DominatorSummary *originalLoopDS,
      std::unordered_set<BasicBlock *> &originalFrontierBlocks);

  void replaceUsesOfSpilledPHIWithLoads(
      LoopDependenceInfo *LDI,
      std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
      SpilledLoopCarriedDependency *spill,
      Value *spillEnvPtr,
      DominatorSummary *originalLoopDS,
      std::unordered_set<BasicBlock *> &originalFrontierBlocks);

  std::vector<SequentialSegment *> identifySequentialSegments(
      LoopDependenceInfo *originalLDI,
      LoopDependenceInfo *LDI,
      DataFlowResult *reachabilityDFR);

  void squeezeSequentialSegments(LoopDependenceInfo *LDI,
                                 std::vector<SequentialSegment *> *sss,
                                 DataFlowResult *reachabilityDFR);

  void scheduleSequentialSegments(LoopDependenceInfo *LDI,
                                  std::vector<SequentialSegment *> *sss,
                                  DataFlowResult *reachabilityDFR);

  void addSynchronizations(LoopDependenceInfo *LDI,
                           std::vector<SequentialSegment *> *sss,
                           HELIXTask *helixTask);

  virtual CallInst *injectWaitCall(IRBuilder<> &builder, uint32_t ssID);

  virtual CallInst *injectSignalCall(IRBuilder<> &builder, uint32_t ssID);

  virtual CallInst *injectNikhilWaitCall(IRBuilder<> &builder,
                                         uint32_t ssID,
                                         ConstantInt *cachelineSize,
                                         HELIXTask *task);

  virtual CallInst *injectNikhilSignalCall(IRBuilder<> &builder, uint32_t ssID);

  virtual void computeAndCachePointerOfPastSequentialSegment(
      HELIXTask *helixTask,
      uint32_t ssID);

  virtual void computeAndCachePointerOfFutureSequentialSegment(
      HELIXTask *helixTask,
      uint32_t ssID);

  virtual Value *getPointerOfSequentialSegment(HELIXTask *helixTask,
                                               Value *ssArray,
                                               uint32_t ssID);

  void inlineCalls(Task *task);

  void rewireLoopForIVsToIterateNthIterations(LoopDependenceInfo *LDI);

  BasicBlock *getBasicBlockExecutedOnlyByLastIterationBeforeExitingTask(
      LoopDependenceInfo *LDI,
      uint32_t taskIndex,
      BasicBlock &bb) override;

  /*
   * Fields
   */
  Function *waitSSCall, *signalSSCall;
  Function *NIKHILwaitSSCall, *NIKHILsignalSSCall;
  LoopDependenceInfo *originalLDI;
  LoopEnvironmentBuilder *loopCarriedLoopEnvironmentBuilder;
  std::unordered_set<SpilledLoopCarriedDependency *> spills;
  std::unordered_map<Instruction *, Instruction *>
      lastIterationExecutionDuplicateMap;
  BasicBlock *lastIterationExecutionBlock;
  bool enableInliner;
  Function *taskDispatcherSS;
  Function *taskDispatcherCS;
  void squeezeSequentialSegment(LoopDependenceInfo *LDI,
                                DataFlowResult *reachabilityDFR,
                                SequentialSegment *ss);

  DataFlowResult *computeReachabilityFromInstructions(LoopDependenceInfo *LDI);

private:
  std::string prefixString;
  std::vector<Value *> ssPastPtrs;
  std::vector<Value *> ssFuturePtrs;
};

class SpilledLoopCarriedDependency {
public:
  PHINode *originalLoopCarriedPHI;
  PHINode *loopCarriedPHI;
  Value *clonedInitialValue;
  std::unordered_set<LoadInst *> environmentLoads;
  std::unordered_set<StoreInst *> environmentStores;
};

} // namespace llvm::noelle
