/*
 * Copyright 2019 - 2020  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"
#include "LoopDependenceInfo.hpp"
#include "DominatorSummary.hpp"

namespace llvm::noelle {

  class LoopSchedulerContext {

    public:

      /*
       * Constructor
       */
      LoopSchedulerContext(
        LoopStructure * const LS
      );


      /*
       * Pass analysis state
       */ 
      LoopStructure *TheLoop;


      /*
       * Derived analysis state
       */
      BasicBlock *OriginalLatch;
      std::unordered_set<BasicBlock *> Blocks;
      std::vector<std::pair<BasicBlock *, BasicBlock *>> ExitEdges;



      /*
       * New analysis state
       */
      bool PrologueCalculated=false;
      bool BodyCalculated=false;
      bool DiscrepancyExists=false;  /* Discrepancy between analysis
                                        state and loop structure */
      std::unordered_set<BasicBlock *> Prologue;
      std::unordered_set<BasicBlock *> Body;

  };

  class Scheduler {

    public:

      /*
       * Methods
       */

      /*
       * For simplicity and time --- the scheduler has 
       * NOT been interfaced properly. The sole concern
       * of the scheduler is to shrink the loop prologue
       * at this time
       * 
       * TODO --- Engine, generalization
       * 
       */ 
      Scheduler ();

      bool shrinkLoopPrologue (
        LoopDependenceInfo const &LDI,
        DomTreeSummary const &PDT
      ) const ;

      std::vector<BasicBlock *> getLoopPrologue (
        LoopStructure * const LS,
        DomTreeSummary const &PDT,
        LoopSchedulerContext *LSC=nullptr
      ) const ;

      std::vector<BasicBlock *> getLoopBody (
        LoopDependenceInfo const &LDI,
        DomTreeSummary const &PDT,
        LoopSchedulerContext *LSC=nullptr,
        std::vector<BasicBlock *> *Prologue=nullptr
      ) const ;

  };

}
