/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopStructure.hpp"

using namespace llvm;

uint64_t LoopStructure::globalID = 0;

LoopStructure::LoopStructure (
  Loop *l
  ) 
  : LoopStructure(l, nullptr)
  {

  return ;
}

LoopStructure::LoopStructure (
  Loop *l,
  LoopStructure *parentLoop
  ) 
  : parent{parentLoop}
  {

  /*
   * Set the nesting level
   */
  this->depth = l->getLoopDepth();

  /*
   * Set the headers.
   */
  this->header = l->getHeader();
  this->preHeader = l->getLoopPreheader();

  /*
   * Set the basic blocks and latches of the loop.
   */
  for (auto bb : l->blocks()) {
    // NOTE: Unsure if this is program forward order
    orderedBBs.push_back(bb);
    this->bbs.insert(bb);
    if (l->isLoopLatch(bb)) {
      latchBBs.insert(bb);
    }

    for (auto& inst : *bb){

      /*
       * NOTE: Loop implementation of isLoopInvariant simply checks if the value
       * is in the loop, not if it changes between iterations.
       */
      if (l->isLoopInvariant(&inst)){
        this->invariants.insert(&inst);
      }
    }
  }

  /*
   * Set the loop exits and exit edges.
   */
  SmallVector<BasicBlock *, 10> exits;
  l->getExitBlocks(exits);
  this->exitBlocks = std::vector<BasicBlock *>(exits.begin(), exits.end());

  SmallVector<std::pair<BasicBlock *, BasicBlock *>, 10> exitEdges;
  l->getExitEdges(exitEdges);
  this->exitEdges = std::vector<std::pair<BasicBlock *, BasicBlock *>>(exitEdges.begin(), exitEdges.end());

  /*
   * Fetch the metadata.
   */
  this->addMetadata("noelle.loop_ID");
  this->addMetadata("noelle.loop_optimize");

  /*
   * Check if there is metadata for the ID.
   */
  if (this->doesHaveMetadata("noelle.loop_ID")){

    /*
     * Fetch the ID from the metadata.
     */
    auto IDString = this->getMetadata("noelle.loop_ID");
    this->ID = std::stoul(IDString);

  } else {

    /*
     * There is no metadata.
     * Hence, we assign an arbitrary ID.
     */
    this->ID = LoopStructure::globalID++;
  }

  return ;
}

BasicBlock * LoopStructure::getHeader (void) const {
  return this->header;
}
 
BasicBlock * LoopStructure::getPreHeader (void) const {
  return this->preHeader;
}
      
Instruction * LoopStructure::getEntryInstruction (void) const {
  auto header = this->getHeader();
  auto firstInst = &*header->begin();
  return firstInst;
}

uint32_t LoopStructure::getNestingLevel (void) const {
  return this->depth;
}
      
LoopStructure * LoopStructure::getParentLoop (void) const {
  return this->parent;
}
      
void LoopStructure::setParentLoop (LoopStructure *parentLoop) {
  this->parent = parentLoop;

  return ;
}
      
std::unordered_set<LoopStructure *> LoopStructure::getChildren (void) const {
  return this->children;
}

std::unordered_set<LoopStructure *> LoopStructure::getDescendants (void) const {
  std::unordered_set<LoopStructure *> descendants;
  for (auto child : this->children) {
    descendants.insert(child);
    auto childDescendants = child->getDescendants();
    descendants.insert(childDescendants.begin(), childDescendants.end());
  }

  return descendants;
}

void LoopStructure::addChild (LoopStructure *child) {
  this->children.insert(child);

  return ;
}
      
std::unordered_set<BasicBlock *> LoopStructure::getLatches (void) const {
  return this->latchBBs;
}
      
std::unordered_set<BasicBlock *> LoopStructure::getBasicBlocks (void) const {
  return this->bbs;
}
      
std::unordered_set<Instruction *> LoopStructure::getInstructions (void) const {
  std::unordered_set<Instruction *> insts{};
  for (auto bb : this->bbs){
    for (auto &inst : *bb){
      insts.insert(&inst);
    }
  }

  return insts;
}
      
