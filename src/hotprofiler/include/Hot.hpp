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

namespace llvm {

  class Hot {
    public:

      Hot ();

      bool isAvailable (void) const ;

      /*
       * Basic blocks.
       */
      void setBasicBlockInvocations (BasicBlock *bb, uint64_t invocations);

      uint64_t getBasicBlockInvocations (BasicBlock *bb) const ;

      uint64_t getBasicBlockInstructions (BasicBlock *bb) const ;


      /*
       * Loops
       */

      /*
       * Return the total number of instructions executed excluding the instructions executed by the callees.
       */
      uint64_t getLoopSelfInstructions (Loop *loop) const ;

      /*
       * Return the total number of instructions executed including the instructions executed by the callees.
       */
      uint64_t getLoopTotalInstructions (Loop *loop) const ;

      /*
       * Set the total number of instructions executed by the loop.
       */
      void setLoopTotalInstructions (Loop *loop, uint64_t insts);


      /*
       * Functions
       */
      uint64_t getFunctionInstructions (Function *f) const ;

      uint64_t getFunctionInvocations (Function *f) const ;


      /*
       * Module
       */
      uint64_t getModuleInstructions (void) const ;

 
      /*
       * Branches
       */
      double getBranchFrequency (BasicBlock *sourceBB, BasicBlock *targetBB) const ;

      void setBranchFrequency (BasicBlock *src, BasicBlock *dst, double branchFrequency);


      void computeProgramInvocations (void);

    private:
      std::map<BasicBlock *, uint64_t> bbInvocations;
      std::map<Function *, uint64_t> functionInstructions;
      std::map<Function *, uint64_t> functionInvocations;
      std::map<BasicBlock *, std::map<BasicBlock *, double>> branchProbability;
      std::unordered_map<Loop *, uint64_t> totalLoopInstructions;
      uint64_t moduleNumberOfInstructionsExecuted;
  };

}
