/*
 * Copyright 2019 - 2020  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Scheduler.hpp"

using namespace llvm;
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
  DominatorSummary * const DS,
  PDG * const ThePDG
) const {

  return LoopScheduler(LS, DS, ThePDG);

}

/*
 * ------------------------------------------------------------------ 
 * PUBLIC --- Driver Methods
 * ------------------------------------------------------------------
 */

/*
 * BasicBlock Drivers
 */  
bool Scheduler::canMoveAnyInstOutOfBasicBlock(
  BasicBlock * const Block
) const {

  /* 
   * TOP --- Decision is in the context of the ENTIRE CFG
   * 
   * Instructions in @Block can be scheduled only if the
   * following conditions are held:
   * 
   * 1. If the basic block only ends in a BranchInst
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
  
  errs() << "Scheduler: canMoveAnyInstOutOfBasicBlock --- @Block: " << *Block << "\n";


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
         << "Scheduler:     Success for canMoveAnyInstOutOfBasicBlock...\n";

  return true;

}


std::set<Instruction *> Scheduler::getAllInstsMoveableOutOfBasicBlock(
  BasicBlock * const Block,
  PDG * const ThePDG,
  ScheduleDirection Direction
) const {

  errs() << "Scheduler: getAllInstsMoveableOutOfBasicBlock --- @Block: " << *Block << "\n";

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

  if (!(this->canMoveAnyInstOutOfBasicBlock(Block))) {
    
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
    if (!(this->canMoveInstOutOfBasicBlock(Next))) {

      errs() << "Scheduler:         Keep --- Can't move Next!\n";

      Keeps.insert(Next);
      continue;

    }


    /*
     * Get the outgoing dependence instructions that reside in @Block
     */ 
    errs() << "Scheduler:       Now the dependences...\n";

    auto Outgoing =
      this->getOutgoingDependencesInParentBasicBlock(
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
          || !(this->canMoveInstOutOfBasicBlock(D))
          || Keeps.find(D) != Keeps.end()) {

        ShouldKeep |= true;
        break;
      
      }

    }

    if (ShouldKeep) {

      errs() << "Scheduler:       Keep --- Dependence(s) can't be moved!\n";
      Keeps.insert(Next);

    } else {

      errs() << "Scheduler:       Move!\n";
      Moves.insert(Next);

    }

  }


  /*
   * Debugging
   */ 
  errs() << "Scheduler: getAllInstsMoveableOutOfBasicBlock --- All moves ("
         << Moves.size() << "): \n";
  
  for (auto Move : Moves) {
    errs() << "Scheduler:   " << *Move << "\n";
  }


  return Moves;

}


/*
 * Instruction Drivers
 */  
bool Scheduler::canMoveInstOutOfBasicBlock(
  Instruction * const I
) const {

  /*
   * TOP --- Decision is in the context of JUST @I 
   * 
   * @I can only be moved if it is NOT a PHINode or a terminator
   */ 

  errs() << "Scheduler: canMoveInstOutOfBasicBlock --- @I: " << *I << "\n";

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
         << "Scheduler:     Success for canMoveInstOutOfBasicBlock...\n";

  return true;

}


std::set<Instruction *> Scheduler::getAllInstsToMoveForSpecifiedInst(
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

  errs() << "Scheduler: getAllInstsToMoveForSpecifiedInst --- @I: " << *I << "\n";

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
  if (!(this->canMoveInstOutOfBasicBlock(I))) {

    errs() << "Scheduler:     Can't get requirements --- @I can't be moved!\n";
    return Requirements;

  }


  /*
   * Execute the following:
   *   1. Iterate through dependences of the next instruction in 
   *      the worklist, adding to the Requirements set when necessary, 
   *   2. Add outgoing dependences to the worklist according to 
   *      Scheduler::getOutgoingDependencesInParentBasicBlock
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
      this->getOutgoingDependencesInParentBasicBlock(
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
      if (!(this->canMoveInstOutOfBasicBlock(D))) {

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
   * Check if @Second post-dominates @First
   */
  IsControlEquivalent &= PDT.dominates(Second, First);


  /*
   * Debugging
   */
  errs() << "Scheduler: First --- \n" << *First << "\n"
         << "Scheduler: Second --- \n" << *Second << "\n"
         << "Scheduler: IsControlEquivalent --- " 
         << IsControlEquivalent << "\n";


  return IsControlEquivalent;
  
}