uint64_t LoopStructure::getNumberOfInstructions (void) const {
  uint64_t t = 0;
  for (auto bb : this->bbs){
    t += bb->size();
  }

  return t;
}

std::vector<BasicBlock *> LoopStructure::getLoopExitBasicBlocks (void) const {
  return this->exitBlocks;
}

std::vector<std::pair<BasicBlock *, BasicBlock *>> LoopStructure::getLoopExitEdges (void) const {
  return this->exitEdges;
}

bool LoopStructure::isLoopInvariant (Value *value) const {

  /*
   * Check if value is an instruction.
   */
  if (auto inst = dyn_cast<Instruction>(value)) {

    /*
     * Check if the instruction is not included in the loop.
     */
    if (!this->isIncluded(inst->getParent())) {
      return true;
    }

    return this->isContainedInstructionLoopInvariant(inst);
  } 

  /*
   * Check if value is an argument.
   */
  if (auto arg = dyn_cast<Argument>(value)) {
    return true;
  }

  /*
   * We cannot determine whether the value is loop invariant without further analysis
   */
  return false;
}

bool LoopStructure::isContainedInstructionLoopInvariant (Instruction *inst) const {

  /*
   * Currently, we are as naive as LLVM, not including loop internal instructions
   * which derive from loop invariants as being loop invariant. We simply cache
   * loop instructions which LLVM's isLoopInvariant returns true for
   */
  return this->invariants.find(inst) != this->invariants.end();
}

bool LoopStructure::isIncluded (BasicBlock *bb) const {
  auto found = this->bbs.find(bb) != this->bbs.end();

  return found;
}
      
bool LoopStructure::isIncluded (Instruction *i) const {
  auto bb = i->getParent();
  auto contained = this->isIncluded(bb);

  return contained;
}
      
void LoopStructure::print (raw_ostream &stream) {
  stream << "Loop summary: " << this->ID << ", depth: " << depth << "\n";
  header->begin()->print(stream); stream << "\n";

  return ;
}

uint64_t LoopStructure::getID (void) const {
  return this->ID;
}
   
Function * LoopStructure::getFunction (void) const {
  auto f = this->header->getParent();
  return f;
}

bool LoopStructure::isIncludedInItsSubLoops (Instruction *inst) const {

  /*
   * Check if the instruction is part of the loop.
   */
  if (!this->isIncluded(inst)){
    return false;
  }

  /*
   * Check its children.
   */
  for (auto subLoop : this->children){

    /*
     * Check if the instruction belongs to the current child.
     */
    if (subLoop->isIncluded(inst)){
      return true;
    }

    /*
     * The instruction does not belong to the current child.
     */
  }

  return false;
}

uint32_t LoopStructure::getNumberOfSubLoops (void) const {

  /*
   * Check its children.
   */
  uint32_t subloops = 0;
  for (auto subLoop : this->children){

    /*
     * Account for the current sub-loop.
     */
    subloops++;

    /*
     * Account for the sub-loops of the current sub-loop.
     */
    subloops += subLoop->getNumberOfSubLoops();
  }

  return subloops;
}

bool LoopStructure::doesHaveMetadata (const std::string &metadataName) const {
  if (this->metadata.find(metadataName) == this->metadata.end()){
    return false;
  }

  return true;
}

std::string LoopStructure::getMetadata (const std::string &metadataName) const {

  /*
   * Check if the metadata exists.
   */
  if (!this->doesHaveMetadata(metadataName)){
    return "";
  }

  return this->metadata.at(metadataName);
}

void LoopStructure::addMetadata (const std::string &metadataName){

  /*
   * Fetch the header terminator.
   */
  auto headerTerm = this->getHeader()->getTerminator();

  /*
   * Fetch the metadata node.
   */
  auto metaNode = headerTerm->getMetadata(metadataName);
  if (!metaNode){
    return ;
  }

  /*
   * Fetch the string.
   */
  auto metaString = cast<MDString>(metaNode->getOperand(0))->getString();

  /*
   * Add the metadata.
   */
  this->metadata[metadataName] = metaString;

  return ;
}
