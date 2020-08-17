/*
 * Copyright 2019 - 2020  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Scheduler.hpp"

using namespace llvm::noelle;

/*
 * ------------------------------------------------------------------ 
 * *** SCHEDULER ***
 * ------------------------------------------------------------------
 */

/*
 * ------------------------------------------------------------------ 
 * Constructors
 * ------------------------------------------------------------------
 */

Scheduler::Scheduler() {
  return;
} 


LoopScheduler Scheduler::getNewLoopScheduler(
  LoopStructure * const LS,
  DominatorSummary const &DS
) const {

  return LoopScheduler(LS, DS);

}

/*
 * ------------------------------------------------------------------ 
 * PUBLIC --- Driver Methods
 * ------------------------------------------------------------------
 */

/*
 * BasicBlock Drivers
 */  
bool Scheduler::canScheduleBlock(
  BasicBlock * const Block
) const {

  /* 
   * TOP --- Decision is in the context of the ENTIRE CFG
   * 
   * Instructions in @Block can be scheduled only if the
   * following conditions are held:
   * 
   * 1. If the basic block does NOT end in anything other than a BranchInst
   *    - InvokeInst --- (want to avoid possible fiascos involving 
   *      exceptional control flow)
   *    - IndirectBr --- (want to avoid numerous possible landing points)
   * 
   * 2. Successors must have a single predecessor that is @Block
   *    - This is a fairly strong constraint because the current 
   *      capacity of the scheduler is to handle common but simple 
   *      control flow patterns. For example, the constraint prevents 
   *      scheduling of @Block's instructions into critical edges
   *      (already unlikely becauase of noelle-norm) and into 
   *      possible loops (don't want to expand hot loop prologues
   *      by accident)
   *    - TODO : Relax this constraint
   */ 
  
  errs() << "Scheduler: canScheduleBlock --- @Block: " << *Block << "\n";


  /*
   * <Constraint 1.>
   */ 
  Instruction *BlockTerminator = Block->getTerminator();

  if (!(isa<BranchInst>(BlockTerminator))) {

    errs() << "Scheduler:     No! @Block terminator is not a branch\n";
    return false;

  }


  /*
   * <Constraint 2.>
   */ 
  for (auto *SuccBB : successors(Block)) {
  
    auto SinglePred = SuccBB->getSinglePredecessor();

    if (!SinglePred) {

      errs() << "Scheduler:     No! A successor does not have a single predecessor == @Block\n";
      return false;

    }

  }


  errs() << "Scheduler:     Yes!\n"
         << "Scheduler:     Success for canScheduleBlock...\n";

  return true;

}


std::set<Instruction *> Scheduler::getInstructionsThatCanMove(
  BasicBlock * const Block,
  PDG * const ThePDG,
  ScheduleDirection Direction
) const {

  errs() << "Scheduler: getInstructionsThatCanMove --- @Block: " << *Block << "\n";

  auto Moves = std::set<Instruction *>();

  /*
   * <Constraint 1.>
   */ 
  if (Direction != ScheduleDirection::Down) {
      
    errs() << "Scheduler:     No instructions --- Direction to move is not down!\n" << *Block << "\n";
    return Moves;

  }


  /*
   * <Constraint 2. --- Context = ENTIRE CFG>
   */ 
  errs() << "Scheduler:     Checking the block ...\n";

  if (!(this->canScheduleBlock(Block))) {
    
    errs() << "Scheduler:     No instructions --- Block can't be scheduled!\n" << *Block << "\n";
    return Moves;

  }


  /*
   * Perform the following algorithm:
   * 
   * for I in all instructions of @Block (starting at the terminator):
   *    a. If I cannot be moved, add to Keeps set
   *    b. Iterate over outgoing dependences of I. If any 
   *       dependence is a member of the Keeps set or not part 
   *       of @Block add I to Keeps and continue. Otherwise, 
   *       add I to Moves set and continue.
   * 
   * NOTE --- this algorithm ignores possible cycles that arise
   * from alias analysis --- because this is in the context of a
   * single basic block, cyclic dependencies can be ignored
   */
  
  
  /*
   * Set up worklist --- reverse instruction iteration
   */ 
  std::queue<Instruction *> WorkList;
  std::set<Instruction *> Keeps;

  for (auto IIT = Block->rbegin(); 
       IIT != Block->rend();
       ++IIT) {
    
    Instruction &I = *IIT;
    WorkList.push(&I);
    
  }


  /*
   * Iterate through the worklist, get outgoing dependences
   * of each instruction in the worklist and determine whether
   * the instructions should be kept or moved
   * 
   * Context = @Block
   */ 
  errs() << "Scheduler:     Now the worklist...\n";

  while (!WorkList.empty()) {

    /*
     * Set up the next instruction
     */ 
    Instruction *Next = WorkList.front();
    WorkList.pop();

    errs() << "Scheduler:       Next: " << *Next << "\n";


    /*
     * Check if the instruction can be moved, if not 
     * add to the Keeps set and continue
     */ 
    if (!(this->canMoveInstruction(Next))) {

      errs() << "Scheduler:         Keep --- Can't move Next!\n";

      Keeps.insert(Next);
      continue;

    }


    /*
     * Get the outgoing dependence instructions that reside in @Block
     */ 
    errs() << "Scheduler:       Now the dependences...\n";

    auto Outgoing =
      this->getOutgoingDependencesInParent(
        Next,
        ThePDG
      );

    
    /*
     * Filter the dependences, determine status for Next
     */ 
    bool ShouldKeep = false;
    for (auto D : Outgoing) {
    
      errs() << "         D: " << *D << "\n";

      /*
       * Check if the dependence can be moved or belongs
       * to the Keeps set --- Next must be kept in this
       * case
       */ 
      if (false
          || !(this->canMoveInstruction(D))
          || Keeps.find(D) != Keeps.end()) {

        ShouldKeep |= true;
        break;
      
      }

    }

    if (ShouldKeep) {

      errs() << "LoopScheduler:       Keep --- Dependence(s) can't be moved!\n";
      Keeps.insert(Next);

    } else {

      errs() << "LoopScheduler:       Move!\n";
      Moves.insert(Next);

    }

  }


  /*
   * Debugging
   */ 
  errs() << "Scheduler: getInstructionsThatCanMove --- All moves: \n";
  
  for (auto Move : Moves) {
    errs() << "Scheduler:   " << *Move << "\n";
  }


  return Moves;

}


