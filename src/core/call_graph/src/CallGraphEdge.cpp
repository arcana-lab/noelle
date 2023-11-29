/*
 * Copyright 2019 - 2022 Simone Campanoni
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
#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/CallGraphEdge.hpp"

namespace arcana::noelle {

void CallGraphEdge::setMust(void) {
  this->isMust = true;

  return;
}

void CallGraphEdge::unsetMust(void) {
  this->isMust = false;

  return;
}

bool CallGraphEdge::isAMustCall(void) const {
  return this->isMust;
}

CallGraphFunctionNode *CallGraphEdge::getCallee(void) const {
  return this->callee;
}

CallGraphFunctionFunctionEdge::CallGraphFunctionFunctionEdge(
    CallGraphFunctionNode *caller,
    CallGraphFunctionNode *callee,
    bool isMust)
  : caller{ caller } {
  this->isMust = isMust;
  this->callee = callee;

  return;
}

CallGraphFunctionNode *CallGraphFunctionFunctionEdge::getCaller(void) const {
  return this->caller;
}

void CallGraphFunctionFunctionEdge::print(void) {
  errs() << "TODO\n";

  return;
}

void CallGraphFunctionFunctionEdge::addSubEdge(
    CallGraphInstructionFunctionEdge *subEdge) {

  /*
   * Fetch the caller.
   */
  auto instNode = subEdge->getCaller();
  auto inst = instNode->getInstruction();

  /*
   * Add the sub-edge.
   */
  this->subEdges.insert(subEdge);
  this->subEdgesMap[inst] = subEdge;

  return;
}

void CallGraphFunctionFunctionEdge::removeSubEdge(
    CallGraphInstructionFunctionEdge *subEdge) {

  /*
   * Remove the sub-edge from the set.
   */
  assert(this->subEdges.find(subEdge) != this->subEdges.end());
  this->subEdges.erase(subEdge);

  /*
   * Remove the sub-edge from the map.
   */
  auto caller = subEdge->getCaller();
  auto callInst = caller->getInstruction();
  this->subEdgesMap.erase(callInst);

  /*
   * Update the attribute of the edge.
   */
  if (subEdge->isAMustCall()) {

    /*
     * Check if there is another sub-edge that is a must edge.
     * In this way, the edge can stay as a must edge; otherwise, it cannot.
     */
    assert(this->isAMustCall());
    auto found = false;
    for (auto anotherSubEdge : this->subEdges) {
      if (anotherSubEdge->isAMustCall()) {
        found = true;
        break;
      }
    }
    if (!found) {

      /*
       * The edge doesn't have a must-sub-edge anymore.
       */
      this->unsetMust();
    }
  }

  /*
   * Destroy the sub-edge.
   */
  delete subEdge;

  return;
}

uint64_t CallGraphFunctionFunctionEdge::getNumberOfSubEdges(void) const {
  return this->subEdges.size();
}

std::unordered_set<CallGraphInstructionFunctionEdge *>
CallGraphFunctionFunctionEdge::getSubEdges(void) const {
  return this->subEdges;
}

CallGraphInstructionFunctionEdge::CallGraphInstructionFunctionEdge(
    CallGraphInstructionNode *caller,
    CallGraphFunctionNode *callee,
    bool isMust) {
  this->caller = caller;
  this->callee = callee;
  this->isMust = isMust;

  return;
}

CallGraphInstructionNode *CallGraphInstructionFunctionEdge::getCaller(
    void) const {
  return this->caller;
}

void CallGraphInstructionFunctionEdge::print(void) {
  errs() << "TODO\n";

  return;
}

CallGraphEdge::~CallGraphEdge() {
  return;
}

CallGraphInstructionFunctionEdge::~CallGraphInstructionFunctionEdge() {
  return;
}

CallGraphFunctionFunctionEdge::~CallGraphFunctionFunctionEdge() {
  return;
}

} // namespace arcana::noelle
