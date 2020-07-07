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
   * Check if the loop can be whilified, 
   */ 
  BasicBlock *Header = *Latch = *Exit = nullptr;
  if (!canWhilify(LDI, Header, Latch, Exit)) {
    return modified;
  }

  // Acquire all necessary info
  const LoopStructure *LS = LDI.getLoopStructure();

  return modified;

}


bool isDoWhile(
  LoopDependenceInfo const &LDI,
) {

  bool isDoWhile = false;

  return isDoWhile;

}


bool canWhilify (
  LoopDependenceInfo const &LDI,
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
  Header = LDI.getHeader();
  canWhilify &= Header;


  /*
   * Acquire latch
   */ 
  auto Latches = LDI.getLatches();
  if (Latches.size() != 1) {
    canWhilify &= false;
  } else {
    Latch = *(Latches.begin());
  }


  /*
   * Acquire exit
   */ 
  auto Exits = LDI.getLoopExitBasicBlocks();
  if (Exits.size() != 1) {
    canWhilify &= false;
  } else {
    Exit = Exits.front();
  }


  /*
   * Check if loop is in do-while form
   */ 
  if (canWhilify) {
    canWhilify = _isDoWhile(LDI);
  }


  return canWhilify;

}


void getLatchInfo (
  LoopDependenceInfo const &LDI,
  BasicBlock * const Latch,
  CmpInst *&LatchCmpInst,
  BranchInst *&LatchTerm
) {

  return;

}


bool isLatchLoopExiting (
  LoopDependenceInfo const &LDI,
  BasicBlock * const Latch
) {

  bool isLatchLoopExiting = false;

  return isLatchLoopExiting;

}
