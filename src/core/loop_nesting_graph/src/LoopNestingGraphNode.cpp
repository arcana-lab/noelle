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
#include "noelle/core/LoopStructure.hpp"
#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/LoopNestingGraphNode.hpp"
#include "noelle/core/LoopNestingGraphEdge.hpp"

namespace arcana::noelle {

LoopNestingGraphLoopNode::LoopNestingGraphLoopNode(LoopStructure *loop)
  : loop{ loop } {

  return;
}

LoopStructure *LoopNestingGraphLoopNode::getLoop(void) const {
  return this->loop;
}

void LoopNestingGraphLoopNode::addOutgoingEdge(
    LoopNestingGraphLoopLoopEdge *edge) {
  assert(edge->getParent() == this);

  /*
   * Add the edge.
   */
  this->outgoingEdges.insert(edge);

  auto calleeNode = edge->getParent();
  this->outgoingEdgesMap[calleeNode] = edge;

  return;
}

void LoopNestingGraphLoopNode::addIncomingEdge(
    LoopNestingGraphLoopLoopEdge *edge) {
  assert(edge->getChild() == this);

  /*
   * Add the edge.
   */
  this->incomingEdges.insert(edge);

  auto callerNode = edge->getChild();
  this->incomingEdgesMap[callerNode] = edge;

  return;
}

std::unordered_set<LoopNestingGraphLoopLoopEdge *> LoopNestingGraphLoopNode::
    getIncomingEdges(void) const {
  return this->incomingEdges;
}

std::unordered_set<LoopNestingGraphLoopLoopEdge *> LoopNestingGraphLoopNode::
    getOutgoingEdges(void) const {
  return this->outgoingEdges;
}

std::unordered_set<LoopNestingGraphLoopLoopEdge *> LoopNestingGraphLoopNode::
    getEdges(void) const {
  std::unordered_set<LoopNestingGraphLoopLoopEdge *> edges{
    this->incomingEdges
  };
  edges.insert(this->outgoingEdges.begin(), this->outgoingEdges.end());

  return edges;
}

LoopNestingGraphLoopLoopEdge *LoopNestingGraphLoopNode::getNestingEdgeTo(
    LoopNestingGraphLoopNode *callee) const {
  if (this->outgoingEdgesMap.find(callee) == this->outgoingEdgesMap.end()) {
    return nullptr;
  }
  auto edge = this->outgoingEdgesMap.at(callee);

  return edge;
}

LoopNestingGraphLoopLoopEdge *LoopNestingGraphLoopNode::getNestingEdgeFrom(
    LoopNestingGraphLoopNode *caller) const {
  if (this->incomingEdgesMap.find(caller) == this->incomingEdgesMap.end()) {
    return nullptr;
  }
  auto edge = this->incomingEdgesMap.at(caller);

  return edge;
}

void LoopNestingGraphLoopNode::print(void) {
  errs() << this->loop->getFunction()->getName()
         << "::" << this->loop->getHeader()->getName() << "\n";

  return;
}

LoopNestingGraphInstructionNode::LoopNestingGraphInstructionNode(Instruction *i)
  : i{ i } {
  return;
}

Instruction *LoopNestingGraphInstructionNode::getInstruction(void) const {
  return i;
}

void LoopNestingGraphInstructionNode::print(void) {
  errs() << *this->i << "\n";

  return;
}

} // namespace arcana::noelle
