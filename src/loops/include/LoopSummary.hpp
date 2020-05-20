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
      LoopSummary (Loop *l);

      LoopSummary (Loop *l, LoopSummary *parentLoop);

      LoopSummary (Loop *l, LoopSummary *parentLoop, uint64_t loopTripCount);

      uint64_t getID (void) const ;

      BasicBlock * getHeader (void) const ;

      uint32_t getNestingLevel (void) const ;

      bool doesHaveCompileTimeKnownTripCount (void) const ;
      
      uint64_t getCompileTimeTripCount (void) const ;

      LoopSummary * getParentLoop (void) const ;
      
      void setParentLoop (LoopSummary *parentLoop) ;

      std::unordered_set<LoopSummary *> getChildren (void) const ;

      void addChild (LoopSummary *child) ;

      std::unordered_set<BasicBlock *> getLatches (void) const ;

      std::unordered_set<BasicBlock *> getBasicBlocks (void) const ;

      bool isBasicBlockWithin (BasicBlock *bb) const ;

      bool isLoopInvariant (Value *v);

      void print (raw_ostream &stream);
      
      std::vector<BasicBlock *> orderedBBs;

    private:
      uint64_t ID;
      BasicBlock *header;
      uint32_t depth;
      std::set<Value *> invariants;
      LoopSummary *parent;
      bool compileTimeKnownTripCount;
      uint64_t tripCount;
      std::unordered_set<LoopSummary *> children;
      std::unordered_set<BasicBlock *> latchBBs;
      std::unordered_set<BasicBlock *> bbs;
      static uint64_t globalID;
  };

}
