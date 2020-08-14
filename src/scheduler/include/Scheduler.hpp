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

  enum ScheduleOption {
    Shrink=0,
    Expand
  };

  class LoopSchedulerContext {

    public:

      /*
       * Methods
       */
      LoopSchedulerContext(
        LoopStructure * const LS
      );

      void Dump (void) const ;


      /*
       * Pass analysis state
       */ 
      LoopStructure *TheLoop;


      /*
       * Derived analysis state
       */
      BasicBlock *OriginalLatch;
      std::set<BasicBlock *> Blocks;
      std::vector<std::pair<BasicBlock *, BasicBlock *>> ExitEdges;


      /*
       * New analysis state
       */
      bool PrologueCalculated=false;
      bool BodyCalculated=false;
      bool DiscrepancyExists=false;  /* Discrepancy between analysis
                                        state and loop structure */
      std::set<BasicBlock *> Prologue;
      std::set<BasicBlock *> Body;

  };

  class Scheduler {

    public:

      /*
       * For simplicity and time --- the scheduler has 
       * NOT been interfaced properly. The sole concern
       * of the scheduler is to shrink the loop prologue
       * at this time
       * 
       * TODO --- Engine, generalization
       * 
       */ 

      /*
       * Constructors
       */ 

      Scheduler ();


      /*
       * Drivers
       */

      bool scheduleBasicBlock(
        BasicBlock *Block,
        PDG *ThePDG,
        ScheduleOption Option=ScheduleOption::Shrink
      ) const ;

      bool pushInstructionOut(
        Instruction *I,
        BasicBlock *Block
      ) const ;


      /*
       * Methods
       */

      bool shrinkLoopPrologue (
        LoopStructure * const LS,
        DomTreeSummary const &PDT,
        Function *F,
        PDG *ThePDG,
        LoopSchedulerContext *LSC=nullptr
      ) const ;

      std::set<BasicBlock *> getLoopPrologue (
        LoopStructure * const LS,
        DomTreeSummary const &PDT,
        LoopSchedulerContext *LSC=nullptr,
        bool UpdateContext=false
      ) const ;

      std::set<BasicBlock *> getLoopBody (
        LoopStructure * const LS,
        DomTreeSummary const &PDT,
        LoopSchedulerContext *LSC=nullptr,
        bool UpdateContext=false,
        std::set<BasicBlock *> *PassedPrologue=nullptr
      ) const ;

  };

}
