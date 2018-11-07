#pragma once

#include "llvm/IR/Module.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "Parallelization.hpp"
#include "LoopDependenceInfoForParallelizer.hpp"
#include "Heuristics.hpp"
#include "TechniqueWorker.hpp"

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
      virtual bool apply (LoopDependenceInfoForParallelizer *LDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) = 0;

      /*
       * Can the current parallelization technique be applied to parallelize loop LDI?
       */
      virtual bool canBeAppliedToLoop (LoopDependenceInfoForParallelizer *LDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) const = 0 ;

      Value * getEnvArray () { return envBuilder->getEnvArray(); }

      /*
       * Destructor.
       */
      ~ParallelizationTechnique ();

    protected:

      /*
       * Parallelized loop's environment
       */
      void initEnvBuilder (LoopDependenceInfoForParallelizer *LDI);
      virtual void createEnvironment (LoopDependenceInfoForParallelizer *LDI);

      void populateLiveInEnvironment (LoopDependenceInfoForParallelizer *LDI);
      virtual void propagateLiveOutEnvironment (LoopDependenceInfoForParallelizer *LDI);

      /*
       * Worker helpers for manipulating loop body clones
       */
      virtual void createWorkers (int numWorkers);
      virtual void cloneSequentialLoop (
        LoopDependenceInfoForParallelizer *LDI,
        int workerIndex
      );
      virtual void cloneSequentialLoopSubset (
        LoopDependenceInfoForParallelizer *LDI,
        int workerIndex,
        std::set<SCC *> subset
      );

      /*
       * Worker helpers for environment usage
       */
      virtual void generateCodeToLoadLiveInVariables (
        LoopDependenceInfoForParallelizer *LDI, 
        int workerIndex
      );

      virtual void generateCodeToStoreLiveOutVariables (
        LoopDependenceInfoForParallelizer *LDI, 
        int workerIndex
      );

      virtual void adjustDataFlowToUseClones (
        LoopDependenceInfoForParallelizer *LDI,
        int workerIndex
      );

      /*
       * Fields
       */
      Module& module;
      Verbosity verbose;
      EnvBuilder *envBuilder;

      /*
       * Cloned loop representation for each user
       */
      std::vector<TechniqueWorker *> workers;
  };

}
