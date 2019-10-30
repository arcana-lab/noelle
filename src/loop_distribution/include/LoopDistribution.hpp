#pragma once

#include "SystemHeaders.hpp"
#include "LoopDependenceInfo.hpp"

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

      bool splitLoop (LoopDependenceInfo &LDI, std::set<Instruction *> instructionsToPullOut);

    private:

      /*
       * Fields
       */

      /*
       * Methods
       */

  };

}