/*
 * Instruction Drivers
 */  
bool Scheduler::canMoveInstruction(
  Instruction * const I
) const {

  /*
   * TOP --- Decision is in the context of JUST @I 
   * 
   * @I can only be moved if it is NOT a PHINode or a terminator
   */ 

  errs() << "Scheduler: canMoveInstruction --- @I: " << *I << "\n";

  /*
   * <Constraint>
   */ 
  if (false
      || (isa<PHINode>(I))
      || (I->isTerminator())) {

    errs() << "Scheduler:     No! @I is a PHI or terminator\n";
    return false;

  }


  errs() << "Scheduler:     Yes!\n"
         << "Scheduler:     Success for canMoveInstruction...\n";

  return true;

}


std::set<Instruction *> Scheduler::getRequirementsToMoveInstruction(
  Instruction * const I,
  PDG * const ThePDG,
  ScheduleDirection Direction
) const {

  /*
   * TOP --- Decision is in the context @I's PARENT BASIC BLOCK
   * 
   * A user wants to move @I out of its parent basic 
   * block in a given direction --- calculate the set of 
   * instructions that reside in the same parent basic block 
   * that must be moved IN ADDITION to @I for the given 
   * direction IFF @I can be moved in the first place
   * 
   * If @I cannot be moved (for any reason), return an empty set
   * 
   * Example scenarios:
   * - If @I is a condition of a branch, the worklist for the
   *   dependences will eventually determine that moving @I is
   *   not possible, abort the iteration, and return an empty set
   * - If @I is a PHINode, return an empty set immediately
   * 
   * Constraints:
   *   1. Can only handle the "Down" direction for movement (TEMPORARY)
   *
   */ 

  auto Requirements = std::set<Instruction *>();

  errs() << "Scheduler: getRequirementsToMoveInstruction --- @I: " << *I << "\n";

  /*
   * <Constraint 1.>
   */ 
  if (Direction != ScheduleDirection::Down) {
      
    errs() << "Scheduler:     Can't get requirements --- Direction to move is not down!\n";
    return Requirements;

  }


  /*
   * Sanity check @I
   */ 
  if (!(this->canMoveInstruction(I))) {

    errs() << "Scheduler:     Can't get requirements --- @I can't be moved!\n";
    return Requirements;

  }


  /*
   * Execute the following:
   *   1. Iterate through dependences of the next instruction in 
   *      the worklist, adding to the Requirements set when necessary, 
   *   2. Add outgoing dependences to the worklist according to 
   *      Scheduler::getOutgoingDependencesInParent
   */ 
  BasicBlock *Parent = I->getParent();

  std::queue<Instruction *> WorkList;
  WorkList.push(I);
  Requirements.insert(I);

  errs() << "Scheduler:     Now the dependences...\n";

  while (!WorkList.empty()) {

    /*
     * Get the next instruction
     */  
    Instruction *Next = WorkList.front();
    WorkList.pop();

    errs() << "Scheduler:     Next: " << *Next << "\n";


    /*
     * Get the outgoing dependence instructions
     */ 
    auto Outgoing =
      this->getOutgoingDependencesInParent(
        Next,
        ThePDG
      );

    
    /*
     * Filter the dependences
     */ 
    for (auto D : Outgoing) {

      errs() << "Scheduler:       D: " << *D << "\n";

      /*
       * If the instruction can't be moved abort the computation
       */ 
      if (!(this->canMoveInstruction(D))) {

        errs() << "Scheduler:         Can't get requirements --- A dependence can't be moved!\n";
        return std::set<Instruction *>();

      }

      /*
       * Add back worklist
       */ 
      WorkList.push(D);

    }

  }


  return Requirements;


}



