/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"
#include "AccumulatorOpInfo.hpp"
#include "SCC.hpp"

namespace llvm {

  class SCCAttrs {
    public:

      /*
       * Types.
       */
      enum SCCType {SEQUENTIAL, REDUCIBLE, INDEPENDENT};

      /*
       * Iterators.
       */
      typedef typename set<PHINode *>::iterator phi_iterator;
      typedef typename set<Instruction *>::iterator instruction_iterator;

      /*
       * Fields
       */
      std::set<Value *> stronglyConnectedDataValues;
      std::set<Value *> weaklyConnectedDataValues;

      std::set<std::pair<Value *, Instruction *>> controlPairs;

      /*
       * Constructor
       */
      SCCAttrs (
        SCC *s, 
        AccumulatorOpInfo &opInfo
        );

      /*
       * Get the SCC.
       */
      SCC * getSCC (void);

      /*
       * Return the type of SCC.
       */
      SCCType getType (void) const;

      /*
       * Return true if the iterations of the SCC must execute sequentially.
       * Return false otherwise.
       */
      bool mustExecuteSequentially (void) const ;

      /*
       * Return true if a reduction transformation can be applied to the SCC. 
       * Return false otherwise.
       */
      bool canExecuteReducibly (void) const ;

      /*
       * Return true if the iterations of the SCC are independent between each other.
       * Return false otherwise.
       */
      bool canExecuteIndependently (void) const ;

      /*
       * Return true if it is safe to clone the SCC.
       * Return false otherwise.
       */
      bool canBeCloned (void) const ;

      /*
       * Return true if the SCC exists because of updates of an induction variable.
       * Return false otherwise.
       */
      bool isInductionVariableSCC (void) const ;

      /*
       * Get the PHIs.
       */
      iterator_range<phi_iterator> getPHIs (void);

      /*
       * Check if the SCC contains a PHI instruction.
       */
      bool doesItContainThisPHI (PHINode *phi);

      /*
       * Return the single PHI if it exists. nullptr otherwise.
       */
      PHINode * getSinglePHI (void);

      /*
       * Return the number of PHIs included in the SCC.
       */
      uint32_t numberOfPHIs (void);

     /*
       * Get the accumulators.
       */
      iterator_range<instruction_iterator> getAccumulators (void);

      /*
       * Return the single accumulator if it exists. nullptr otherwise.
       */
      Instruction *getSingleAccumulator (void);

      /*
       * Check if the SCC contains an accumulator.
       */
      bool doesItContainThisInstructionAsAccumulator (Instruction *inst);

      /*
       * Return the number of accumulators included in the SCC.
       */
      uint32_t numberOfAccumulators (void);

      /*
       * Set the type of SCC.
       */
      void setType (SCCType t);

      /*
       * Set the SCC as created by updated of an induction variable.
       */
      void setSCCToBeInductionVariable (bool hasIV = true);

      /*
       * Set the SCC to be clonable.
       */
      void setSCCToBeClonable (bool isClonable = true);

      void collectSCCValues ();

      const std::pair<Value *, Instruction *> * getSingleInstructionThatControlLoopExit (void);

    private:
      SCC *scc;
      SCCType sccType;
      std::set<BasicBlock *> bbs;
      AccumulatorOpInfo accumOpInfo;
      std::set<Instruction *> controlFlowInsts;
      std::set<PHINode *> PHINodes;
      std::set<Instruction *> accumulators;
      bool isClonable;
      bool hasIV;
  
      void collectPHIsAndAccumulators (void);
      void collectControlFlowInstructions (void);
  };

}
