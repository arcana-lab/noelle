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

  class HELIX : public ParallelizationTechnique {
    public:

      /*
       * Methods
       */
      HELIX (Module &module, Verbosity v);

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

    private:

  };

}
