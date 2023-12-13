/*
 * Copyright 2016 - 2023  Angelo Matni, Simone Campanoni
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
#ifndef NOELLE_SRC_TOOLS_HELIX_H_
#define NOELLE_SRC_TOOLS_HELIX_H_

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/LoopContent.hpp"
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
#include "noelle/tools/SpilledLoopCarriedDependence.hpp"
#include "HeuristicsPass.hpp"

namespace arcana::noelle {

class HELIX
  : public ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences {
public:
  /*
   * Methods
   */
  HELIX(Noelle &n, bool forceParallelization);

  bool apply(LoopContent *LDI, Heuristics *h) override;

  bool canBeAppliedToLoop(LoopContent *LDI, Heuristics *h) const override;

  PDG *constructTaskInternalDependenceGraphFromOriginalLoopDG(LoopContent *LDI);

  Function *getTaskFunction(void) const;

  SCC *getTheSequentialSCCThatCreatesTheSequentialPrologue(
      LoopContent *LDI) const;

  bool doesHaveASequentialPrologue(LoopContent *LDI) const;

  uint32_t getMinimumNumberOfIdleCores(void) const override;

  std::string getName(void) const override;

  Transformation getParallelizationID(void) const override;

  virtual ~HELIX();

protected:
  virtual HELIXTask *createParallelizableTask(LoopContent *LDI, Heuristics *h);

  virtual bool synchronizeTask(LoopContent *LDI,
                               Heuristics *h,
                               HELIXTask *helixTask);

  virtual void invokeParallelizedLoop(LoopContent *LDI,
                                      uint64_t numberOfSequentialSegments);

  void spillLoopCarriedDataDependencies(LoopContent *LDI,
                                        DataFlowResult *reachabilityDFR,
                                        HELIXTask *helixTask);

  void createLoadsAndStoresToSpilledLCD(
      LoopContent *LDI,
      DataFlowResult *reachabilityDFR,
      std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
      SpilledLoopCarriedDependence *spill,
      Value *spillEnvPtr);

  void insertStoresToSpilledLCD(
      LoopContent *LDI,
      std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
      SpilledLoopCarriedDependence *spill,
      Value *spillEnvPtr);

  void defineFrontierForLoadsToSpilledLCD(
      LoopContent *LDI,
      DataFlowResult *reachabilityDFR,
      std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
      SpilledLoopCarriedDependence *spill,
      DominatorSummary *originalLoopDS,
      std::unordered_set<BasicBlock *> &originalFrontierBlocks);

  void replaceUsesOfSpilledPHIWithLoads(
      LoopContent *LDI,
      std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
      SpilledLoopCarriedDependence *spill,
      Value *spillEnvPtr,
      DominatorSummary *originalLoopDS,
      std::unordered_set<BasicBlock *> &originalFrontierBlocks);

  std::vector<SequentialSegment *> identifySequentialSegments(
      LoopContent *originalLDI,
      LoopContent *LDI,
      DataFlowResult *reachabilityDFR,
      HELIXTask *helixTask);

  void squeezeSequentialSegments(LoopContent *LDI,
                                 std::vector<SequentialSegment *> *sss,
                                 DataFlowResult *reachabilityDFR);

  void scheduleSequentialSegments(LoopContent *LDI,
                                  std::vector<SequentialSegment *> *sss,
                                  DataFlowResult *reachabilityDFR);

  void addSynchronizations(LoopContent *LDI,
                           std::vector<SequentialSegment *> *sss,
                           HELIXTask *helixTask);

  virtual CallInst *injectWaitCall(IRBuilder<> &builder, uint32_t ssID);

  virtual CallInst *injectSignalCall(IRBuilder<> &builder, uint32_t ssID);

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

  void rewireLoopForIVsToIterateNthIterations(LoopContent *LDI);

  void rewireLoopForPeriodicVariables(LoopContent *LDI);

  BasicBlock *getBasicBlockExecutedOnlyByLastIterationBeforeExitingTask(
      LoopContent *LDI,
      uint32_t taskIndex,
      BasicBlock &bb) override;

  /*
   * Fields
   */
  Function *waitSSCall, *signalSSCall;
  LoopContent *originalLDI;
  LoopEnvironmentBuilder *loopCarriedLoopEnvironmentBuilder;
  std::unordered_set<SpilledLoopCarriedDependence *> spills;
  std::unordered_map<Instruction *, Instruction *>
      lastIterationExecutionDuplicateMap;
  BasicBlock *lastIterationExecutionBlock;
  bool enableInliner;
  Function *taskDispatcherSS;
  Function *taskDispatcherCS;
  void squeezeSequentialSegment(LoopContent *LDI,
                                DataFlowResult *reachabilityDFR,
                                SequentialSegment *ss);

  DataFlowResult *computeReachabilityFromInstructions(LoopContent *LDI);

private:
  std::string prefixString;
  std::vector<Value *> ssPastPtrs;
  std::vector<Value *> ssFuturePtrs;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_TOOLS_HELIX_H_
