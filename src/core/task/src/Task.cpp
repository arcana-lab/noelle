/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Task.hpp"

using namespace llvm;

namespace llvm::noelle {
  Task::Task (
    uint32_t ID,
    FunctionType *taskSignature,
    Module &M
    )
    : ID{ID}
    {

    /*
    * Create the empty body of the task.
    */
    auto functionCallee = M.getOrInsertFunction("", taskSignature);
    this->F = cast<Function>(functionCallee.getCallee());

    /*
    * Add the entry and exit basic blocks.
    */
    auto &cxt = M.getContext();
    this->entryBlock = BasicBlock::Create(cxt, "", this->F);
    this->exitBlock = BasicBlock::Create(cxt, "", this->F);

    return ;
  }

  uint32_t Task::getID (void) const {
    return this->ID;
  }
        
  bool Task::isAnOriginalLiveIn (Value *v) const {
    if (this->liveInClones.find(v) == this->liveInClones.end()){
      return false;
    }

    return true;
  }

  Value * Task::getCloneOfOriginalLiveIn (Value *o) const {
    if (!this->isAnOriginalLiveIn(o)){
      return nullptr;
    }

    return this->liveInClones.at(o);
  }

  void Task::addLiveIn (Value *original, Value *internal) {
    this->liveInClones[original] = internal;

    return ;
  }

  std::unordered_set<Value *> Task::getOriginalLiveIns (void) const {
    std::unordered_set<Value *> s;
    for (auto p : this->liveInClones){
      s.insert(p.first);
    }

    return s;
  }

  bool Task::doesOriginalLiveOutHaveManyClones (Instruction *I) const {
    return liveOutClones.find(I) != liveOutClones.end();
  }

  std::unordered_set<Instruction *> Task::getClonesOfOriginalLiveOut (Instruction *I) const {
    if (liveOutClones.find(I) == liveOutClones.end()) {
      return {};
    }

    return liveOutClones.at(I);
  }

  void Task::addLiveOut (Instruction *original, Instruction *internal) {
    liveOutClones[original].insert(internal);
  }

  void Task::removeLiveOut (Instruction *original, Instruction *removed) {
    if (liveOutClones.find(original) == liveOutClones.end()) {
      return ;
    }

    liveOutClones[original].erase(removed);
  }

  bool Task::isAnOriginalBasicBlock (BasicBlock *o) const {
    if (this->basicBlockClones.find(o) == this->basicBlockClones.end()){
      return false;
    }

    return true;
  }

  BasicBlock * Task::getCloneOfOriginalBasicBlock (BasicBlock *o) const {
    if (!this->isAnOriginalBasicBlock(o)){
      return nullptr;
    }

    return this->basicBlockClones.at(o);
  }

  void Task::removeOriginalBasicBlock (BasicBlock *b){
    this->basicBlockClones.erase(b);

    return ;
  }

  std::unordered_set<BasicBlock *> Task::getOriginalBasicBlocks (void) const {
    std::unordered_set<BasicBlock *> s;
    for (auto p : this->basicBlockClones){
      s.insert(p.first);
    }

    return s;
  }

  void Task::addBasicBlock (BasicBlock *original, BasicBlock *internal) {
    this->basicBlockClones[original] = internal;

    return ;
  }

  BasicBlock * Task::addBasicBlockStub (BasicBlock *original){

    /*
    * Fetch the context.
    */
    auto &c = this->getLLVMContext();

    /*
    * Allocate a new basic block.
    */
    auto newBB = BasicBlock::Create(c, "", this->F);

    /*
    * Keep track of the mapping.
    */
    this->addBasicBlock(original, newBB);

    return newBB;
  }

  BasicBlock * Task::cloneAndAddBasicBlock (BasicBlock *original){

    /*
    * Create a stub.
    */
    auto cloneBB = this->addBasicBlockStub(original);

    /*
    * Copy the original instructions into the cloned basic block.
    */
    IRBuilder<> builder(cloneBB);
    for (auto &I : *original) {
      auto cloneI = builder.Insert(I.clone());
      this->instructionClones[&I] = cloneI;
    }

    return cloneBB;
  }

  Value * Task::getTaskInstanceID (void) const {
    return this->instanceIndexV;
  }

  Value * Task::getEnvironment (void) const {
    return this->envArg;
  }

  Function * Task::getTaskBody (void) const {
    return this->F;
  }

  BasicBlock * Task::getEntry (void) const {
    return this->entryBlock;
  }

  BasicBlock * Task::getExit (void) const {
    return this->exitBlock;
  }
        
  void Task::tagBasicBlockAsLastBlock (BasicBlock *b) {
    this->lastBlocks.push_back(b);

    return ;
  }

  uint32_t Task::getNumberOfLastBlocks (void) const {
    return this->lastBlocks.size();
  }
        
  BasicBlock * Task::getLastBlock (uint32_t blockID) const {
    return this->lastBlocks[blockID];
  }

  LLVMContext & Task::getLLVMContext (void) const {
    auto& c = this->F->getContext();

    return c;
  }

  Instruction * Task::getCloneOfOriginalInstruction (Instruction *o) const {
    if (!this->isAnOriginalInstruction(o)){
      return nullptr;
    }

    return this->instructionClones.at(o);
  }

  bool Task::isAnOriginalInstruction (Instruction *i) const {
    if (this->instructionClones.find(i) == this->instructionClones.end()){
      return false;
    }

    return true;
  }

  void Task::addInstruction (Instruction *original, Instruction *internal) {
    this->instructionClones[original] = internal;

    return ;
  }

  std::unordered_set<Instruction *> Task::getOriginalInstructions (void) const {
    std::unordered_set<Instruction *> s;
    for (auto p : this->instructionClones){
      s.insert(p.first);
    }

    return s;
  }

  Instruction * Task::cloneAndAddInstruction (Instruction *original){
    auto cloneI = original->clone();

    this->addInstruction(original, cloneI);

    return cloneI;
  }

  void Task::removeOriginalInstruction (Instruction *o) {
    this->instructionClones.erase(o);

    return ;
  }
}