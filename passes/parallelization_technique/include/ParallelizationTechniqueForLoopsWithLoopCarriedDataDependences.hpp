#pragma once

#include "ParallelizationTechnique.hpp"
#include "SCCDAGPartition.hpp"

namespace llvm {

  class ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences : public ParallelizationTechnique {
    public:

      /*
       * Constructor.
       */
      ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences (Module &module, Verbosity v);

      void reset () override ;

      /*
       * Destructor.
       */
      ~ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences ();

    protected:

      /*
       * Partition SCCDAG.
       */
      void partitionSCCDAG (
        LoopDependenceInfo *LDI
      );

      /*
       * Fields
       */
      SCCDAGPartition *partition;
    };

}
