/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "noelle/core/SystemHeaders.hpp"

namespace llvm::noelle {

  class LoopStructure {
    public:

      LoopStructure (
        Loop *l
        );

      uint64_t getID (void) const ;

      Function * getFunction (void) const ;

      BasicBlock * getHeader (void) const ;

      BasicBlock * getPreHeader (void) const ;

      Instruction * getEntryInstruction (void) const ;

      BasicBlock * getFirstLoopBasicBlockAfterTheHeader (void) const ;

      /*
       * Return the nesting level of the loop.
       * 1 means outermost loop.
       */
      uint32_t getNestingLevel (void) const ;

      std::unordered_set<BasicBlock *> getLatches (void) const ;

      std::unordered_set<BasicBlock *> getBasicBlocks (void) const ;

      std::unordered_set<Instruction *> getInstructions (void) const ;
      
      uint64_t getNumberOfInstructions (void) const ;

      std::vector<BasicBlock *> getLoopExitBasicBlocks (void) const ;

      std::vector<std::pair<BasicBlock *, BasicBlock *>> getLoopExitEdges (void) const ;

      uint64_t numberOfExitBasicBlocks (void) const ;

      bool isLoopInvariant (Value *value) const ;

      bool isIncluded (BasicBlock *bb) const ;

      bool isIncluded (Instruction *inst) const ;

      void print (raw_ostream &stream);
      
    private:
      uint64_t ID;
      BasicBlock *header;
      BasicBlock *preHeader;
      uint32_t depth;
      std::unordered_set<Instruction *> invariants;
      std::unordered_set<BasicBlock *> latchBBs;
      std::unordered_set<BasicBlock *> bbs;

      /*
       * Certain parallelization schemes rely on indexing exit blocks, so some arbitrary order needs to be established
       * The reason that ordering isn't dictated later is to maintain reproducibility; LLVM returns a vector to begin
       * with, so losing that ordering and re-establishing it from an unordered data structure is fraught.
       */
      std::vector<BasicBlock *> exitBlocks;
      std::vector<std::pair<BasicBlock *, BasicBlock *>> exitEdges;

      static uint64_t globalID;

      void instantiateIDsAndBasicBlocks(Loop *llvmLoop) ;

      bool isContainedInstructionLoopInvariant (Instruction *inst) const ;

  };

}
