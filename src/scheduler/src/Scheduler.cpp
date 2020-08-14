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
 
Scheduler::Scheduler() {
  return;
}


/*
 * ------------------------------------------------------------------ 
 * Drivers
 * ------------------------------------------------------------------
 */

bool Scheduler::scheduleBasicBlock(
  BasicBlock *Block,
  PDG *ThePDG,
  ScheduleOption Option
) const {
  
  /*
   * TOP --- Iteratively determine if any instructions can
   * be pushed out of the basic block by analyzing the 
   * register and memory data dependences
   * 
   * Iterate backwards through the instructions in the 
   * block
   */
  

  /*
   * Set up necessary data structures:
   * - Keeps --- Instructions to stay in the block
   * - Pushes --- Instructions to push out of the block
   * - NotProcessed --- Instructions in the queue not 
   *   yet processed in the algorithm
   * 
   * - WorkList --- Instructions to process
   * 
   * - CurrentInstruction --- For PDG iterator
   */
  std::set<Instruction *> Keeps,
                          Pushes,
                          NotProcessed;

  queue<Instruction *> WorkList;

  Instruction *CurrentInstruction = nullptr;


  /*
   * Algorithm:
   * 
   * for I in all instructions (starting at the terminator):
   *    a. if I is a PHINode or terminator, add I to Keeps 
   *       and continue 
   *    b. iterate over instructions that depend on I. if 
   *       any dependence is a member of the NotProcessed 
   *       or Keeps set or not part of the basic block, add I 
   *       to Keeps and continue. Otherwise, add I to Pushes 
   *       and continue.
   * 
   * NOTE --- there are options to increase efficiency:
   * - Chaining --- this is tricky because it's possible 
   *   to miss depedences from instructions that are skipped 
   * - More efficient to push dependence cycles within a basic 
   *   block out entirely --- current algorithm is too simple
   *   for that
   */
  
  
  /*
   * Set up worklist --- reverse instruction iteration
   */ 
  for (auto IIT = Block->rbegin(); 
       IIT != Block->rend();
       ++IIT) {
    
    Instruction &I = *IIT;

    WorkList.push(&I);
    NotProcessed.insert(&I);
    
  }


  /*
   * Set up lambda for iterating over instruction dependences
   */ 
  auto Iterator = 
    [Block, CurrentInstruction, Keeps, NotProcessed]
    (Value *DependsOn, DataDependenceType D) -> bool {

    errs() << "   D: " << *DependsOn << "\n";

    /*
     * Arguments, globals, and other values are outside
     * of the basic block already, ignore
     */
    Instruction *DependsOnInst = dyn_cast<Instruction>(DependsOn);
    if (!DependsOnInst) {
      return false;
    }


    /*
     * If an instruction depends on itself, ignore *** FIX ***
     */ 
    if (DependsOnInst == CurrentInstruction) {
      return false;
    }


    /*
     * If the instruction is not part of the block, ignore
     */
    if (DependsOnInst->getParent() != Block) {
      return false;
    }


    /*
     * If the instruction is part of the Keeps or NotProcessed
     * set, return true immediately
     */ 
    if (false
        || (Keeps.find(DependsOnInst) != Keeps.end())
        || (NotProcessed.find(DependsOnInst) != NotProcessed.end())) {
      
      return true;

    }


    return false;

  };


  /*
   * Iterate over all instructions in the block
   */
  while (!WorkList.empty()) {

    /*
     * Start processing
     */
    Instruction *I = WorkList.front();
    WorkList.pop();

    errs() << "LoopScheduler: Current I: " << *I << "\n";

    NotProcessed.erase(
      NotProcessed.find(I)
    );


    /*
     * <Part a.>
     */  
    if (false
        || isa<PHINode>(I)
        || I->isTerminator()) {

      errs() << "LoopScheduler:   Keep --- (a)\n"; 

      Keeps.insert(I);
      continue;

    }


    /*
     * <Part b.> 
     */
    auto ShouldKeep = 
      ThePDG->iterateOverDependencesFrom(I, 
        false, /* Control dependences */
        true, /* Memory dependences */
        true, /* Register dependences */
        Iterator
      );

    if (ShouldKeep) {

      errs() << "LoopScheduler:   Keep --- (b)\n";
      Keeps.insert(I);

    } else {

      errs() << "LoopScheduler:   Push\n";
      Pushes.insert(I);

    }
      
  }


  /*
   * Debugging
   */
  errs() << "LoopScheduler: Instructions to push: " 
         << Pushes.size() << "\n";

  for (auto ToPush : Pushes) {
    errs() << "   " << *ToPush << "\n";
    this->pushInstructionOut(ToPush);
  }


  return false;

}


