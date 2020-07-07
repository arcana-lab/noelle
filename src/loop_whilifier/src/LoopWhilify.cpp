/*
 * Copyright 2019 - 2020  Souradip Ghosh, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopWhilify.hpp"

using namespace llvm;
 
LoopWhilifier::LoopWhilifier(Noelle &noelle)
  : noelle{noelle}
  {
  return ;
}

bool LoopWhilifier::whilifyLoop (
  LoopDependenceInfo const &LDI
) {

  auto modified = false;

  /*
   * Check if the loop can be whilified
   */ 
  BasicBlock *Header = nullptr,
             *Latch = nullptr,
             *Exit = nullptr;
  
  LoopStructure *LS = LDI.getLoopStructure();

  if (!(this->canWhilify(LS, Header, Latch, Exit))) {
    return modified;
  }

  /*
   * Acquire all other necessary info to whilify
   */ 
  InductionVariableManager *IVM = LDI.getInductionVariableManager();
  InductionVariable *IV = IVM->getLoopGoverningInductionVariable(*LS);
  PHINode *GoverningPHI = IV->getLoopEntryPHI();


  /* 
   * *** MAJOR TODO *** --- Update all data structures with new 
   * instructions and blocks when transforming
   */


  while (!(this->isDoWhile(LS, Latch))) {

    /*
     * TOP LEVEL --- Insert the condition at the top of the 
     * header --- compare to the appropriate PHINode instead 
     * of the iterator instruction, delete the condition in 
     * the loop latch
     */ 

    /* 
     * Get the compare instruction that's used by the branch
     * in the latch
     */
    CmpInst *LatchCmpInst = nullptr;
    BranchInst *LatchTerm = nullptr;

    this->getLatchInfo(Latch, LatchCmpInst, LatchTerm);

    if (false
        || (!LatchCmpInst) 
        || (!LatchTerm)) {

      errs() << "LatchCmpInst and/or LatchTerm is toast!\n";
      return modified;

    }


    /*
     * Insert a new compare instruction that compare directly with
     * the governing PHINode --- this will function as the new 
     * condition
     */ 
    Instruction *HeaderInsertionPoint = Header->getFirstNonPHI();
    if (!HeaderInsertionPoint) {

      errs() << "HeaderInsertionPoint is toast!\n";
      return modified;

    }

    IRBuilder<> HeaderBuilder{HeaderInsertionPoint};
    Value *NewHeaderCmp = HeaderBuilder.CreateICmp(
      LatchCmpInst->getPredicate(), /* Original predicate */
      GoverningPHI, /* New LHS */
      LatchCmpInst->getOperand(1), /* Original RHS */
      LatchCmpInst->getName() /* Original name */
    );


    /*
     * Split the header --- contains a true edge to the block
     * that will contain the rest of the original header; has a 
     * false edge to the exit block
     */ 
    Instruction *NewHeaderCmpInst = dyn_cast<CmpInst>(NewHeaderCmp),
                *SplitPoint = (NewHeaderCmpInst) ?
                              (NewHeaderCmpInst->getNextNode()) : 
                              (HeaderInsertionPoint); // Header->getFirstNonPHI()

    BasicBlock *RestOfHeaderBlock = SplitBlock(Header, SplitPoint); 
    
    // Reset state --- Suspicious
    if (Header == Latch) { 
      Latch = RestOfHeaderBlock; 
    }


    /*
     * Repalce the default unconditional branch at the end of the 
     * new header block with the condition (described in the 
     * previous comment)
     */ 
    Instruction *NewHeaderTerm = Header->getTerminator();
    IRBuilder<> NewHeaderBranchBuilder{NewHeaderTerm};
    BranchInst *NewHeaderBranchTerm = NewHeaderBranchBuilder.CreateCondBr(
      NewHeaderCmp,
      RestOfHeaderBlock,
      Exit
    );


    // *** NOTE *** --- All removes after this line have
    // been declared as suspicious

    NewHeaderTerm->eraseFromParent(); // Removes default unconditional


    /*
     * Remove the original condition and original branch in
     * the original latch block
     */ 
    IRBuilder<> NewLatchBranchBuilder{LatchTerm};
    BranchInst *NewLatchBranchTerm = NewLatchBranchBuilder.CreateBr(Header);

    LatchTerm->eraseFromParent(); // Remove original latch terminator
    if (!(LatchCmpInst->getNumUses())) { 
      LatchCmpInst->eraseFromParent(); // Remove original compare instruction
    }


    modified |= true;

  }

  return modified;

}


/*
 * NOTE --- Based on isLoopExiting from LoopInfo.h
 */  
bool LoopWhilifier::isDoWhile(
  LoopStructure * const LS,
  BasicBlock * const Latch
) {

  bool isDoWhile = false;

  /*
   * TOP --- If any of the successors of the latch are not part 
   * of the loop --- the latch must be an intra-loop edge to an 
   * exit block
   * 
   * The loop is a do-while loop as long as the latch is loop-
   * exiting
   */  

  for (auto *SuccBB : successors(Latch)) {
    if (!(LS->isIncluded(SuccBB))) {
      isDoWhile |= true;
      return isDoWhile;
    }
  }

  return isDoWhile;

}


bool LoopWhilifier::canWhilify (
  LoopStructure * const LS,
  BasicBlock *&Header,
  BasicBlock *&Latch,
  BasicBlock *&Exit
) {

  bool canWhilify = true;

  /*
   * TOP --- Require valid header, single latch, and single exit, 
   * then check if the loop is in do-while form
   */ 


  /*
   * Acquire header
   */ 
  Header = LS->getHeader();
  canWhilify &= !!Header;


  /*
   * Acquire latch
   */ 
  auto Latches = LS->getLatches();
  if (Latches.size() != 1) {
    canWhilify &= false;
  } else {
    Latch = *(Latches.begin());
  }


  /*
   * Acquire exit
   */ 
  auto Exits = LS->getLoopExitBasicBlocks();
  if (Exits.size() != 1) {
    canWhilify &= false;
  } else {
    Exit = Exits.front();
  }


  /*
   * Check if loop is in do-while form
   */ 
  if (canWhilify) {
    canWhilify &= this->isDoWhile(LS, Latch);
  }


  return canWhilify;

}


void LoopWhilifier::getLatchInfo (
  BasicBlock * const Latch,
  CmpInst *&LatchCmpInst,
  BranchInst *&LatchTerm
) {

  /*
   * TOP --- acquire the terminator instruction in the latch, only
   * if it exists as a conditional branch; get the compare instruction 
   * associated with the conditional branch
   */ 

  LatchTerm = dyn_cast<BranchInst>(Latch->getTerminator());

  if (LatchTerm) {
    if (LatchTerm->isConditional()) {
      LatchCmpInst = dyn_cast<CmpInst>(LatchTerm->getCondition());
    }
  }

  return;

}



