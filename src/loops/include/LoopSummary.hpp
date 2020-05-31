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

using namespace llvm;

namespace llvm {

  class LoopSummary {
    public:
      LoopSummary (
        BasicBlock *header,
        std::function<Loop * (BasicBlock *header)> getLLVMLoop
        );

      LoopSummary (
        BasicBlock *header,
        std::function<Loop * (BasicBlock *header)> getLLVMLoop,
        LoopSummary *parentLoop
        );

      LoopSummary (
        BasicBlock *header,
        std::function<Loop * (BasicBlock *header)> getLLVMLoop,
        LoopSummary *parentLoop, 
        uint64_t loopTripCount
        );

      uint64_t getID (void) const ;

      Function * getFunction (void) const ;

      BasicBlock * getHeader (void) const ;

      BasicBlock * getPreHeader (void) const ;

      /*
       * Return the nesting level of the loop.
       * 1 means outermost loop.
       */
      uint32_t getNestingLevel (void) const ;

      bool doesHaveCompileTimeKnownTripCount (void) const ;
      
      uint64_t getCompileTimeTripCount (void) const ;

      Loop * getLLVMLoop (void) const ;

      LoopSummary * getParentLoop (void) const ;
      
      void setParentLoop (LoopSummary *parentLoop) ;

      std::unordered_set<LoopSummary *> getChildren (void) const ;

      void addChild (LoopSummary *child) ;

      std::unordered_set<BasicBlock *> getLatches (void) const ;

      std::unordered_set<BasicBlock *> getBasicBlocks (void) const ;

      std::vector<BasicBlock *> getLoopExitBasicBlocks (void) const ;

      bool isBasicBlockWithin (BasicBlock *bb) const ;

      bool isLoopInvariant (Value *v);

      void print (raw_ostream &stream);
      
      std::vector<BasicBlock *> orderedBBs;

    private:
      uint64_t ID;
      BasicBlock *header;
      BasicBlock *preHeader;
      uint32_t depth;
      std::set<Value *> invariants;
      LoopSummary *parent;
      bool compileTimeKnownTripCount;
      uint64_t tripCount;
      std::unordered_set<LoopSummary *> children;
      std::unordered_set<BasicBlock *> latchBBs;
      std::unordered_set<BasicBlock *> bbs;
      std::function<Loop * (BasicBlock *)> getLLVMLoopExternalFunction;

      /*
       * Certain parallelization schemes rely on indexing exit blocks, so some arbitrary order needs to be established
       * The reason that ordering isn't dictated later is to maintain reproducibility; LLVM returns a vector to begin
       * with, so losing that ordering and re-establishing it from an unordered data structure is fraught.
       */
      std::vector<BasicBlock *> exitBlocks;

      static uint64_t globalID;

      void instantiateIDsAndBasicBlocks(Loop *llvmLoop) ;
  };

}
