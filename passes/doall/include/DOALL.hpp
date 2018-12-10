#pragma once

#include "SystemHeaders.hpp"
#include "LoopDependenceInfo.hpp"
#include "PDG.hpp"
#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "PDGAnalysis.hpp"
#include "Parallelization.hpp"
#include "HeuristicsPass.hpp"
#include "ParallelizationTechnique.hpp"

namespace llvm {

  class DOALL : public ParallelizationTechnique {
    public:

      /*
       * Methods
       */
      DOALL (Module &module, Verbosity v);

      bool apply (
        LoopDependenceInfo *LDI,
        Parallelization &par,
        Heuristics *h,
        ScalarEvolution &SE
      ) override ;

      bool canBeAppliedToLoop (
        LoopDependenceInfo *LDI,
        Parallelization &par,
        Heuristics *h,
        ScalarEvolution &SE
      ) const override ;


    protected:

      /*
       * Environment overrides
       */
      void propagateLiveOutEnvironment (
        LoopDependenceInfo *LDI
      ) override ;

      /*
       * DOALL specific generation
       */
      void simplifyOriginalLoopIV (
        LoopDependenceInfo *LDI
      );
      void generateOuterLoopAndAdjustInnerLoop (
        LoopDependenceInfo *LDI
      );
      void propagatePHINodesThroughOuterLoop (
        LoopDependenceInfo *LDI
      );
      void addChunkFunctionExecutionAsideOriginalLoop (
        LoopDependenceInfo *LDI,
        Parallelization &par
      );

  };

}
