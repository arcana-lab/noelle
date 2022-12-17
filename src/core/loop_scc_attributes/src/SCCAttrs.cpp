/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/SCCAttrs.hpp"

namespace llvm::noelle {

SCCAttrs::SCCAttrs(SCCKind K, SCC *s, LoopStructure *loop)
  : loop{ loop },
    scc{ s },
    PHINodes{},
    headerPHINodes{},
    controlFlowInsts{},
    controlPairs{},
    isClonable{ false },
    isSCCClonableIntoLocalMemory{ false },
    hasIV{ false },
    commutative{ false },
    kind{ K } {

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
   * Collect PHIs included in the SCC.
   */
  this->collectPHIs(*loop);

  return;
}

iterator_range<SCCAttrs::phi_iterator> SCCAttrs::getPHIs(void) const {
  return make_range(this->PHINodes.begin(), this->PHINodes.end());
}

bool SCCAttrs::doesItContainThisPHI(PHINode *phi) {
  return this->PHINodes.find(phi) != this->PHINodes.end();
}

bool SCCAttrs::isCommutative(void) const {
  return this->commutative;
}

uint32_t SCCAttrs::numberOfPHIs(void) {
  return this->PHINodes.size();
}

PHINode *SCCAttrs::getSinglePHI(void) {
  if (this->PHINodes.size() != 1) {
    return nullptr;
  }

  auto singlePHI = *this->PHINodes.begin();
  return singlePHI;
}

PHINode *SCCAttrs::getSingleHeaderPHI(void) {
  return this->headerPHINodes.size() != 1 ? nullptr
                                          : *this->headerPHINodes.begin();
}

void SCCAttrs::collectPHIs(LoopStructure &LS) {

  /*
   * Iterate over elements of the SCC to collect PHIs.
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
  }

  return;
}

void SCCAttrs::collectControlFlowInstructions(void) {

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
    if (!isa<Instruction>(currentValue)) {
      continue;
    }
    auto currentInst = cast<Instruction>(currentValue);

    /*
     * Check if the instruction is a terminator.
     */
    if (!currentInst->isTerminator()) {
      continue;
    }

    /*
     * The instruction is a terminator that have a dependence that leaves its
     * SCC.
     */
    this->controlFlowInsts.insert(currentInst);
  }

  /*
   * Collect the (condition, jump) pairs.
   */
  for (auto term : this->controlFlowInsts) {
    assert(term->isTerminator());
    if (auto br = dyn_cast<BranchInst>(term)) {
      assert(
          br->isConditional()
          && "BranchInst with outgoing edges in an SCC must be conditional!");
      this->controlPairs.insert(std::make_pair(br->getCondition(), br));
    }
    if (auto switchI = dyn_cast<SwitchInst>(term)) {
      this->controlPairs.insert(
          std::make_pair(switchI->getCondition(), switchI));
    }
  }

  return;
}

SCC *SCCAttrs::getSCC(void) {
  return this->scc;
}

const std::pair<Value *, Instruction *>
    *SCCAttrs::getSingleInstructionThatControlLoopExit(void) {
  if (this->controlPairs.size() != 1) {
    return nullptr;
  }

  auto controlPair = &*this->controlPairs.begin();

  return controlPair;
}

void SCCAttrs::setSCCToBeInductionVariable(bool hasIV) {
  this->hasIV = hasIV;
  return;
}

void SCCAttrs::setSCCToBeClonable(bool isClonable) {
  this->isClonable = isClonable;
  return;
}

void SCCAttrs::setSCCToBeClonableUsingLocalMemory(void) {
  this->isSCCClonableIntoLocalMemory = true;
}

bool SCCAttrs::canBeClonedUsingLocalMemoryLocations(void) const {
  return this->isSCCClonableIntoLocalMemory;
}

void SCCAttrs::addClonableMemoryLocationsContainedInSCC(
    std::unordered_set<const ClonableMemoryLocation *> locations) {
  this->clonableMemoryLocations = locations;
}

std::unordered_set<AllocaInst *> SCCAttrs::getMemoryLocationsToClone(
    void) const {
  std::unordered_set<AllocaInst *> allocations;
  for (auto location : clonableMemoryLocations) {
    allocations.insert(location->getAllocation());
  }
  return allocations;
}

bool SCCAttrs::canExecuteReducibly(void) const {
  return false;
}

bool SCCAttrs::canBeCloned(void) const {
  return this->isClonable;
}

bool SCCAttrs::isInductionVariableSCC(void) const {
  return this->hasIV;
}

SCCAttrs::SCCKind SCCAttrs::getKind(void) const {
  return this->kind;
}

SCCAttrs::~SCCAttrs() {}

} // namespace llvm::noelle
