#pragma once

#include "llvm/IR/Module.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "Parallelization.hpp"
#include "LoopDependenceInfoForParallelizer.hpp"
#include "Heuristics.hpp"

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

      void initEnvBuilder (LoopDependenceInfoForParallelizer *LDI);
      virtual void createEnvironment (LoopDependenceInfoForParallelizer *LDI);
      void populateLiveInEnvironment (LoopDependenceInfoForParallelizer *LDI);
      virtual void propagateLiveOutEnvironment (LoopDependenceInfoForParallelizer *LDI);

      /*
       * Fields
       */
      Module& module;
      Verbosity verbose;
      EnvBuilder *envBuilder;
  };

}