std::set<Value *> Scheduler::getAllOutgoingDependences(
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
    (Value *Outgoing, DGEdge<Value> *dep) -> bool {

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



std::set<Instruction *> Scheduler::getOutgoingDependencesInParentBasicBlock(
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
    (Value *Outgoing, DGEdge<Value> *dep) -> bool {

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
  DominatorSummary * const DS,
  PDG * const ThePDG
) {

  /*
   * Save passed analysis state
   */
  this->TheLoop = LS;
  this->DS = DS;
  this->ThePDG = ThePDG;


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
 * PUBLIC --- Analysis Methods
 * ------------------------------------------------------------------
 */
bool LoopScheduler::canMoveAnyInstOutOfLoop (void) const {

  /*
   * Scheduling instructions into/out of a loop is currently a 
   * constrained process --- the following constraints are checked:
   * 
   * 1. If the body is empty, the loop can't be scheduled (try 
   *    whilifying the loop first)
   * 
   * 2. Nothing else yet
   */ 

  errs() << "LoopScheduler:   canMoveAnyInstOutOfLoop\n";


  /*
   * <Constraint 1.>
   */ 
  if (!(this->Body.size())) {
  
    errs() << "LoopScheduler:     No! Loop body is empty\n";
    return false;

  }


  errs() << "LoopScheduler:     Yes! Loop can be scheduled\n";
  return true;

}


bool LoopScheduler::canQuicklyHandleLoop(void) const {

  /* 
   * TOP --- To handle performance concerns --- make sure that 
   * the number of basic blocks in the prologue is not larger 
   * than MaxPrologueSizeToHandle.
   * 
   * Do not handle loops where this constraint is not held.
   */  

  if (this->Prologue.size() > this->MaxPrologueSizeToHandle) {
    
    errs() << "LoopScheduler:     No! Too many blocks in the loop prologue\n";
    return false;

  }

  errs() << "LoopScheduler:     Yes! Loop can be quickly handled\n";
  return true;

}


/*
 * ------------------------------------------------------------------ 
 * PUBLIC --- Transformation Methods
 * ------------------------------------------------------------------
 */
bool LoopScheduler::shrinkLoopPrologue (void) {

  bool Modified = false;

  /*
   * Check constraints
   */  
  if (!(this->canMoveAnyInstOutOfLoop())) {

    errs() << "LoopScheduler:     Abort! Can't schedule the loop\n";
    return Modified;

  }

  
  if (!(this->canQuicklyHandleLoop())) {

    errs() << "LoopScheduler:     Can't seem to quickly handle this loop\n";

    /*
     * Attempt to merge prologue blocks to handle the issue, return
     * immediately anyway (if false == abort, if true == some merging
     * has occurred)
     */ 
    Modified |= this->mergePrologueBasicBlocks(); 
    this->SafeToDump &= false; /* Debugging */

    return Modified;

  }


  /*
   * Set up a worklist --- try shrinking the prologue --- if
   * there is any modification to any block, return immediately
   * (let noelle-enable start a new invocation to avoid the 
   * headaches of invalidated noelle analysis info) --- INEFFICIENT
   * 
   * Iteration is bottom-up --- FIX (modified inorder traversal 
   * is more efficient but unwise given the current setup)
   */ 
  std::queue<BasicBlock *> WorkList;
  std::set<BasicBlock *> ProcessedBlocks,
                         CannotProcessBlocks;

  /*
   * Set up
   */
  for (auto BlockIT = this->Prologue.rbegin();
       BlockIT != this->Prologue.rend();
       ++BlockIT) {

    BasicBlock *Block = *BlockIT;
    WorkList.push(Block);

  }

  /*
   * Iterate over the worklist
   */
  while(!WorkList.empty()) {

    /*
     * Get next basic block, check:
     * 
     * 1. Check block scheduling constraints --- if Next can't be
     *    scheduled, add to CannotProcessBlocks and continue
     * 
     * 2. If any successor can't be processed (in CannotProcessBlocks),
     *    don't process Next --- add Next to CannotProcessBlocks and 
     *    continue
     * 
     * 3. If any successor that is in the prologue has not been
     *    processed first (in ProcessedBlocks) --- don't process
     *    Next --- push it to the back of the worklist
     */ 

    BasicBlock *Next = WorkList.front();
    WorkList.pop();

    errs() << "LoopScheduler:       Next: " << *Next << "\n";


    /*
     * <Constraint 1.> 
     */ 
    if (!(this->canMoveAnyInstOutOfBasicBlock(Next))) {

      CannotProcessBlocks.insert(Next);
      continue;
      
    }


    /*
     * FOLLOWING FUNCTIONALITY NEEDS A HUGE FIX
     */

    bool ReadyToProcess = true,
         CannotProcess = false;

    for (auto *SuccBB : successors(Next)) {

      /*
       * <Constraint 2.>
       */ 
      if (CannotProcessBlocks.find(SuccBB) != CannotProcessBlocks.end()) {
        
        CannotProcess |= true;
        break;

      }

      /*
       * <Constraint 3.>
       */
      if (true 
          && this->Prologue.find(SuccBB) != this->Prologue.end()
          && ProcessedBlocks.find(SuccBB) == ProcessedBlocks.end()) {
        
        ReadyToProcess &= false;
        break;

      }

    }

    if (CannotProcess) { /* <Constraint 2.> */
      
      CannotProcessBlocks.insert(Next);
      continue;

    } else if (!ReadyToProcess) { /* <Constraint 3.> */

      WorkList.push(Next);
      continue;

    }


    /*
     * If there was a move, return immediately --- otherwise 
     * add Next to the ProcessedBlocks set and continue
     */ 
    Modified |= this->shrinkPrologueBasicBlock(Next);

    if (Modified) {
      return Modified;
    }

    ProcessedBlocks.insert(Next);

  }


  return Modified;

}


/*
 * ------------------------------------------------------------------ 
 * PUBLIC --- Debugging
 * ------------------------------------------------------------------
 */
void LoopScheduler::Dump (void) const {

  if (!(this->SafeToDump)) {

    errs() << "LoopScheduler: Not safe to dump --- returning...\n";
    return;

  }

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


  /*
   * Dump the parent function
   */ 
  errs() << "LoopScheduler: Parent Function\n"
         << *(this->TheLoop->getFunction()) << "\n";


  errs() << "LoopScheduler: End dump ...\n";
  return;

}


/*
 * ------------------------------------------------------------------ 
 * PUBLIC --- Getter Methods
 * ------------------------------------------------------------------
 */
LoopStructure * LoopScheduler::getLoop (void) const {
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
  auto PDT = this->DS->PDT;


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


/*
 * ------------------------------------------------------------------ 
 * PRIVATE --- Transformation Methods
 * ------------------------------------------------------------------
 */
bool LoopScheduler::mergePrologueBasicBlocks(void) {

  /*
   * TOP --- Merge prologue basic blocks whenever possible
   * 
   * This alleviates compilation time bottlenecks, since the 
   * scheduler currently relies on noelle-enable's reinvocation
   * scheme and returns after modification at a basic block
   * granularity
   */ 

  bool Modified = false;

  errs() << "LoopScheduler:       Attempting to merge prologue blocks\n";

  for (auto Block : Prologue) {
    Modified |= llvm::MergeBlockIntoPredecessor(Block);
  }

  return Modified;

}


bool LoopScheduler::shrinkPrologueBasicBlock(
  BasicBlock *Block
) {

  bool Modified = false;
 
  // DEBUGGING
  const uint32_t NumInstToMove = 4;
  uint32_t NumMoved = 0;


  /*
   * Find all instructions to move from @Block --- FIX --- VERY INEFFIEICNT
   */ 
  auto InstructionsToMove = 
    this->getAllInstsMoveableOutOfBasicBlock(
      Block,
      this->ThePDG 
    );


  /*
   * Nothing to move from the block
   */ 
  if (!(InstructionsToMove.size())) {
    return Modified;
  }


  /*
   * Get correct order for all possible instructions to move --- Horrible
   */ 
  std::vector<Instruction *> OrderedInstructionsToMove;

  for (auto IIT = Block->rbegin(); 
       IIT != Block->rend();
       ++IIT) {
  
    Instruction &Move = *IIT;
    
    if (InstructionsToMove.find(&Move) != InstructionsToMove.end()) {
      OrderedInstructionsToMove.push_back(&Move);

      // DEBUGGING
      NumMoved++;
    }

#if 0
    // DEBUGGING
    if (NumMoved == NumInstToMove) break;
#endif

  }


  /*
   * Perform the move or clone --- Terrible
   */ 
  ValueToValueMapTy OriginalToClones;
  std::set<Instruction *> Clones;

  for (auto Move : OrderedInstructionsToMove) {

    errs() << "LoopScheduler:       Next instruction to move: " << Move << "\n";

    this->moveInstOutOfPrologueBasicBlock(
      Move,
      OriginalToClones,
      Clones
    );

    Modified |= true;

  }


  /*
   * Remap all cloned instructions if necessary --- Attrocious
   */ 
  this->remapClonedInstructions(
    OriginalToClones,
    Clones
  );


  if (Modified) {
    this->Dump();
    errs() << *(this->TheLoop->getFunction()) << "\n";
  }


  return Modified;

} 


bool LoopScheduler::moveInstOutOfPrologueBasicBlock(
  Instruction *I,
  ValueToValueMapTy &OriginalToClones,
  std::set<Instruction *> &Clones,
  ScheduleDirection Direction
) {

  /*
   * TOP --- Main transformation method for moving an instruction 
   * outside of a basic block
   * 
   * Users --- LoopScheduler::shrinkLoopPrologue
   * 
   * Constraints:
   * 
   * 1. Scheduling direction --- moving out of a basic block is
   *    constrained to the ScheduleDirection::Down direction
   * 
   * 2. Because the current use case of this method is to shrink
   *    the loop prologue, there are constraints for where an 
   *    instruction can be moved down to:
   *    
   *    a. If @I's parent has a single successor --- moving is 
   *       allowed and trivial
   * 
   *    b. If @I's parent has a pair of successors (confirm 2), 
   *       confirm that one successor is inside the loop and 
   *       the other successor is an exit block
   * 
   *       NOTE --- this is quite a strong constraint, as the 
   *       LoopScheduler needs to address only certain, common
   *       loop CFG patterns (temporarily)
   * 
   * Other notes:
   * - Replacing uses is non-trivial in a general case. However, for the
   *   aforementioned common loop CFG pattern, there's a shortcut to 
   *   replacing uses:
   *    
   *   1. For the successor block inside the loop, @I can simply 
   *      be moved into the successor
   *   2. For the successor block outside the loop, @I needs to be 
   *      cloned and placed into the successor. However, because this
   *      is an exit block, noelle-norm must have already placed the 
   *      loop in lcssa form --- meaning the exit block's uses of @I
   *      are in its PHI nodes. As a result, simply resolving the uses
   *      in the exit block PHIs will suffice.
   * 
   * - Other common CFG patterns (if-then-else, etc.) should be
   *   handled using the dominator summary and reachability 
   *   analysis (not implemented)
   */ 

  BasicBlock *Parent = I->getParent();

  errs() << "LoopScheduler:   moveInstOutOfPrologueBasicBlock --- @I: " << *I << "\n";


  /*
   * CASE 1
   */ 
  if (Direction != ScheduleDirection::Down) {
      
    errs() << "LoopScheduler:     No instructions --- Direction to move is not down!\n";
    return false;

  }


  /*
   * CASE 2a. --- Parent has only one successor
   */ 
  BasicBlock *SingleSuccessor = Parent->getSingleSuccessor();

  if (SingleSuccessor) {
    return this->moveInstIntoSuccessor(I, SingleSuccessor);
  }


  /*
   * CASE 2b. --- Parent has two successors
   */ 
  
  /*
   * <Constraint 2b.> --- Sanity check number of successors
   */ 
  assert(succ_size(Parent) == 2
         && "LoopScheduler --- moveInstOutOfPrologueBasicBlock --- Case 2: Parent of @I should only have 2 successors!\n");

  
  /*
   * <Constraint 2b.> --- Position of successors
   */  
  uint32_t NumSuccessorsInside = 0;
  BasicBlock *InsideBlock = nullptr,
             *OutsideBlock = nullptr;

  for (auto *SuccBB : successors(Parent)) {

    if (this->TheLoop->isIncluded(SuccBB)) {

      NumSuccessorsInside++;
      InsideBlock = SuccBB;

    } else {
      OutsideBlock = SuccBB;
    }

  }

  assert(NumSuccessorsInside == 1
         && "LoopScheduler --- moveInstOutOfPrologueBasicBlock --- Constraint 2a.: Should have one successor inside the loop, one outside the loop!\n");


  /*
   * Perform move for InsideBlock
   */ 
  bool SuccessfullyMoved = this->moveInstIntoSuccessor(I, InsideBlock);
  
  
  /*
   * Perform clones for OutsideBlock
   */ 
  SuccessfullyMoved |= 
    this->cloneInstIntoSuccessor(
      I, 
      OutsideBlock,
      OriginalToClones,
      Clones
    );


  return SuccessfullyMoved;

}


bool LoopScheduler::moveInstIntoSuccessor(
  Instruction *I,
  BasicBlock *Successor
) {

  /*
   * Move the instruction to the correct insertion point
   */ 
  Instruction *InsertionPoint = Successor->getFirstNonPHI();
  I->moveBefore(InsertionPoint);


  /*
   * Resolve any successor PHINodes
   */ 
  this->resolveSuccessorPHIs(I, I, Successor);


  /*
   * Return success
   */ 
  errs() << "LoopScheduler:     Success! Moved @I to successor\n";
  return true;

}


bool LoopScheduler::cloneInstIntoSuccessor(
  Instruction *I,
  BasicBlock *Successor,
  ValueToValueMapTy &OriginalsToClones,
  std::set<Instruction *> &Clones
) {

  /*
   * Clone the instruction into the correct insertion point
   */ 
  Instruction *InsertionPoint = Successor->getFirstNonPHI(),
              *Clone = I->clone();

  Clone->insertBefore(InsertionPoint);


  /*
   * Resolve any successor PHINodes
   */ 
  this->resolveSuccessorPHIs(I, Clone, Successor);


  /*
   * Fill out the entry in the ValueToValueMap, insert 
   * into Clones set
   */ 
  OriginalsToClones[I] = Clone;
  Clones.insert(Clone);


  /*
   * Return success
   */ 
  errs() << "LoopScheduler:     Success! Cloned @I to successor\n";
  return true;

}


void LoopScheduler::remapClonedInstructions(
  ValueToValueMapTy &OriginalToClones,
  std::set<Instruction *> &Clones
) {

  std::set<Value *> ValuesToEnter;

  /*
   * Find all possible operands across the moved instructions
   * that are not already in the ValueToValueMap --- these must
   * be entered so llvm:remapInstruction can work properly --- FIX
   * 
   * FIX --- POSSIBLY REFACTOR
   */ 
  for (auto const &[Original, Clone] : OriginalToClones) {

    auto OriginalInstruction = cast<Instruction>(Original);

    for (auto Index = 0; 
         Index < OriginalInstruction->getNumOperands();
         ++Index) {

      /*
       * Record the operand to enter if it doesn't already exist
       * in the ValueToValueMap
       */ 
      Value *Operand = OriginalInstruction->getOperand(Index);

      if (OriginalToClones.find(Operand) == OriginalToClones.end()) {
        ValuesToEnter.insert(Operand);
      }
      
    }

  }


  /*
   * Make the entries
   */ 
  for (auto V : ValuesToEnter) {
    OriginalToClones[V] = V;
  }


  /*
   * For each clone, remap the operands
   */
  for (auto Clone : Clones) {
    llvm::RemapInstruction(Clone, OriginalToClones);
  }


  return;

}


void LoopScheduler::resolveSuccessorPHIs(
  Instruction * const Moved,
  Instruction * const Replacement,
  BasicBlock *SuccBB
) {

  /*
   * TOP --- Record any single incoming PHIs that need to be
   * folded because of a moved instruction (@Moved), replace all 
   * uses with the moved instruction
   * 
   * NEEDS REFACTORING
   */ 

  // errs() << "LoopScheduler:         resolveSuccessorPHIs for " << *SuccBB << "\n";

  /*
   * Sanity check --- should have already determined @SuccBB is fit
   * to resolve in another Scheduler analysis method --- should have
   * a single predecessor
   */
  BasicBlock *Pred = SuccBB->getSinglePredecessor();
  assert(Pred && "Scheduler: @SuccBB should only have a single predecessor!");


  std::set<PHINode *> PHIsToResolve;

  for (auto &PHI : SuccBB->phis()) {

    /*
     * Process the incoming value
     */ 
    Value *Incoming = PHI.getIncomingValue(0);
    Instruction *IncomingInst = dyn_cast<Instruction>(Incoming);
    if (!IncomingInst) {
      continue;
    }


    /*
     * If the incoming value == I, the PHINode should 
     * be folded, and all uses should be replaced with
     * the pushed instr
     */ 
    if (IncomingInst == Moved) {
      PHIsToResolve.insert(&PHI);
    }

  }


  for (auto PHI : PHIsToResolve) {

    /*
     * Replace all uses
     */
    PHI->replaceAllUsesWith(Replacement);


    /*
     * Fold the PHINode
     */ 
    const uint32_t IndexToRemove = 0;
    PHI->removeIncomingValue(
      IndexToRemove, 
      true /* DeletePHIIfEmpty */
    );

  }


  return;

}
