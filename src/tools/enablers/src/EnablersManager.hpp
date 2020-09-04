/*
 * Copyright 2019 - 2020 Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"

#include "LoopDependenceInfo.hpp"
#include "PDG.hpp"
#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "PDGAnalysis.hpp"
#include "Noelle.hpp"
#include "LoopDistribution.hpp"
#include "LoopUnroll.hpp"
#include "LoopWhilify.hpp"
#include "LoopInvariantCodeMotion.hpp"
#include "SCEVSimplification.hpp"

namespace llvm {

  class EnablersManager : public ModulePass {
    public:

      /*
       * Class fields
       */
      static char ID;

      /*
       * Methods
       */
      EnablersManager();
      bool doInitialization (Module &M) override ;
      bool runOnModule (Module &M) override ;
      void getAnalysisUsage (AnalysisUsage &AU) const override ;

    private:

      /*
       * Fields
       */
      bool enableEnablers;

      /*
       * Methods
       */
      std::vector<LoopDependenceInfo *> getLoopsToParallelize (
          Module &M, 
          Noelle &par
        );

      bool applyEnablers (
        LoopDependenceInfo *LDI,
        Noelle &par,
        LoopDistribution &loopDist,
        LoopUnroll &loopUnroll,
        LoopWhilifier &LoopWhilifier,
        LoopInvariantCodeMotion &loopInvariantCodeMotion,
        SCEVSimplification &scevSimplification
        );

      bool applyLoopWhilifier (
          LoopDependenceInfo *LDI,
          Noelle &par,
          LoopWhilifier &loopWhilifier
        );

      bool applyLoopDistribution (
          LoopDependenceInfo *LDI,
          Noelle &par,
          LoopDistribution &loopDist
        );

      bool applyDevirtualizer (
        LoopDependenceInfo *LDI,
        Noelle &par,
        LoopUnroll &loopUnroll
        );
  };

}
