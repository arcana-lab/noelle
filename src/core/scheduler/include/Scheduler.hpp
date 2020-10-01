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
      bool canMoveAnyInstOutOfBasicBlock( 
        BasicBlock * const Block
      ) const ;

      std::set<Instruction *> getAllInstsMoveableOutOfBasicBlock(
        BasicBlock * const Block,
        PDG * const ThePDG,
        ScheduleDirection Direction=ScheduleDirection::Down
      ) const ;

      bool canMoveInstOutOfBasicBlock(
        Instruction * const I
      ) const ;

      std::set<Instruction *> getAllInstsToMoveForSpecifiedInst(
        Instruction * const I,
        PDG * const ThePDG,
        ScheduleDirection Direction=ScheduleDirection::Down
      ) const ;


      /*
       * Analysis methods
       */ 
      bool isControlEquivalent(
        BasicBlock * const First,
        BasicBlock * const Second,
        DominatorSummary const &DS
      ) const ;

      std::set<Value *> getAllOutgoingDependences(
        Instruction * const I,
        PDG * const ThePDG
      ) const ;

      std::set<Instruction *> getOutgoingDependencesInParentBasicBlock(
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
      LoopStructure * getLoop (void) const ;
      
      std::set<BasicBlock *> getLoopPrologue (void) const ;

      std::set<BasicBlock *> getLoopBody (void) const ;


      /*
       * Analysis methods
       */ 
      bool canMoveAnyInstOutOfLoop (void) const;


      bool canQuicklyHandleLoop(void) const ;/* 
                                              * An arbitrary method to confirm 
                                              * that the scheduler does not become 
                                              * a compilation bottleneck  
                                              */ 


      /*
       * Transformation methods
       */ 

      /*
       * FIX --- This method performs the mechanism, but does
       * not give the user an option to decide whether or not
       * the prologue should be shrunk --- want to separate 
       * user policy from the mechanism
       * 
       * Complications:
       * - The LoopScheduler takes advantage of noelle-enable's 
       *   reinvocation scheme (the LoopScheduler currently to be
       *   called from enabler passes). As a result, providing
       *   an API that returns the set of instructions that 
       *   can be moved, and possibly where those instructions
       *   will be moved to (upon a fixed point) is non-trivial
       *   to calculate
       * - The user can decide to move instructions out of 
       *   prologue basic blocks based on the Scheduler method
       *   getAllInstsMoveableOutOfBasicBlock. However, this is
       *   not entirely obvious or completely useful to users.
       */ 
      bool shrinkLoopPrologue (void) ;


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
      bool mergePrologueBasicBlocks(void) ;

      bool shrinkPrologueBasicBlock(
        BasicBlock *Block
      );

      bool moveInstOutOfPrologueBasicBlock(
        Instruction *I,
        ValueToValueMapTy &OriginalToClones,
        std::set<Instruction *> &Clones,
        ScheduleDirection Direction=ScheduleDirection::Down
      );

      bool moveInstIntoSuccessor(
        Instruction *I,
        BasicBlock *Successor
      );

      bool cloneInstIntoSuccessor(
        Instruction *I,
        BasicBlock *Successor,
        ValueToValueMapTy &OriginalsToClones,
        std::set<Instruction *> &Clones
      );

      void remapClonedInstructions(
        ValueToValueMapTy &OriginalToClones,
        std::set<Instruction *> &Clones
      );

      void resolveSuccessorPHIs(
        Instruction * const Moved, /* Find PHIs to resolve using @Moved */
        Instruction * const Replacement, /* Replace uses with @Replacement */
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
      const uint32_t MaxPrologueSizeToHandle=16; /* Arbitrary, for performance concerns */
      bool SafeToDump=true;
      bool DiscrepancyExists=false;  /* Discrepancy between analysis
                                        state and loop structure */
      std::set<BasicBlock *> Prologue;
      std::set<BasicBlock *> Body;

  };

}
