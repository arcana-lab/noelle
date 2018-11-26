#pragma once

#include "llvm/IR/Module.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "Parallelization.hpp"
#include "LoopDependenceInfoForParallelizer.hpp"
#include "Heuristics.hpp"
#include "TaskExecution.hpp"

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
        LoopDependenceInfoForParallelizer *LDI,
        Parallelization &par,
        Heuristics *h,
        ScalarEvolution &SE
      ) = 0;

      /*
       * Can the current parallelization technique be applied to parallelize loop LDI?
       */
      virtual bool canBeAppliedToLoop (
        LoopDependenceInfoForParallelizer *LDI,
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
       * Task helpers for preparing loop code generation
       */
      virtual void generateTasks (
        LoopDependenceInfoForParallelizer *LDI, 
        std::vector<TaskExecution *> taskStructs
      );

      /*
       * Parallelized loop's environment
       */
      void initializeEnvironmentBuilder (
        LoopDependenceInfoForParallelizer *LDI,
        std::set<int> simpleVars,
        std::set<int> reducableVars
      );
      void allocateEnvironmentArray (LoopDependenceInfoForParallelizer *LDI);
      void populateLiveInEnvironment (LoopDependenceInfoForParallelizer *LDI);
      virtual void propagateLiveOutEnvironment (LoopDependenceInfoForParallelizer *LDI);

      /*
       * Task helpers for manipulating loop body clones
       */
      virtual void cloneSequentialLoop (
        LoopDependenceInfoForParallelizer *LDI,
        int taskIndex
      );
      virtual void cloneSequentialLoopSubset (
        LoopDependenceInfoForParallelizer *LDI,
        int taskIndex,
        std::set<Instruction *> subset
      );

      /*
       * Task helpers for environment usage
       */
      virtual void generateCodeToLoadLiveInVariables (
        LoopDependenceInfoForParallelizer *LDI, 
        int taskIndex
      );

      virtual void generateCodeToStoreLiveOutVariables (
        LoopDependenceInfoForParallelizer *LDI, 
        int taskIndex
      );

      void generateCodeToStoreExitBlockIndex (
        LoopDependenceInfoForParallelizer *LDI,
        int taskIndex
      );

      virtual void adjustDataFlowToUseClones (
        LoopDependenceInfoForParallelizer *LDI,
        int taskIndex
      );

      /*
       * Fields
       */
      Module& module;
      Verbosity verbose;
      EnvBuilder *envBuilder;

      /*
       * Cloned loop representation for users
       */
      Function *taskDispatcher;
      FunctionType *taskType;
      std::vector<TaskExecution *> tasks;
      int numTaskInstances;
  };

}
