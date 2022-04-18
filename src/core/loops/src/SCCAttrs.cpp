/*
 * Copyright 2016 - 2021  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/SCCAttrs.hpp"

namespace llvm::noelle {

SCCAttrs::SCCAttrs (
    SCC *s, 
    AccumulatorOpInfo &opInfo,
    LoopStructure *loop
  ) : 
      scc{s}
    , sccType{SCCType::SEQUENTIAL}
    , accumOpInfo{opInfo}
    , PHINodes{}
    , headerPHINodes{}
    , accumulators{}
    , controlFlowInsts{}
    , controlPairs{}
    , loopCarriedVariables{}
    , isClonable{false}
    , isSCCClonableIntoLocalMemory{false}
    , hasIV{false}
    , commutative{false}
  {

  /*
   * Collect the basic blocks of the instructions contained within SCC.
   */
  for (auto nodePair : this->scc->internalNodePairs()) {
    auto valueIncludedInSCC = nodePair.first;
    if (auto instIncludedInSCC = dyn_cast<Instruction>(valueIncludedInSCC)) {
      this->bbs.insert(instIncludedInSCC->getParent());
    }
  }

  /*
   * Collect the control flows of the SCC.
   */
  this->collectControlFlowInstructions();

  /*
   * Collect PHIs and accumulators included in the SCC.
   */
  this->collectPHIsAndAccumulators(*loop);

  return;
}

SCCAttrs::SCCType SCCAttrs::getType (void) const {
  return this->sccType;
}

void SCCAttrs::setType (SCCAttrs::SCCType t) {
  this->sccType = t;

  return ;
}

iterator_range<SCCAttrs::phi_iterator> SCCAttrs::getPHIs (void){
  return make_range(this->PHINodes.begin(), this->PHINodes.end()); 
}

iterator_range<SCCAttrs::instruction_iterator> SCCAttrs::getAccumulators (void){
  return make_range(this->accumulators.begin(), this->accumulators.end()); 
}
      
bool SCCAttrs::doesItContainThisPHI (PHINode *phi){
  return this->PHINodes.find(phi) != this->PHINodes.end();
}

bool SCCAttrs::doesItContainThisInstructionAsAccumulator (Instruction *inst){
  return this->accumulators.find(inst) != this->accumulators.end();
}
      
bool SCCAttrs::isCommutative (void) const {
  return this->commutative;
}

uint32_t SCCAttrs::numberOfPHIs (void){
  return this->PHINodes.size();
}

uint32_t SCCAttrs::numberOfAccumulators (void){
  return this->accumulators.size();
}
      
PHINode * SCCAttrs::getSinglePHI (void){
  if (this->PHINodes.size() != 1) {
    return nullptr;
  }

  auto singlePHI = *this->PHINodes.begin();
  return singlePHI;
}
      
PHINode * SCCAttrs::getSingleHeaderPHI (void){
  return this->headerPHINodes.size() != 1
    ? nullptr : *this->headerPHINodes.begin();
}

Instruction * SCCAttrs::getSingleAccumulator (void){
  if (this->accumulators.size() != 1) {
    return nullptr;
  }
  
  auto singleAccumulator = *this->accumulators.begin();
  return singleAccumulator;
}

void SCCAttrs::collectPHIsAndAccumulators (LoopStructure &LS) {

  /*
   * Iterate over elements of the SCC to collect PHIs and accumulators.
   */
  for (auto iNodePair : this->scc->internalNodePairs()) {

    /*
     * Fetch the current element of the SCC.
     */
    auto V = iNodePair.first;

    /*
     * Check if it is a PHI.
     */
    if (auto phi = dyn_cast<PHINode>(V)) {
      this->PHINodes.insert(phi);
      if (LS.getHeader() == phi->getParent()) {
        this->headerPHINodes.insert(phi);
      }
      continue;
    }

    /*
     * Check if it is an accumulator.
     */
    if (auto I = dyn_cast<Instruction>(V)) {

      /*
       * Fetch the opcode.
       */
      auto binOp = I->getOpcode();

      /*
       * Check if this is an opcode we handle.
       */
      if (accumOpInfo.accumOps.find(binOp) != accumOpInfo.accumOps.end()) {
        this->accumulators.insert(I);
        continue;
      }
    }
  }

  return ;
}

