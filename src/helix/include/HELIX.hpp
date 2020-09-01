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
#include "LoopDependenceInfo.hpp"
#include "SubCFGs.hpp"
#include "PDG.hpp"
#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "PDGAnalysis.hpp"
#include "Noelle.hpp"
#include "HeuristicsPass.hpp"
#include "ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences.hpp"
#include "SequentialSegment.hpp"
#include "ControlFlowEquivalence.hpp"

namespace llvm {

  class SpilledLoopCarriedDependency;

  class HELIX : public ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences {
    public:

      /*
       * Methods
       */
      HELIX (
        Module &module,
        Hot &p,
        bool forceParallelization,
        Verbosity v
      );

      bool apply (
        LoopDependenceInfo *LDI, 
        Noelle &par, 
        Heuristics *h
        ) override ;

      bool canBeAppliedToLoop (
        LoopDependenceInfo *LDI, 
        Noelle &par, 
        Heuristics *h
        ) const override ;

      PDG * constructTaskInternalDependenceGraphFromOriginalLoopDG (
        LoopDependenceInfo *LDI,
        PostDominatorTree &postDomTreeOfTaskFunction
      );

      Function * getTaskFunction (void) const ;

      void reset () override ;

    protected:
      void createParallelizableTask (
        LoopDependenceInfo *LDI,
        Noelle &par, 
        Heuristics *h
      );

      bool synchronizeTask (
        LoopDependenceInfo *LDI,
        Noelle &par, 
        Heuristics *h
      );

      void addChunkFunctionExecutionAsideOriginalLoop (
        LoopDependenceInfo *LDI,
        Noelle &par,
        uint64_t numberOfSequentialSegments
      );

      void spillLoopCarriedDataDependencies (
        LoopDependenceInfo *LDI,
        DataFlowResult *reachabilityDFR
      );

      void createLoadsAndStoresToSpilledLCD (
        LoopDependenceInfo *LDI,
        DataFlowResult *reachabilityDFR,
        std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
        SpilledLoopCarriedDependency *spill,
        Value *spillEnvPtr
      );

      void insertStoresToSpilledLCD (
        LoopDependenceInfo *LDI,
        std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
        SpilledLoopCarriedDependency *spill,
        Value *spillEnvPtr
      );

      void defineFrontierForLoadsToSpilledLCD (
        LoopDependenceInfo *LDI,
        DataFlowResult *reachabilityDFR,
        std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
        SpilledLoopCarriedDependency *spill,
        DominatorSummary *originalLoopDS,
        std::unordered_set<BasicBlock *> &originalFrontierBlocks
      );

      void replaceUsesOfSpilledPHIWithLoads (
        LoopDependenceInfo *LDI,
        std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
        SpilledLoopCarriedDependency *spill,
        Value *spillEnvPtr,
        DominatorSummary *originalLoopDS,
        std::unordered_set<BasicBlock *> &originalFrontierBlocks
      );

      std::vector<SequentialSegment *> identifySequentialSegments (
        LoopDependenceInfo *originalLDI,
        LoopDependenceInfo *LDI,
        DataFlowResult *reachabilityDFR
      );
 
      void squeezeSequentialSegments (
        LoopDependenceInfo *LDI,
        std::vector<SequentialSegment *> *sss,
        DataFlowResult *reachabilityDFR
      );

      void scheduleSequentialSegments (
        LoopDependenceInfo *LDI,
        std::vector<SequentialSegment *> *sss,
        DataFlowResult *reachabilityDFR
      );

      void addSynchronizations (
        LoopDependenceInfo *LDI,
        std::vector<SequentialSegment *> *sss
      );

      void inlineCalls (
        void
      );

      void rewireLoopForIVsToIterateNthIterations (
        LoopDependenceInfo *LDI
      );

    private:
      Function *waitSSCall, *signalSSCall;
      LoopDependenceInfo *originalLDI;
      PDG *taskFunctionDG;

      EnvBuilder *loopCarriedEnvBuilder;
      std::unordered_set<SpilledLoopCarriedDependency *> spills;
      std::unordered_map<Instruction *, Instruction *> lastIterationExecutionDuplicateMap;
      BasicBlock *lastIterationExecutionBlock;

      void squeezeSequentialSegment (
        LoopDependenceInfo *LDI,
        DataFlowResult *reachabilityDFR,
        SequentialSegment *ss
      );

      DataFlowResult *computeReachabilityFromInstructions (LoopDependenceInfo *LDI) ;

  };

  class SpilledLoopCarriedDependency {
    public:
      PHINode *originalLoopCarriedPHI;
      PHINode *loopCarriedPHI;
      Value *clonedInitialValue;
      std::unordered_set<LoadInst *> environmentLoads;
      std::unordered_set<StoreInst *> environmentStores;
  };

}
