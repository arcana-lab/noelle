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
    isClonable{ false },
    commutative{ false },
    kind{ K } {

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

bool SCCAttrs::isCommutative(void) const {
  return this->commutative;
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
  }

  return;
}

SCC *SCCAttrs::getSCC(void) {
  return this->scc;
}

void SCCAttrs::setSCCToBeClonable(bool isClonable) {
  this->isClonable = isClonable;
  return;
}

void SCCAttrs::addClonableMemoryLocationsContainedInSCC(
    std::set<ClonableMemoryLocation *> locations) {
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

bool SCCAttrs::canBeCloned(void) const {
  return this->isClonable;
}

SCCAttrs::SCCKind SCCAttrs::getKind(void) const {
  return this->kind;
}

SCCAttrs::~SCCAttrs() {}

} // namespace llvm::noelle