void SCCAttrs::collectControlFlowInstructions (void){

  /*
   * Collect the terminators of the SCC that are involved in dependences.
   */
  for (auto iNodePair : this->scc->internalNodePairs()) {

    /*
     * Check if there are dependences from this SCC to another.
     */
    auto sccValue = iNodePair.first;
    auto sccNode = iNodePair.second;
    if (sccNode->numOutgoingEdges() == 0) {
      continue;
    }

    /*
     * Check if the current SCC node is an instruction.
     */
    auto currentValue = sccValue;
    if (!isa<Instruction>(currentValue)){
      continue ;
    }
    auto currentInst = cast<Instruction>(currentValue);

    /*
     * Check if the instruction is a terminator.
     */
    if (!currentInst->isTerminator()){
      continue ;
    }

    /*
     * The instruction is a terminator that have a dependence that leaves its SCC.
     */
    this->controlFlowInsts.insert(currentInst);
  }

  /*
   * Collect the (condition, jump) pairs.
   */
  for (auto term : this->controlFlowInsts) {
    assert(term->isTerminator());
    if (auto br = dyn_cast<BranchInst>(term)) {
      assert(br->isConditional()
        && "BranchInst with outgoing edges in an SCC must be conditional!");
      this->controlPairs.insert(std::make_pair(br->getCondition(), br));
    }
    if (auto switchI = dyn_cast<SwitchInst>(term)) {
      this->controlPairs.insert(std::make_pair(switchI->getCondition(), switchI));
    }
  }

  return ;
}

SCC * SCCAttrs::getSCC (void){
  return this->scc;
}

const std::pair<Value *, Instruction *> * SCCAttrs::getSingleInstructionThatControlLoopExit (void){
  if (this->controlPairs.size() != 1){
    return nullptr;
  }

  auto controlPair = &*this->controlPairs.begin();

  return controlPair;
}

void SCCAttrs::setSCCToBeInductionVariable (bool hasIV){
  this->hasIV = hasIV;
  return;
}

void SCCAttrs::setSCCToBeClonable (bool isClonable){
  this->isClonable = isClonable;
  return;
}

void SCCAttrs::addLoopCarriedVariable (LoopCarriedVariable *variable) {
  loopCarriedVariables.insert(variable);
}

LoopCarriedVariable * SCCAttrs::getSingleLoopCarriedVariable (void) const {
  if (loopCarriedVariables.size() != 1) return nullptr;
  return *loopCarriedVariables.begin();
}

void SCCAttrs::setSCCToBeClonableUsingLocalMemory (void) {
  this->isSCCClonableIntoLocalMemory = true;
}

bool SCCAttrs::canBeClonedUsingLocalMemoryLocations (void) const {
  return this->isSCCClonableIntoLocalMemory;
}

void SCCAttrs::addClonableMemoryLocationsContainedInSCC (std::unordered_set<const ClonableMemoryLocation *> locations) {
  this->clonableMemoryLocations = locations;
}

std::unordered_set<AllocaInst *> SCCAttrs::getMemoryLocationsToClone (void) const {
  std::unordered_set<AllocaInst *> allocations;
  for (auto location : clonableMemoryLocations) {
    allocations.insert(location->getAllocation());
  }
  return allocations;
}

bool SCCAttrs::mustExecuteSequentially (void) const {
  return this->getType() == SCCAttrs::SCCType::SEQUENTIAL;
}

bool SCCAttrs::canExecuteReducibly (void) const {
  return this->getType() == SCCAttrs::SCCType::REDUCIBLE;
}

bool SCCAttrs::canExecuteIndependently (void) const {
  return this->getType() == SCCAttrs::SCCType::INDEPENDENT;
}

bool SCCAttrs::canBeCloned (void) const {
  return this->isClonable;
}

bool SCCAttrs::isInductionVariableSCC (void) const {
  return this->hasIV;
}

SCCAttrs::~SCCAttrs () {
  for (auto var : loopCarriedVariables) {
    delete var;
  }
}

}
