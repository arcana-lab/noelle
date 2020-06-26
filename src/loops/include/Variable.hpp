/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"
#include "LoopStructure.hpp"
#include "LoopCarriedDependencies.hpp"
#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "SCC.hpp"

namespace llvm {

  class VariableUpdate;

  /*
   * A Variable is an evolving value over some loop context
   * 
   * Since LLVM IR is in SSA and because we are trying to generalize the evolution of a value
   * through some context even if it isn't a memory location, we operate at some loop level,
   * describing the evolution of a data OR memory value
   */
  class Variable {
    public:
      Variable (
        const LoopStructure &loop,
        const LoopCarriedDependencies &LCD,
        PDG &loopDG,
        SCC &variableSCC,
        PHINode *declarationPHI
      ) ;

      bool isEvolutionReducibleAcrossLoopIterations (void) const ;

    private:

      /*
       * TODO: Implement and make public
       */
      Variable (
        const LoopStructure &loop,
        const LoopCarriedDependencies &LCD,
        PDG &loopDG,
        SCC &variableSCC,
        Value *memoryLocation
      ) ;

      /*
       * We track the outer-most loop and make claims about the Variable's evolution across iterations
       * by virtue of loop carried updates (values that produce loop carried dependencies)
       */
      const LoopStructure &outermostLoopOfVariable;

      /*
       * We compute the strongly connected component containing only updates to the variable
       * This removes loop carried dependencies to other variables contained within the provided SCC
       */
      SCC *sccOfVariableOnly;
      SCC *sccOfDataAndMemoryVariableValuesOnly;

      /*
       * This is the declaration of the variable
       * For data variables, this is the loop entry PHI
       * For memory variables, this is an allocation of memory always updated contiguously while in the loop
       */
      Value *declarationValue;
      bool isDataVariable;

      /*
       * The initial value of the variable at the time of its declaration
       * For data variables, this is the loop entry's preheader value
       * For memory variables, this is optionally the value used to instantiate the allocated memory
       */
      Value *initialValue;

      /*
       * This is the complete set of possible updates made to the variable within any given loop iteration
       */ 
      std::unordered_set<VariableUpdate *> variableUpdates;

      /*
       * This is the set of possible updates that can be loop carried
       */
      std::unordered_set<VariableUpdate *> loopCarriedVariableUpdates;

      /*
       * Any control values (terminator instructions, select instruction predicates, etc...)
       * that determine which updates are applied to the variable
       */
      std::set<Value *> controlValuesGoverningEvolution;

  };

  /*
   * Information about an instruction that contributes to the evolution of the Variable
   */
  class VariableUpdate {
    public:

      VariableUpdate (Instruction *updateInstruction, SCC *dataMemoryVariableSCC) ;

      bool mayUpdateBeOverride (void) const ;

      bool isCommutativeWithSelf (void) const ;

      bool isAssociativeWithSelf (void) const ;

      bool isTransformablyCommutativeWith (const VariableUpdate &otherUpdate) const ;

      bool isAssociativeWith (const VariableUpdate &otherUpdate) const ;

      bool isTransformablyCommutativeWithSelf (void) const ;

    private:

      bool isBothUpdatesAddOrSub (const VariableUpdate &otherUpdate) const ;
      bool isBothUpdatesMul (const VariableUpdate &otherUpdate) const ;
      bool isBothUpdatesSameBitwiseLogicalOp (const VariableUpdate &otherUpdate) const ;

      bool isAdd (void) const ;
      bool isMul (void) const ;
      bool isSub (void) const ;
      bool isSubTransformableToAdd (void) const ;

      /*
       * The instruction that constitutes the update
       */
      Instruction *updateInstruction;

      /*
       * For data variables, this is the instruction itself.
       * For memory variables, this is what is stored into the memory location
       */
      Value *newVariableValue;

      /*
       * This is all previous values of the variable used in determining the new value
       */
      std::unordered_set<Use *> oldVariableValuesUsed;

      /*
       * This is all external values used in determining the new value
       */
      std::unordered_set<Use *> externalValuesUsed;

  };

}
