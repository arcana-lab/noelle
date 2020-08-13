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


bool Scheduler::shrinkLoopPrologue (
  LoopStructure * const LS,
  DomTreeSummary const &PDT,
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
 * --------------------------------- 
 * LoopSchedulerContext
 * ---------------------------------
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