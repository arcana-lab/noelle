#pragma once

#include "llvm/IR/Module.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "Parallelization.hpp"
#include "LoopDependenceInfo.hpp"
#include "Heuristics.hpp"
#include "TaskExecution.hpp"
#include "SCCDAGPartition.hpp"

namespace llvm {

  class ParallelizationTechnique {
    public:

      /*
       * Constructor.
       */
      ParallelizationTechnique (Module &module, Verbosity v);

      /*
       * Apply the parallelization technique to the loop LDI.
       */
      virtual bool apply (
        LoopDependenceInfo *LDI,
        Parallelization &par,
        Heuristics *h,
        ScalarEvolution &SE
      ) = 0;

      /*
       * Can the current parallelization technique be applied to parallelize loop LDI?
       */
      virtual bool canBeAppliedToLoop (
        LoopDependenceInfo *LDI,
        Parallelization &par,
        Heuristics *h,
        ScalarEvolution &SE
      ) const = 0 ;

      Value * getEnvArray () { return envBuilder->getEnvArray(); }

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

      virtual void propagateLiveOutEnvironment (LoopDependenceInfo *LDI);

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

      void generateCodeToStoreExitBlockIndex (
        LoopDependenceInfo *LDI,
        int taskIndex
      );

      virtual void adjustDataFlowToUseClones (
        LoopDependenceInfo *LDI,
        int taskIndex
      );

      /*
       * Partition SCCDAG.
       */
      void partitionSCCDAG (
        LoopDependenceInfo *LDI
      );

      /*
       * Fields
       */
      Module& module;
      Verbosity verbose;
      EnvBuilder *envBuilder;
      SCCDAGPartition *partition;

      /*
       * Parallel task related information.
       */
      Function *taskDispatcher;
      FunctionType *taskType;
      std::vector<Task *> tasks;
      int numTaskInstances;
  };

}
