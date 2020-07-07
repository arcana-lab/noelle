/*
 * Copyright 2019 - 2020  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"
#include "Noelle.hpp"
#include "LoopDependenceInfo.hpp"

#include "PDGPrinter.hpp"
#include "SubCFGs.hpp"

namespace llvm {

  class LastLiveOutPeeler {
    public:
      LastLiveOutPeeler (LoopDependenceInfo const &LDI, Noelle &noelle) ;

      bool peelLastLiveOutComputation () ;

    private:
      LoopDependenceInfo const &LDI;
      Noelle &noelle;

      /*
       * We used normalized SCCs when analyzing the loop structure to determine if peeling is possible
       * This is simply because SCCDAGAttrs uses the normalized SCCDAG. We wouldn't need to otherwise
       * 
       * We use strict SCCs from the not normalized SCCDAG for last live out analysis as
       * SCCDAGAttrs does not have such an analysis
       */
      std::unordered_set<SCC *> normalizedSCCsOfGoverningIVs;
      std::unordered_set<SCC *> normalizedSCCsOfConditionsAndBranchesDependentOnIVSCCs;
      std::unordered_set<SCC *> sccsOfLastLiveOuts;

      bool fetchNormalizedSCCsGoverningControlFlowOfLoop (void) ;

      /*
       * Determining last live out instructions
       */
      void fetchSCCsOfLastLiveOuts (void) ;
      std::unordered_set<SCC *> fetchChainOfSCCsForLastLiveOutLeafSCC (DGNode<SCC> *sccNode) ;

      std::unordered_set<Value *> loopCarriedConsumers;

  };

}