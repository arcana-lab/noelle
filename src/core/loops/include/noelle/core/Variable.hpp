/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni, Brian Homerding
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/LoopStructure.hpp"
#include "noelle/core/PDG.hpp"
#include "noelle/core/SCCDAG.hpp"
#include "noelle/core/SCC.hpp"
#include "noelle/core/StayConnectedNestedLoopForest.hpp"

namespace llvm::noelle {

  class EvolutionUpdate;

  /*
   * A LoopCarriedCycle is an evolving value over some loop context
   * 
   * Since LLVM IR is in SSA and because we are trying to generalize the evolution of a value
   * through some context even if it isn't a memory location, we operate at some loop level,
   * describing the evolution of a data OR memory value
   */
  class LoopCarriedCycle {
    public:

      virtual bool isEvolutionReducibleAcrossLoopIterations (void) const ;

  };

  class LoopCarriedMemoryLocation : public LoopCarriedCycle {
    public:

      /*
       * TODO: Implement
       */
      LoopCarriedMemoryLocation (
        const LoopStructure &loop,
        PDG &loopDG,
        SCC &memoryLocationSCC,
        Value *memoryLocation
      ) ;

      LoopCarriedMemoryLocation () = delete ;

      /*
       * TODO: Implement
       */
      bool isEvolutionReducibleAcrossLoopIterations (void) const override ;

  };

  class LoopCarriedVariable : public LoopCarriedCycle {
    public:
      LoopCarriedVariable (
        const LoopStructure &loop,
        StayConnectedNestedLoopForestNode *loopNode,
        PDG &loopDG,
        SCCDAG &sccdag,
        SCC &variableSCC,
        PHINode *declarationPHI
      ) ;

      LoopCarriedVariable () = delete ;

      ~LoopCarriedVariable () ;

      bool isEvolutionReducibleAcrossLoopIterations (void) const override ;

      PHINode * getLoopEntryPHIForValueOfVariable (Value *value) const ;

    private:

      PDG * produceDataAndMemoryOnlyDGFromVariableDG (PDG &variableDG) const ;

      std::unordered_set<Value *> getConsumersOfVariable (void) const ;

      bool areValuesPropagatingVariableIntermediatesOutsideLoop (std::unordered_set<Value *> values) const ;

      bool hasRoundingError (std::unordered_set<EvolutionUpdate *> &arithmeticUpdates) const ;

      /*
       * A flag to ensure the variable is fully understood
       */
      bool isValid;

      /*
       * We track the outer-most loop and make claims about the Variable's evolution across iterations
       * by virtue of loop carried updates (values that produce loop carried dependencies)
       */
      const LoopStructure &outermostLoopOfVariable;

      /*
       * We compute the strongly connected component containing only updates to the variable
       * This removes loop carried dependencies to other variables contained within the provided SCC
       */
      PDG *dataDGOfVariable;
      SCCDAG *sccdagOfVariable;
      SCCDAG *dataSCCDAGOfVariable;
      SCC *sccOfVariableOnly;
      SCC *sccOfDataAndMemoryVariableValuesOnly;

      /*
       * This is the declaration of the variable
       * For data variables, this is the loop entry PHI
       */
      Value *declarationValue;

      /*
       * The initial value of the variable at the time of its declaration
       * For data variables, this is the loop entry's preheader value
       */
      Value *initialValue;

      /*
       * This is the complete set of possible updates made to the variable within any given loop iteration
       */ 
      std::unordered_set<EvolutionUpdate *> variableUpdates;

      /*
       * This is the set of possible updates that can be loop carried
       */
      std::unordered_set<EvolutionUpdate *> loopCarriedVariableUpdates;

      /*
       * Any control values (terminator instructions, select instruction predicates, etc...)
       * that determine which updates are applied to the variable
       */
      std::set<Value *> controlValuesGoverningEvolution;

      /*
       * Collect all casts made on the variable that propagate iteration to iteration to ensure that
       * no rounding is done disguised as a cast
       */
      std::unordered_set<CastInst *> castsInternalToVariableComputation;

  };

  /*
   * Information about an instruction that contributes to the evolution of the LoopCarriedCycle
   */
  class EvolutionUpdate {
    public:

      EvolutionUpdate (Instruction *updateInstruction, SCC *dataMemoryVariableSCC) ;

      bool mayUpdateBeOverride (void) const ;

      bool isCommutativeWithSelf (void) const ;

      bool isAssociativeWithSelf (void) const ;

      bool isTransformablyCommutativeWith (const EvolutionUpdate &otherUpdate) const ;

      bool isAssociativeWith (const EvolutionUpdate &otherUpdate) const ;

      bool isTransformablyCommutativeWithSelf (void) const ;

      Instruction * getUpdateInstruction (void) const ;

      bool isAdd (void) const ;
      bool isMul (void) const ;
      bool isSub (void) const ;
      bool isSubTransformableToAdd (void) const ;

    private:

      bool isBothUpdatesAddOrSub (const EvolutionUpdate &otherUpdate) const ;
      bool isBothUpdatesMul (const EvolutionUpdate &otherUpdate) const ;
      bool isBothUpdatesSameBitwiseLogicalOp (const EvolutionUpdate &otherUpdate) const ;

      /*
       * The instruction that constitutes the update
       */
      Instruction *updateInstruction;

      /*
       * For variables, this is the instruction itself.
       * For memory locations, this is what is stored into the memory location
       */
      Value *newValue;

      /*
       * This is all previous values of the variable/memory location used in determining the new value
       */
      std::unordered_set<Use *> internalValuesUsed;

      /*
       * This is all external values used in determining the new value
       */
      std::unordered_set<Use *> externalValuesUsed;

  };

}
