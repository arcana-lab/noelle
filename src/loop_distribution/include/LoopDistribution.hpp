#pragma once

#include "SystemHeaders.hpp"
#include "LoopDependenceInfo.hpp"
#include "SCC.hpp"

namespace llvm {

  struct LoopDistribution : public ModulePass {
    public:

      /*
       * Class fields
       */
      static char ID;

      /*
       * Methods
       */
      LoopDistribution();
      bool doInitialization (Module &M) override ;
      bool runOnModule (Module &M) override ;
      void getAnalysisUsage (AnalysisUsage &AU) const override ;

      bool splitLoop (
        LoopDependenceInfo const &LDI, 
        SCC *SCCToPullOut
        );

      bool splitLoop (
        LoopDependenceInfo const &LDI, 
        std::set<SCC *> const &SCCsToPullOut
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
