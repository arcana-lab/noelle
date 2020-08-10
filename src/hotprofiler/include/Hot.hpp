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
#include "SCC.hpp"
#include "LoopStructure.hpp"

namespace llvm {

  class Hot {
    public:

      Hot ();

      bool isAvailable (void) const ;

      /*
       * =========================== Instructions ================================
       */
      uint64_t getStaticInstructions (Instruction *i) const ;
      
      bool hasBeenExecuted (Instruction *i) const ;

      uint64_t getInvocations (Instruction *i) const ;

      uint64_t getSelfInstructions (Instruction *i) const ;

      uint64_t getTotalInstructions (Instruction *i) const ;


      /*
       * =========================== Basic blocks ================================
       */
      uint64_t getStaticInstructions (BasicBlock *bb) const ;

      bool hasBeenExecuted (BasicBlock *bb) const ;

      uint64_t getInvocations (BasicBlock *bb) const ;

      uint64_t getSelfInstructions (BasicBlock *bb) const ;

      uint64_t getTotalInstructions (BasicBlock *bb) const ;

      void setBasicBlockInvocations (BasicBlock *bb, uint64_t invocations);


      /*
       * =========================== SCC  ========================================
       */

      uint64_t getSelfInstructions (SCC *scc) const ;

      uint64_t getTotalInstructions (SCC *scc) const ;


      /*
       * =========================== Loops =======================================
       */
      
      /*
       * Return the number of times the loop @l has been executed.
       */
      uint64_t getInvocations (LoopStructure *l) const ;

      /*
       * Return the total number of iterations executed by @l among all its invocations.
       */
      uint64_t getIterations (LoopStructure *l) const ;

      /*
       * Return the total number of instructions executed excluding the instructions executed by the callees.
       */
      uint64_t getSelfInstructions (LoopStructure *loop) const ;

      /*
       * Return the total number of instructions executed including the instructions executed by the callees.
       */
      uint64_t getTotalInstructions (LoopStructure *loop) const ;

      /*
       * Return the coverage of @loop
       *
       * @return Between 0 and 1
       */
      double getDynamicTotalInstructionCoverage (LoopStructure *loop) const ;

      double getAverageTotalInstructionsPerInvocation (LoopStructure *loop) const ;

      double getAverageLoopIterationsPerInvocation (LoopStructure *loop) const ;

      double getAverageTotalInstructionsPerIteration (LoopStructure *loop) const ;

      /*
       * =========================== Functions ==================================
       */

      bool hasBeenExecuted (Function *f) const ;

      uint64_t getInvocations (Function *f) const ;

      uint64_t getSelfInstructions (Function *f) const ;

      uint64_t getTotalInstructions (Function *f) const ;

      double getDynamicTotalInstructionCoverage (Function *f) const ;


      /*
       * =========================== Module ======================================
       */

      bool hasBeenExecuted (void) const ;

      bool getInvocations (void) const ;

      uint64_t getSelfInstructions (void) const ;

      uint64_t getTotalInstructions (void) const ;

 
      /*
       * =========================== Branches ====================================
       */
      double getBranchFrequency (BasicBlock *sourceBB, BasicBlock *targetBB) const ;

      void setBranchFrequency (BasicBlock *src, BasicBlock *dst, double branchFrequency);


      void computeProgramInvocations (Module &M);

    private:
      std::unordered_map<BasicBlock *, std::unordered_map<BasicBlock *, double>> branchProbability;
      std::unordered_map<BasicBlock *, uint64_t> bbInvocations;
      std::unordered_map<Function *, uint64_t> functionInvocations;
      std::unordered_map<Function *, uint64_t> functionSelfInstructions;
      std::unordered_map<Function *, uint64_t> functionTotalInstructions;
      std::unordered_map<Instruction *, uint64_t> instructionTotalInstructions;
      uint64_t moduleNumberOfInstructionsExecuted;

      void computeTotalInstructions (Module &M); 

      void computeTotalInstructions (Function &F, std::unordered_map<Function *, bool> &evaluationStack);

      void setFunctionTotalInstructions (Function *f, uint64_t totalInstructions) ;

      bool isFunctionTotalInstructionsAvailable (Function &F) const ;
  };

}
