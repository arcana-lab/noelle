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
#include <set>

namespace llvm::noelle {

  enum ScheduleDirection {
    
    Up=0,
    Down

  };

  /*
   * Forward declarations
   */ 
  class LoopScheduler;


  /*
   * Scheduler = per Noelle object
   */ 
  class Scheduler {

    public: 

      /*
       * Constructors
       */ 
      Scheduler ();


      /*
       * Scheduler builder methods --- FIX
       */ 
      LoopScheduler getNewLoopScheduler(
        LoopStructure * const LS,
        DominatorSummary * const DS,
        PDG * const ThePDG
      ) const ;


      /*
       * Driver methods
       */ 
      bool canScheduleBlock(
        BasicBlock * const Block  
      ) const ;

      std::set<Instruction *> getInstructionsThatCanMove(
        BasicBlock * const Block,
        PDG * const ThePDG,
        ScheduleDirection Direction=ScheduleDirection::Down
      ) const ;

      bool canMoveInstruction(
        Instruction * const I
      ) const ;

      std::set<Instruction *> getRequirementsToMoveInstruction(
        Instruction * const I,
        PDG * const ThePDG,
        ScheduleDirection Direction=ScheduleDirection::Down
      ) const ;


      /*
       * Transformation methods --- FIX
       */ 


      /*
       * Analysis methods
       */ 
      bool isControlEquivalent(
        BasicBlock * const First,
        BasicBlock * const Second,
        DominatorSummary const &DS
      ) const ;

      std::set<Value *> getOutgoingDependences(
        Instruction * const I,
        PDG * const ThePDG
      ) const ;

      std::set<Instruction *> getOutgoingDependencesInParent(
        Instruction * const I,
        PDG * const ThePDG
      ) const ;
    

  };

  /*
   * LoopScheduler = per invocation/per loop
   */ 
  class LoopScheduler : public Scheduler {

    public:

      /*
       * --- Public Methods ---
       */ 

      /*
       * Constructors
       */ 
      LoopScheduler(
        LoopStructure * const LS,
        DominatorSummary * const DS,
        PDG * const ThePDG
      );


      /*
       * Getter methods
       */ 
      LoopStructure * getPassedLoop (void) const ;
      
      std::set<BasicBlock *> getLoopPrologue (void) const ;

      std::set<BasicBlock *> getLoopBody (void) const ;


      /*
       * Analysis methods
       */ 
      bool canScheduleLoop (void) const;


      /*
       * Transformation methods
       */ 
      bool shrinkLoopPrologue (
        PDG * const ThePDG
      ) const ;


      /*
       * Debugging
       */ 
      void Dump (void) const ;
      

    private:

      /*
       * --- Private Methods ---
       */ 

      /*
       * Analysis methods
       */ 
      void gatherNecessaryLoopState (void) ;

      void calculateLoopPrologue (void) ;

      void calculateLoopBody (void) ;


      /*
       * Transformation methods
       */ 
      bool shrinkPrologueBlock(
        BasicBlock *Block
      );

      bool moveFromPrologueBlock(
        Instruction *I,
        ScheduleDirection Direction=ScheduleDirection::Down
      );

      bool moveIntoSuccessor(
        Instruction *I,
        BasicBlock *Successor
      );

      bool cloneIntoSuccessor(
        Instruction *I,
        BasicBlock *Successor
      );

      void resolveSuccessorPHIs(
        Instruction * const Moved,
        Instruction * const Replacement,
        BasicBlock *SuccBB
      );
      
      /*
       * --- Private Fields ---
       */ 

      /*
       * Passed state
       */ 
      LoopStructure *TheLoop;
      DominatorSummary *DS;
      PDG *ThePDG;


      /*
       * Derived analysis state
       */
      BasicBlock *OriginalLatch;
      std::set<BasicBlock *> Blocks;
      std::vector<std::pair<BasicBlock *, BasicBlock *>> ExitEdges;


      /*
       * New analysis state
       */
      bool DiscrepancyExists=false;  /* Discrepancy between analysis
                                        state and loop structure */
      std::set<BasicBlock *> Prologue;
      std::set<BasicBlock *> Body;

  };

}
