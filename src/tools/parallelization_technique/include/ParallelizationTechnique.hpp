/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "llvm/IR/Module.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "Noelle.hpp"
#include "LoopDependenceInfo.hpp"
#include "Heuristics.hpp"
#include "Task.hpp"
#include "Hot.hpp"
#include "PDGPrinter.hpp"
#include "SubCFGs.hpp"

namespace llvm::noelle {

  class ParallelizationTechnique {
    public:

      /*
       * Constructor.
       */
      ParallelizationTechnique (
        Module &module, 
        Hot &p,
        Verbosity v
      );

      /*
       * Apply the parallelization technique to the loop LDI.
       */
      virtual bool apply (
        LoopDependenceInfo *LDI,
        Noelle &par,
        Heuristics *h
      ) = 0;

      /*
       * Can the current parallelization technique be applied to parallelize loop LDI?
       */
      virtual bool canBeAppliedToLoop (
        LoopDependenceInfo *LDI,
        Noelle &par,
        Heuristics *h
      ) const = 0 ;

      Value * getEnvArray () { return envBuilder->getEnvArray(); }
      BasicBlock *getParLoopEntryPoint () { return entryPointOfParallelizedLoop; }
      BasicBlock *getParLoopExitPoint () { return exitPointOfParallelizedLoop; }

      virtual void reset () ;

      /*
       * Destructor.
       */
      ~ParallelizationTechnique ();

    protected:

      /*
       * Generate empty tasks.
       *
       * Each task will be composed by the following empty basic blocks:
       * - an entry basic block, which is mapped to the pre-header of the loop LDI
       * - an exit block, which is the only basic block that will exit the task
       * - one basic block per loop exit, which will jump to the exit block
       */
      virtual void generateEmptyTasks (
        LoopDependenceInfo *LDI, 
        std::vector<Task *> taskStructs
      );

      /*
       * Loop's environment
       */
      void initializeEnvironmentBuilder (
        LoopDependenceInfo *LDI,
        std::set<int> nonReducableVars
      );

      void initializeEnvironmentBuilder (
        LoopDependenceInfo *LDI,
        std::set<int> simpleVars,
        std::set<int> reducableVars
      );

      void allocateEnvironmentArray (LoopDependenceInfo *LDI);

      void populateLiveInEnvironment (LoopDependenceInfo *LDI);

      virtual BasicBlock * propagateLiveOutEnvironment (LoopDependenceInfo *LDI, Value *numberOfThreadsExecuted);

      /*
       * Task helpers for manipulating loop body clones
       */
      virtual void cloneSequentialLoop (
        LoopDependenceInfo *LDI,
        int taskIndex
      );
      virtual void cloneSequentialLoopSubset (
        LoopDependenceInfo *LDI,
        int taskIndex,
        std::set<Instruction *> subset
      );

      void cloneMemoryLocationsLocallyAndRewireLoop (
        LoopDependenceInfo *LDI,
        int taskIndex
      );

      std::unordered_map<InductionVariable *, Value *> cloneIVStepValueComputation (
        LoopDependenceInfo *LDI,
        int taskIndex,
        IRBuilder<> &insertBlock
      );

      void adjustStepValueOfPointerTypeIVToReflectPointerArithmetic (
        std::unordered_map<InductionVariable *, Value *> clonedStepValueMap,
        IRBuilder<> &insertBlock
      );

      /*
       * Task helpers for environment usage
       */
      virtual void generateCodeToLoadLiveInVariables (
        LoopDependenceInfo *LDI, 
        int taskIndex
      );

      virtual void generateCodeToStoreLiveOutVariables (
        LoopDependenceInfo *LDI, 
        int taskIndex
      );

      Instruction * fetchOrCreatePHIForIntermediateProducerValueOfReducibleLiveOutVariable (
        LoopDependenceInfo *LDI, 
        int taskIndex,
        int envIndex,
        BasicBlock *insertBasicBlock,
        DominatorSummary &taskDS
      );

      PHINode * fetchLoopEntryPHIOfProducer (
        LoopDependenceInfo *LDI, 
        Value *producer
      );

      void generateCodeToStoreExitBlockIndex (
        LoopDependenceInfo *LDI,
        int taskIndex
      );

      std::set<BasicBlock *> determineLatestPointsToInsertLiveOutStore (
        LoopDependenceInfo *LDI,
        int taskIndex,
        Instruction *liveOut,
        bool isReduced,
        DominatorSummary &taskDS
      );

      void adjustDataFlowToUseClones (
        LoopDependenceInfo *LDI,
        int taskIndex
      );

      void adjustDataFlowToUseClones (
        Instruction *cloneI,
        int taskIndex
      );

      void setReducableVariablesToBeginAtIdentityValue (
        LoopDependenceInfo *LDI,
        int taskIndex
      );

      Value *getIdentityValueForEnvironmentValue (
        LoopDependenceInfo *LDI,
        int environmentIndex,
        Type *typeForValue
      );

      Value *castToCorrectReducibleType (IRBuilder<> &builder, Value *value, Type *targetType) ;

      /*
       * Partition SCCDAG.
       */
      void partitionSCCDAG (
        LoopDependenceInfo *LDI
      );

      /*
       * General purpose helpers (that should be moved to parallelization_utils)
       */
      void doNestedInlineOfCalls (Function *F, std::set<CallInst *> &calls);

      float computeSequentialFractionOfExecution (
        LoopDependenceInfo *LDI,
        Noelle &par
      ) const ;

      /*
       * Debug
       */
      void dumpToFile (LoopDependenceInfo &LDI);

      /*
       * Fields
       */
      Module& module;
      Verbosity verbose;
      EnvBuilder *envBuilder;

      /*
       * Parallel task related information.
       */
      Function *taskDispatcher;
      FunctionType *taskType;
      BasicBlock *entryPointOfParallelizedLoop, *exitPointOfParallelizedLoop;
      std::vector<Task *> tasks;
      int numTaskInstances;

      /*
       * Profiles.
       */
      Hot &profile;
  };

}
