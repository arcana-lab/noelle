/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopSummary.hpp"

using namespace llvm;

uint64_t LoopSummary::globalID = 0;

LoopSummary::LoopSummary (
  Loop *l
  ) 
  : LoopSummary(l, nullptr)
  {

  return ;
}

LoopSummary::LoopSummary (
  Loop *l,
  LoopSummary *parentLoop
  ) 
  : parent{parentLoop}
    , llvmLoop{l}
    , compileTimeKnownTripCount{false}
    , tripCount{0}
  {

  instantiateIDsAndBasicBlocks(l);

}

LoopSummary::LoopSummary (
  Loop *l,
  LoopSummary *parentLoop,
  uint64_t loopTripCount
  ) 
  : parent{parentLoop}
    , llvmLoop{l}
    , compileTimeKnownTripCount{true}
    , tripCount{loopTripCount}
  {

  instantiateIDsAndBasicBlocks(l);

}

void LoopSummary::instantiateIDsAndBasicBlocks(Loop *llvmLoop) {

  /*
   * Set the ID
   */
  this->ID = LoopSummary::globalID++;

  /*
   * Set the nesting level
   */
  this->depth = llvmLoop->getLoopDepth();

  /*
   * Set the header
   */
  this->header = llvmLoop->getHeader();
  this->preHeader = llvmLoop->getLoopPreheader();

  /*
   * Set the basic blocks and latches of the loop.
   */
  for (auto bb : llvmLoop->blocks()) {
    // NOTE: Unsure if this is program forward order
    orderedBBs.push_back(bb);
    this->bbs.insert(bb);
    if (llvmLoop->isLoopLatch(bb)) {
      latchBBs.insert(bb);
    }
  }

  /*
   * Set the loop invariant.
   */
  for (auto bb : this->bbs){
    for (auto& inst : *bb){
      if (llvmLoop->isLoopInvariant(&inst)){
        this->invariants.insert(&inst);
      }
    }
  }

  SmallVector<BasicBlock *, 10> exits;
  llvmLoop->getExitBlocks(exits);
  this->exitBlocks = std::vector<BasicBlock *>(exits.begin(), exits.end());

}

BasicBlock * LoopSummary::getHeader (void) const {
  return this->header;
}
 
BasicBlock * LoopSummary::getPreHeader (void) const {
  return this->preHeader;
}

uint32_t LoopSummary::getNestingLevel (void) const {
  return this->depth;
}
      
LoopSummary * LoopSummary::getParentLoop (void) const {
  return this->parent;
}
      
void LoopSummary::setParentLoop (LoopSummary *parentLoop) {
  this->parent = parentLoop;

  return ;
}
      
std::unordered_set<LoopSummary *> LoopSummary::getChildren (void) const {
  return this->children;
}
      
void LoopSummary::addChild (LoopSummary *child) {
  this->children.insert(child);

  return ;
}
      
std::unordered_set<BasicBlock *> LoopSummary::getLatches (void) const {
  return this->getLatches();
}
      
std::unordered_set<BasicBlock *> LoopSummary::getBasicBlocks (void) const {
  return this->bbs;
}

std::vector<BasicBlock *> LoopSummary::getLoopExitBasicBlocks (void) const {
  return this->exitBlocks;
}

bool LoopSummary::isLoopInvariant (Value *v){
  if (this->invariants.find(v) == this->invariants.end()){
    return false;
  }

  return true;
}
      
bool LoopSummary::isBasicBlockWithin (BasicBlock *bb) const {
  auto found = this->bbs.find(bb) != this->bbs.end();

  return found;
}
      
void LoopSummary::print (raw_ostream &stream) {
  stream << "Loop summary: " << this->ID << ", depth: " << depth << "\n";
  header->begin()->print(stream); stream << "\n";

  return ;
}

uint64_t LoopSummary::getID (void) const {
  return this->ID;
}

bool LoopSummary::doesHaveCompileTimeKnownTripCount (void) const {
  return this->compileTimeKnownTripCount;
}

uint64_t LoopSummary::getCompileTimeTripCount (void) const {
  return this->tripCount;
}
      
Function * LoopSummary::getFunction (void) const {
  auto f = this->header->getParent();
  return f;
}