/*
 * ------------------------------------------------------------------ 
 * PUBLIC --- Transformation Methods
 * ------------------------------------------------------------------
 */
bool Scheduler::moveFromBlock(
  Instruction * const I,
  PDG * const ThePDG,
  DominatorSummary const &DS
) const {

  return false;

}



/*
 * ------------------------------------------------------------------ 
 * PUBLIC --- Analysis Methods
 * ------------------------------------------------------------------
 */
bool Scheduler::isControlEquivalent(
  BasicBlock * const First,
  BasicBlock * const Second,
  DominatorSummary const &DS
) const {

  bool IsControlEquivalent = true;

  /*
   * Get dominator and post-dominator trees, set up
   */ 
  auto DT = DS.DT;
  auto PDT = DS.PDT;


  /*
   * Check if @First dominates @Second
   */ 
  IsControlEquivalent &= DT.dominates(First, Second);


  /*
   * Check if @Second post-dominate @First
   */
  IsControlEquivalent &= PDT.dominates(Second, First);


  /*
   * Debugging
   */
  errs() << "LoopScheduler: First --- \n" << *First << "\n"
         << "LoopScheduler: Second --- \n" << *Second << "\n"
         << "LoopScheduler: IsControlEquivalent --- " 
         << IsControlEquivalent << "\n";


  return IsControlEquivalent;
  
}


std::set<Value *> Scheduler::getOutgoingDependences(
  Instruction * const I,
  PDG * const ThePDG
) const {

  /*
   * TOP --- Get all outgoing dependence values for @I based on the PDG
   */ 

  auto OutgoingDependences = std::set<Value *>();


  /*
   * Set up lambda iterator for PDG
   */ 
  auto Iterator = 
    [&OutgoingDependences]
    (Value *Outgoing, DataDependenceType D) -> bool {

    // errs() << "   D: " << *Outgoing << "\n";

    /*
     * Insert the dependence into the OutgoingDependences set
     */ 
    OutgoingDependences.insert(Outgoing);

    return false;

  };


  /*
   * Iterate over the PDG
   */ 
  auto Iterated = 
    ThePDG->iterateOverDependencesFrom(I, 
      false, /* Control dependences */
      true, /* Memory dependences */
      true, /* Register dependences */
      Iterator
    ); 


  return OutgoingDependences;

}



std::set<Instruction *> Scheduler::getOutgoingDependencesInParent(
  Instruction * const I,
  PDG * const ThePDG
) const {

  /*
   * TOP --- Get all outgoing dependence instructions for @I that 
   * exist in @I's parent basic block --- based on the PDG
   * 
   * Similar to getOutgoingDependences, but more constraints
   */ 

  auto OutgoingDependences = std::set<Instruction *>();


  /*
   * Set up lambda iterator for PDG
   */ 
  auto Iterator = 
    [I, &OutgoingDependences]
    (Value *Outgoing, DataDependenceType D) -> bool {

    // errs() << "   D: " << *Outgoing << "\n";

    /*
     * 1. Arguments, globals, and other values are outside
     *    of the basic block already, ignore
     * 
     * 2. If an instruction depends on itself, ignore
     * 
     * 3. If the instruction is not part of the block, ignore
     */
    Instruction *OutgoingInst = dyn_cast<Instruction>(Outgoing);
    
    if (false 
        || (!OutgoingInst) /* 1. */
        || (OutgoingInst == I) /* 2. */
        || (OutgoingInst->getParent() != I->getParent())) /* 3. */ {

      return false;

    }


    /*
     * Insert the dependence into the OutgoingDependences set
     */ 
    OutgoingDependences.insert(OutgoingInst);


    return false;

  };


  /*
   * Iterate over the PDG
   */ 
  auto Iterated = 
    ThePDG->iterateOverDependencesFrom(I, 
      false, /* Control dependences */
      true, /* Memory dependences */
      true, /* Register dependences */
      Iterator
    ); 


  return OutgoingDependences;

}