/*
 * ------------------------------------------------------------------ 
 * Transformations
 * ------------------------------------------------------------------
 */

bool Scheduler::pushInstructionOut(
  Instruction *I
) const {

  /*
   * TOP --- Push instruction @I out of the basic block @Block
   * 
   * There are conditions when the push isn't possible even
   * though the scheduler algorithm determined @I should be
   * pushed
   */ 

  BasicBlock *Block = I->getParent();
  bool Pushed = false;

  errs() << "LoopScheduler: Attempting to push: " << *I << "\n";

  /*
   * If the terminator of the basic block is an invoke instruction,
   * keep the instruction in the block --- precaution
   * 
   * *** FIX *** 
   * 
   */
  if (isa<InvokeInst>(Block->getTerminator())) {

    errs() << "LoopScheduler: Aborting push --- terminator is InvokeInst\n";
    return Pushed;

  }


  /*
   * Loop through the successors of @Block. If any successor
   * is part of a critical edge with another basic block, then
   * keep the instruction in the block
   */
  auto Successors = std::set<BasicBlock *>();

  for (auto *SuccBB : successors(Block)) {
    
    /*
     * If there isn't a single predecessor, abort the
     * push --- there exists a critical edge with SuccBB
     * as a node in the edge
     */
    auto SinglePred = SuccBB->getSinglePredecessor();

    if (!SinglePred) {

      errs() << "LoopScheduler: Aborting push --- successor(s) part of critical edge\n";
      return Pushed;

    }

  }


  /*
   * Perform the push transformation
   */
  for (auto *SuccBB : successors(Block)) {

    errs() << "LoopScheduler: Before " << *SuccBB << "\n";

    /*
     * Inject the instruction
     */ 
    Instruction *InsertionPoint = SuccBB->getFirstNonPHI(),
                *Clone = I->clone();
    
    Clone->insertBefore(InsertionPoint);

    errs() << "PushedInstruction" << *Clone << "\n";


    /*
     * Resolve any PHINodes with single incoming values
     * where incoming == I
     */
    std::set<PHINode *> PHIsToResolve;
    for (auto &PHI : SuccBB->phis()) {
      
      /*
       * Sanity check --- should have returned already
       * if more than 1 incoming value 
       */ 
      assert(PHI.getNumIncomingValues() == 1
             && "LoopScheduler: PHI to resolve should only have 1 incoming value!");
      

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
      if (I == IncomingInst) {
        PHIsToResolve.insert(&PHI);
      }

    }


    for (auto PHI : PHIsToResolve) {

      /*
       * Replace all uses
       */
      PHI->replaceAllUsesWith(Clone);


      /*
       * Fold the PHINode
       */ 
      const uint32_t IndexToRemove = 0;
      PHI->removeIncomingValue(
        IndexToRemove, 
        true /* DeletePHIIfEmpty */
      );

    }


    errs() << "LoopScheduler: After " << *SuccBB << "\n";

  }


  /*
    * Erase the original instruction
    */ 
  I->eraseFromParent();

  errs() << "LoopScheduler: The block: " << *Block << "\n";

  abort();

  return Pushed;

}


/*
 * ------------------------------------------------------------------ 
 * Methods
 * ------------------------------------------------------------------
 */

bool Scheduler::shrinkLoopPrologue (
  LoopStructure * const LS,
  DomTreeSummary const &PDT,
  Function *F,
  PDG *ThePDG,
  LoopSchedulerContext *LSC
) const {

  /*
   * Set up scheduler context if necessary
   */
  LoopSchedulerContext *TheLSC =
    (!LSC) ? 
    new LoopSchedulerContext(LS) : 
    LSC ;

  bool UpdateContext = true; /* Have to update for functionality */


  /*
   * Get the loop prologue and body
   */
  auto ThePrologue = this->getLoopPrologue(
    LS,
    PDT,
    TheLSC,
    UpdateContext
  );

  auto TheBody = this->getLoopBody(
    LS,
    PDT,
    TheLSC,
    UpdateContext
  );


  /*
   * Dump analyzed context
   */ 
  TheLSC->Dump();


  /*
   * Dependences
   */
  auto iterF = [](Value *src, DataDependenceType d) -> bool {
    errs() << "   'builds' " << *src << "\n";
    return false;
  };

  for (auto *Block : ThePrologue) {

    for (auto &I : *Block) {
      errs() << "The inst: " << I << "\n";
      ThePDG->iterateOverDependencesFrom(&I, false, true, true, iterF);
    }

  }


  for (auto *Block : ThePrologue) {

    errs() << *Block << "\n";

    this->scheduleBasicBlock(
      Block,
      ThePDG
    );

  }



  return false;

}


