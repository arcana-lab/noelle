#pragma once

#include "SystemHeaders.hpp"
#include "LoopDependenceInfo.hpp"
#include "SCC.hpp"

namespace llvm {

  struct LoopUnroll : public ModulePass {
    public:

      /*
       * Class fields
       */
      static char ID;

      /*
       * Methods
       */
      LoopUnroll();
      bool doInitialization (Module &M) override ;
      bool runOnModule (Module &M) override ;
      void getAnalysisUsage (AnalysisUsage &AU) const override ;

      bool fullyUnrollLoop (
        LoopDependenceInfo const &LDI
        );

    private:

      /*
       * Fields
       */

      /*
       * Methods
       */

  };

}
