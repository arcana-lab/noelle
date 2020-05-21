/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "SystemHeaders.hpp"
#include "SCCDAG.hpp"
#include "SCC.hpp"
#include "LoopsSummary.hpp"
#include "LoopEnvironment.hpp"
#include "DominatorSummary.hpp"

using namespace std;
using namespace llvm;

namespace llvm {

  class InductionVariable;

  class InductionVariables {
    public:
      // HACK: Once caching Loop.getInductionVariable in LoopSummary, use LoopsSummary only, not LoopInfo as well
      InductionVariables (LoopsSummary &LIS, LoopInfo &LI, ScalarEvolution &SE, SCCDAG &sccdag);
      ~InductionVariables ();

      std::set<InductionVariable *> &getInductionVariables (LoopSummary &LS) ;
      InductionVariable *getLoopGoverningInductionVariable (LoopSummary &LS) ;

    private:
      std::unordered_map<LoopSummary *, std::set<InductionVariable *>> loopToIVsMap;
      std::unordered_map<LoopSummary *, InductionVariable *> loopToGoverningIVMap;
  };

  class InductionVariable {
    public:
      InductionVariable  (LoopSummary *LS, ScalarEvolution &SE, PHINode *headerPHI, SCC &scc) ;

      BasicBlock *getExitBlock () { return exitBlock; }
      PHINode *getHeaderPHI () { return headerPHI; }
      std::set<PHINode *> &getPHIs() { return PHIs; }
      std::set<Instruction *> &getAccumulators() { return accumulators; }
      std::set<Instruction *> &getAllInstructions() { return allInstructions; }
      Value *getStartAtHeader () { return startValue; }
      Value *getStepSize () { return stepSize; }

    private:
      SCC &scc;
      BasicBlock *exitBlock;
      PHINode *headerPHI;
      std::set<PHINode *> PHIs;
      std::set<Instruction *> accumulators;
      std::set<Instruction *> allInstructions;

      Value *startValue;
      Value *stepSize;
  };

  class LoopGoverningIVAttribution {
    public:
      LoopGoverningIVAttribution (InductionVariable &IV, SCC &scc) ;

      InductionVariable &getInductionVariable() { return IV; }
      CmpInst *getHeaderCmpInst() { return headerCmp; }
      Value *getHeaderCmpInstConditionValue() { return conditionValue; }
      BranchInst *getHeaderBrInst() { return headerBr; }
      bool isSCCContainingIVWellFormed() { return isWellFormed; }
      std::set<Instruction *> &getConditionValueDerivation() { 
        return conditionValueDerivation;
      }

    private:
      InductionVariable &IV;
      SCC &scc;
      std::set<Instruction *> conditionValueDerivation;
      Value *conditionValue;
      CmpInst *headerCmp;
      BranchInst *headerBr;
      bool isWellFormed;
  };

  class LoopGoverningIVUtility {
    public:
      LoopGoverningIVUtility (InductionVariable &IV, LoopGoverningIVAttribution &attribution) ;

      std::vector<Instruction *> &getConditionValueDerivation () {
        return conditionValueOrderedDerivation;
      }

      void updateConditionToCatchIteratingPastExitValue (CmpInst *cmpToUpdate) ;

      void cloneConditionalCheckFor (
        Value *recurrenceOfIV,
        Value *clonedComparedValue,
        BasicBlock *continueBlock,
        BasicBlock *exitBlock,
        IRBuilder<> &cloneBuilder) ;

    private:
      LoopGoverningIVAttribution &attribution;
      CmpInst *condition;
      std::vector<Instruction *> conditionValueOrderedDerivation;

      CmpInst::Predicate nonStrictPredicate;
      bool doesOriginalCmpInstHaveIVAsLeftOperand;
      bool flipOperandsToUseNonStrictPredicate;
  };
}