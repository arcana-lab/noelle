/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/IVDescriptors.h"
#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/ScalarEvolutionReferencer.hpp"
#include "noelle/core/StayConnectedNestedLoopForest.hpp"
#include "noelle/core/LoopEnvironment.hpp"
#include "noelle/core/DominatorSummary.hpp"
#include "noelle/core/SCCDAG.hpp"
#include "noelle/core/SCC.hpp"
#include "noelle/core/Invariants.hpp"

namespace llvm::noelle {

  class LoopGoverningIVAttribution;

  class InductionVariable {
    public:

      InductionVariable  (
          LoopStructure *LS,
          InvariantManager &IVM,
          ScalarEvolution &SE,
          PHINode *loopEntryPHI,
          SCC &scc,
          LoopEnvironment &loopEnvironment,
          ScalarEvolutionReferentialExpander &referentialExpander
          ) ;

          // For LLVM IVs
          InductionVariable  (
          LoopStructure *LS,
          InvariantManager &IVM,
          ScalarEvolution &SE,
          PHINode *loopEntryPHI,
          SCC &scc,
          LoopEnvironment &loopEnvironment,
          ScalarEvolutionReferentialExpander &referentialExpander,
          InductionDescriptor &ID
          ) ;

      SCC * getSCC (void) const ;

      PHINode * getLoopEntryPHI (void) const ;

      std::unordered_set<PHINode *> getPHIs (void) const ;

      std::unordered_set<Instruction *> getNonPHIIntermediateValues (void) const ;

      std::unordered_set<Instruction *> getAllInstructions (void) const ;

      std::unordered_set<Instruction *> getDerivedSCEVInstructions (void) const ;

      Value * getStartValue (void) const;

      Value * getSingleComputedStepValue (void) const;

      std::vector<Instruction *> getComputationOfStepValue (void) const;

      bool isStepValueLoopInvariant (void) const;

      bool isStepValuePositive (void) const ;

      const SCEV * getStepSCEV (void) const;

      bool isIVInstruction (Instruction *I) const;

      bool isDerivedFromIVInstructions (Instruction *I) const;

      Type * getIVType (void) const;

      ~InductionVariable ();

    private:

      /*
       * The SCC that contains the induction variable
       */
      SCC &scc;

      /*
       * The loop entry PHI node. For normalized loops with a single header,
       * this PHI is the destination of all loop carried dependencies for the IV
       */
      PHINode *loopEntryPHI;

      /*
       * All PHIs, whether intermediate or the loop entry PHI
       */
      std::unordered_set<PHINode *> PHIs;

      /*
       * All non-PHI intermediate values of the IV
       */
      std::unordered_set<Instruction *> nonPHIIntermediateValues;

      /*
       * All PHI/non-PHI intermediate values AND all casts of the IV
       */
      std::unordered_set<Instruction *> allInstructions;

      /*
       * Derived SCEV instructions relying solely on loop invariants, constants, and this IV
       */
      std::unordered_set<Instruction *> derivedSCEVInstructions;

      /*
       * Start value (the incoming value to the loop entry PHI from the preheader)
       */
      Value *startValue;

      /*
       * The SCEV representing the step recurrence
       */
      const SCEV *stepSCEV;

      /*
       * A single constant or loop external value representing the step recurrence
       */
      Value *singleStepValue;

      /*
       * The values, in order of execution, used to compute the step recurrence
       * The last value is the step value between iterations
       * NOTE: these values expand the step SCEV so that all uses in the values are
       * 1) loop invariant and loop external
       * OR
       * 2) derived from another induction variable in the loop
       * 
       * TODO: Imply in name that this computation is a list of instructions, and
       * that if the value need not be computed and can instead be referenced, this
       * list will be empty
       */
      std::vector<Instruction *> computationOfStepValue;

      /*
       * Whether the computed step value's uses are all loop invariant/external
       */
      bool isComputedStepValueLoopInvariant;

      /*
      * Type of the loopEntryPHI which represents the type of the whole IV
      */
      Type * loopEntryPHIType;

      /*
       * Helper functions and structures
       */ 
      std::set<Value *> valuesToReferenceInComputingStepValue;
      std::set<Value *> valuesInScopeOfInductionVariable;
      void collectValuesInternalAndExternalToLoopAndSCC (
          LoopStructure *LS,
          LoopEnvironment &loopEnvironment
          ) ;

      void deriveStepValue (
          LoopStructure *LS,
          ScalarEvolution &SE,
          ScalarEvolutionReferentialExpander &referentialExpander
          ) ;

      void deriveStepValueFromSCEVConstant (const SCEVConstant *scev) ;
      void deriveStepValueFromSCEVUnknown (const SCEVUnknown *scev, LoopStructure *LS) ;
      bool deriveStepValueFromCompositeSCEV (
          const SCEV *scev,
          ScalarEvolutionReferentialExpander &referentialExpander,
          LoopStructure *LS
          ) ;

      void traverseCycleThroughLoopEntryPHIToGetAllIVInstructions (LoopStructure *LS) ;

      void traverseConsumersOfIVInstructionsToGetAllDerivedSCEVInstructions (
          LoopStructure *LS,
          InvariantManager &IVM,
          ScalarEvolution &SE
          ) ;

  };

  class InductionVariableManager {
    public:

      InductionVariableManager (
          StayConnectedNestedLoopForestNode *loop,
          InvariantManager &IVM,
          ScalarEvolution &SE,
          SCCDAG &sccdag,
          LoopEnvironment &loopEnv,
          Loop &LLVMLoop
          );

      InductionVariableManager () = delete;

      /*
       * Return all induction variables including the loop-governing one of the outermost loop of the loop sub-tree related to @this.
       */
      std::unordered_set<InductionVariable *> getInductionVariables (void) const ;

      /*
       * Return all induction variables including the loop-governing one.
       */
      std::unordered_set<InductionVariable *> getInductionVariables (LoopStructure &LS) const ;

      /*
       * Return all induction variables that @i is involved in for any loop/sub-loop related to this manager.
       */
      std::unordered_set<InductionVariable *> getInductionVariables (Instruction *i) const ;

      InductionVariable * getInductionVariable (LoopStructure &LS, Instruction *i) const ;

      InductionVariable * getLoopGoverningInductionVariable (LoopStructure &LS) const ;

      bool doesContributeToComputeAnInductionVariable (Instruction *i) const ;

      LoopGoverningIVAttribution * getLoopGoverningIVAttribution (LoopStructure &LS) const ;

      InductionVariable * getDerivingInductionVariable (LoopStructure &LS, Instruction *derivedInstruction) const ;

      ~InductionVariableManager ();

    private:
      StayConnectedNestedLoopForestNode *loop;
      std::unordered_map<LoopStructure *, std::unordered_set<InductionVariable *>> loopToIVsMap;
      std::unordered_map<LoopStructure *, LoopGoverningIVAttribution *> loopToGoverningIVAttrMap;
  };

}