/*
 * ------------------------------------------------------------------ 
 * *** LOOP SCHEDULER ***
 * ------------------------------------------------------------------
 */

/*
 * ------------------------------------------------------------------ 
 * Constructors
 * ------------------------------------------------------------------
 */
LoopScheduler::LoopScheduler(
  LoopStructure * const LS,
  DominatorSummary const &DS
) {

  /*
   * Save passed analysis state
   */
  this->TheLoop = LS;
  this->DS = &DS;


  /*
   * TOP --- Acquire necessary derived state, sanity check the loop
   */
  this->gatherNecessaryLoopState();


  /*
   * Generate prologue and body
   */ 
  this->calculateLoopPrologue();
  this->calculateLoopBody();


  return;


}


/*
 * ------------------------------------------------------------------ 
 * PUBLIC --- Transformation Methods
 * ------------------------------------------------------------------
 */
bool LoopScheduler::shrinkLoopPrologue (
  PDG * const ThePDG
) const {

  return false;

}


/*
 * ------------------------------------------------------------------ 
 * PUBLIC --- Debugging
 * ------------------------------------------------------------------
 */
void LoopScheduler::Dump (void) const {

  errs() << "LoopScheduler: Starting dump ...\n";

  /*
   * Dump the loop blocks
   */
  errs() << "LoopScheduler: Blocks\n";
  
  for (auto Block : this->Blocks) {
    errs() << *Block << "\n";
  }


  /*
   * Dump the loop latch
   */
  errs() << "LoopScheduler: Latch\n"
         << *(this->OriginalLatch) << "\n";


  /*
   * Dump the loop prologue
   */
  errs() << "LoopScheduler: Prologue\n";
  
  for (auto Block : this->Prologue) {
    errs() << *Block << "\n";
  }


  /*
   * Dump the loop body
   */
  errs() << "LoopScheduler: Body\n";
  
  for (auto Block : this->Body) {
    errs() << *Block << "\n";
  }


  errs() << "LoopScheduler: End dump ...\n";
  return;

}


/*
 * ------------------------------------------------------------------ 
 * PUBLIC --- Getter Methods
 * ------------------------------------------------------------------
 */
LoopStructure * LoopScheduler::getPassedLoop (void) const {
  return this->TheLoop;
}

std::set<BasicBlock *> LoopScheduler::getLoopPrologue (void) const {
  return this->Prologue;
}

std::set<BasicBlock *> LoopScheduler::getLoopBody (void) const {
  return this->Body;
}


/*
 * ------------------------------------------------------------------ 
 * PRIVATE --- Analysis Methods
 * ------------------------------------------------------------------
 */
void LoopScheduler::gatherNecessaryLoopState (void) {

  /*
   * Acquire latch 
   */ 
  auto Latches = this->TheLoop->getLatches();

  assert(Latches.size() == 1
         && "Scheduler can't handle loops with multiple latches!");

  this->OriginalLatch = *(Latches.begin());


  /*
   * Acquire loop blocks
   */
  this->Blocks = 
    std::set(this->TheLoop->getBasicBlocks().begin(),
             this->TheLoop->getBasicBlocks().end());


  /*
   * Acquire exit edges
   */
  this->ExitEdges = 
    std::vector<std::pair<BasicBlock *, BasicBlock *>>(
      this->TheLoop->getLoopExitEdges()
    );


  return;

}


void LoopScheduler::calculateLoopPrologue (void) {

   /*
   * Get the post-dominator tree
   */  
  auto PDT = DS->PDT;


  /*
   * Prologue is calculated by finding all loop blocks NOT
   * post-dominated by the latch --- calculate this set
   */
  for (auto Block : this->Blocks) {

    auto DoesPostDominate = PDT.dominates(this->OriginalLatch, Block);

    if (!DoesPostDominate) {
      this->Prologue.insert(Block);
    }

  }
  

  return;

}


void LoopScheduler::calculateLoopBody (void) {

  /*
   * The body is the rest of the loop blocks that are NOT the
   * prologue --- calculate via a set difference
   */

  std::set_difference(
    this->Blocks.begin(), this->Blocks.end(),
    this->Prologue.begin(), this->Prologue.end(),
    std::inserter(this->Body, this->Body.begin())
  );
  

  return;

}

