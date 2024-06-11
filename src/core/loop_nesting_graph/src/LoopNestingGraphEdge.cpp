/*
 * Copyright 2019 - 2020 Simone Campanoni
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
#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/LoopNestingGraphEdge.hpp"

namespace arcana::noelle {

void LoopNestingGraphEdge::setMust(void) {
  this->isMust = true;

  return;
}

void LoopNestingGraphEdge::unsetMust(void) {
  this->isMust = false;

  return;
}

bool LoopNestingGraphEdge::isAMustEdge(void) const {
  return this->isMust;
}

LoopNestingGraphLoopNode *LoopNestingGraphEdge::getChild(void) const {
  return this->child;
}

LoopNestingGraphLoopLoopEdge::LoopNestingGraphLoopLoopEdge(
    LoopNestingGraphLoopNode *parent,
    LoopNestingGraphLoopNode *child,
    bool isMust)
  : parent{ parent } {
  this->isMust = isMust;
  this->child = child;

  return;
}

LoopNestingGraphLoopNode *LoopNestingGraphLoopLoopEdge::getParent(void) const {
  return this->parent;
}

void LoopNestingGraphLoopLoopEdge::print(void) {
  this->parent->print();
  errs() << " -> ";
  this->child->print();
  errs() << "\n";

  return;
}

void LoopNestingGraphLoopLoopEdge::addSubEdge(
    LoopNestingGraphInstructionLoopEdge *subEdge) {

  /*
   * Fetch the caller.
   */
  auto instNode = subEdge->getCaller();
  if (instNode == nullptr) {
    return;
  }
  auto inst = instNode->getInstruction();

  /*
   * Add the sub-edge.
   */
  this->subEdges.insert(subEdge);
  this->subEdgesMap[inst] = subEdge;

  return;
}

std::unordered_set<LoopNestingGraphInstructionLoopEdge *>
LoopNestingGraphLoopLoopEdge::getSubEdges(void) const {
  return this->subEdges;
}

LoopNestingGraphInstructionLoopEdge::LoopNestingGraphInstructionLoopEdge(
    LoopNestingGraphInstructionNode *caller,
    LoopNestingGraphLoopNode *child,
    bool isMust) {
  this->caller = caller;
  this->child = child;
  this->isMust = isMust;

  return;
}

LoopNestingGraphInstructionNode *LoopNestingGraphInstructionLoopEdge::getCaller(
    void) const {
  return this->caller;
}

void LoopNestingGraphInstructionLoopEdge::print(void) {
  errs() << "TODO\n";

  return;
}

} // namespace arcana::noelle