std::set<BasicBlock *> Scheduler::getLoopPrologue (
  LoopStructure * const LS,
  DomTreeSummary const &PDT,
  LoopSchedulerContext *LSC,
  bool UpdateContext
) const {

  /*
   * Set up prologue structure and scheduler 
   * context if necessary
   */
  auto ThePrologue = std::set<BasicBlock *>();

  LoopSchedulerContext *TheLSC =
    (!LSC) ? 
    new LoopSchedulerContext(LS) : 
    LSC ;


  /*
   * Prologue is calculated by finding all loop blocks NOT
   * post-dominated by the latch --- calculate this set
   */
  for (auto Block : TheLSC->Blocks) {

    auto DoesPostDominate = PDT.dominates(TheLSC->OriginalLatch, Block);

    if (!DoesPostDominate) {
      ThePrologue.insert(Block);
    }

  }
  

  /*
   * Update the context if necessary
   */
  if (UpdateContext) {
    TheLSC->PrologueCalculated |= true;
    TheLSC->Prologue = ThePrologue;
  }


  return ThePrologue;

}


std::set<BasicBlock *> Scheduler::getLoopBody (
  LoopStructure * const LS,
  DomTreeSummary const &PDT,
  LoopSchedulerContext *LSC,
  bool UpdateContext,
  std::set<BasicBlock *> *PassedPrologue
) const {

  /*
   * Set up body structure and scheduler 
   * context if necessary
   */
  auto TheBody = std::set<BasicBlock *>();

  bool NewLSC = !LSC;
  LoopSchedulerContext *TheLSC =
    (NewLSC) ? 
    new LoopSchedulerContext(LS) : 
    LSC ;


  /*
   * Search for a loop prologue if it's already
   * calculated
   */

  std::set<BasicBlock *> ThePrologue;

  if (PassedPrologue) { /* From parameter */

    ThePrologue = *PassedPrologue;

  } else if (true 
             && !NewLSC
             && TheLSC->PrologueCalculated) { /* From context */

    ThePrologue = TheLSC->Prologue;

  } else { /* Newly calculated */

    ThePrologue = this->getLoopPrologue(
      LS,
      PDT,
      TheLSC,
      UpdateContext
    );

  }


  /*
   * The body is the rest of the loop blocks that
   * are NOT the prologue --- calculate via a set
   * difference
   */
  std::set_difference(
    TheLSC->Blocks.begin(), TheLSC->Blocks.end(),
    ThePrologue.begin(), ThePrologue.end(),
    std::inserter(TheBody, TheBody.begin())
  );


  /*
   * Update context if necessary
   */
  if (UpdateContext) {
    TheLSC->BodyCalculated |= true;
    TheLSC->Body = TheBody;
  }


  return TheBody;

}


/*
 * ------------------------------------------------------------------ 
 * LoopSchedulerContext
 * ------------------------------------------------------------------
 */
LoopSchedulerContext::LoopSchedulerContext(
  LoopStructure * const LS
) {

  /*
   * Save passed analysis state
   */
  this->TheLoop = LS;


  /*
   * TOP --- Acquire necessary derived state, sanity 
   * check the loop
   */

  /*
   * Acquire latch 
   */ 
  auto Latches = this->TheLoop->getLatches();

  assert(Latches.size() == 1
         && "Scheduler can't currently handle loops with multiple latches!");

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
    std::vector<std::pair<BasicBlock *, BasicBlock *>>(this->TheLoop->getLoopExitEdges());

}


void LoopSchedulerContext::Dump() const {

  errs() << "LoopSchedulerContext: Starting dump ...\n";

  /*
   * Dump the loop blocks
   */
  errs() << "LoopSchedulerContext: Blocks\n";
  
  for (auto Block : this->Blocks) {
    errs() << *Block << "\n";
  }


  /*
   * Dump the loop latch
   */
  errs() << "LoopSchedulerContext: Latch\n"
         << *(this->OriginalLatch) << "\n";


  /*
   * Dump the loop prologue if possible
   */
  errs() << "LoopSchedulerContext: Prologue\n";
  
  if (this->PrologueCalculated) {

    for (auto Block : this->Prologue) {
      errs() << *Block << "\n";
    }

  }


  /*
   * Dump the loop body if possible
   */
  errs() << "LoopSchedulerContext: Body\n";
  
  if (this->BodyCalculated) {

    for (auto Block : this->Body) {
      errs() << *Block << "\n";
    }

  }

  errs() << "LoopSchedulerContext: End dump ...\n";

  return;

}