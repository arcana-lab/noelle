/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

// #include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"
#include "ScalarEvolutionExpander.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "SystemHeaders.hpp"
#include "SCCDAG.hpp"
#include "SCC.hpp"
#include "LoopsSummary.hpp"
#include "LoopEnvironment.hpp"
#include "DominatorSummary.hpp"
#include "ScalarEvolutionReferencer.hpp"

using namespace std;
using namespace llvm;

namespace llvm {

  class InductionVariable;

  class InductionVariables {
    public:
      InductionVariables (
        LoopsSummary &LIS,
        ScalarEvolution &SE,
        SCCDAG &sccdag,
        LoopEnvironment &loopEnv
      );

      ~InductionVariables ();

      std::set<InductionVariable *> & getInductionVariables (LoopSummary &LS) ;

      InductionVariable * getLoopGoverningInductionVariable (LoopSummary &LS) ;

    private:
      std::unordered_map<LoopSummary *, std::set<InductionVariable *>> loopToIVsMap;
      std::unordered_map<LoopSummary *, InductionVariable *> loopToGoverningIVMap;
  };

  class InductionVariable {
    public:

      InductionVariable  (
        LoopSummary *LS,
        ScalarEvolution &SE,
        PHINode *headerPHI,
        SCC &scc,
        LoopEnvironment &loopEnv,
        ScalarEvolutionReferentialExpander &referentialExpander
      ) ;

      SCC *getSCC (void) const ;

      PHINode * getHeaderPHI (void) const ;

      std::set<PHINode *> & getPHIs (void) ;

      std::set<Instruction *> & getAccumulators (void) ;

      std::set<Instruction *> & getAllInstructions(void) ;

      Value * getStartAtHeader (void) const;

      Value * getSimpleValueOfStepSize (void) const;

      const SCEV *getComposableStepSize (void) const;

      std::vector<Instruction *> getExpansionOfCompositeStepSize(void) const;

      bool isStepSizeLoopInvariant(void) const;

      ~InductionVariable ();

    private:
      SCC &scc;
      PHINode *headerPHI; // outermostPHI
      std::set<PHINode *> PHIs;
      std::set<Instruction *> accumulators;
      std::set<Instruction *> allInstructions;

      Value *startValue;
      Value *stepSize;
      const SCEV *compositeStepSize;
      std::vector<Instruction *> expansionOfCompositeStepSize;
      bool isStepLoopInvariant;
  };

}
