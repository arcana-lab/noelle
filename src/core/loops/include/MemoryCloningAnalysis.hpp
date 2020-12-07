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

#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "LoopsSummary.hpp"
#include "Invariants.hpp"
#include "DominatorSummary.hpp"

namespace llvm::noelle {

  class ClonableMemoryLocation ;

  class MemoryCloningAnalysis {
    public:
      MemoryCloningAnalysis (LoopStructure *loop, DominatorSummary &DS) ;

      const ClonableMemoryLocation * getClonableMemoryLocationFor (Instruction *I) const ;

      std::unordered_set<ClonableMemoryLocation *> getClonableMemoryLocations (void) const ;

    private:
      std::unordered_set<std::unique_ptr<ClonableMemoryLocation>> clonableMemoryLocations;
  };

  class ClonableMemoryLocation {
    public:
      ClonableMemoryLocation (
        AllocaInst *allocation,
        uint64_t sizeInBits,
        LoopStructure *loop,
        DominatorSummary &DS
      ) ;

      AllocaInst *getAllocation (void) const ;

      std::unordered_set<Instruction *> getLoopInstructionsUsingLocation (void) const ;

      bool isInstructionCastOrGEPOfLocation (Instruction *I) const ;
      bool isInstructionStoringLocation (Instruction *I) const ;
      bool isInstructionLoadingLocation (Instruction *I) const ;

      bool isClonableLocation (void) const ;

      static bool isMemCpyInstrinsicCall (CallInst *call) ;

    private:
      AllocaInst *allocation;
      Type *allocatedType;
      uint64_t sizeInBits;
      LoopStructure *loop;
      bool isClonable;

      std::unordered_set<Instruction *> castsAndGEPs;
      std::unordered_set<Instruction *> storingInstructions;
      std::unordered_set<Instruction *> nonStoringInstructions;

      bool identifyStoresAndOtherUsers (LoopStructure *loop, DominatorSummary &DS) ;

      /*
       * A set of storing instructions that completely override the allocation's
       * values before any use it dominates gets to using the allocation
       */
      struct OverrideSet {
        BasicBlock *dominatingBlockOfNonStoringInsts;
        std::unordered_set<Instruction *> initialStoringInstructions;
        std::unordered_set<Instruction *> subsequentNonStoringInstructions;
      };
      std::unordered_set<std::unique_ptr<OverrideSet>> overrideSets;

      bool identifyInitialStoringInstructions (DominatorSummary &DS) ;

      bool areOverrideSetsFullyCoveringTheAllocationSpace (void) const ;

      bool isOverrideSetFullyCoveringTheAllocationSpace (OverrideSet *overrideSet) const ;

  };

}