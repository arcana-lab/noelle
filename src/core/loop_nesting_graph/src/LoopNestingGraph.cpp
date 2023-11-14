/*
 * Copyright 2019 - 2023 Simone Campanoni
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
#include "noelle/core/LoopNestingGraph.hpp"

namespace llvm::noelle {

LoopNestingGraph::LoopNestingGraph(FunctionsManager &fncsM,
                                   std::vector<LoopStructure *> const &loops)
  : fm{ fncsM } {

  for (auto l : loops) {
    auto node = new LoopNestingGraphLoopNode(l);
    this->loops[l] = node;
  }
  return;
}

std::unordered_set<LoopNestingGraphLoopNode *> LoopNestingGraph::getLoopNodes(
    void) const {
  std::unordered_set<LoopNestingGraphLoopNode *> s;

  for (auto pair : this->loops) {
    s.insert(pair.second);
  }

  return s;
}

std::unordered_set<LoopNestingGraphEdge *> LoopNestingGraph::getEdges(
    void) const {
  std::unordered_set<LoopNestingGraphEdge *> e;
  for (auto p : this->edges) {
    for (auto edge : p.second) {
      e.insert(edge);
    }
  }

  return e;
}

LoopNestingGraphLoopNode *LoopNestingGraph::getEntryNode(void) const {

  /*
   * Fetch the entry function.
   */
  auto mainF = this->fm.getEntryFunction();
  assert(mainF != nullptr);

  /*
   * Fetch a loop of the entry function that has no incoming edges.
   */
  for (auto p : this->loops) {
    auto l = p.first;
    auto n = p.second;

    /*
     * Check if the current loop is within the entry function.
     */
    if (l->getFunction() != mainF) {
      continue;
    }

    /*
     * Check if the current loop has no incoming edges.
     */
    auto ins = n->getIncomingEdges();
    if (ins.size() == 0) {
      return n;
    }
  }

  return nullptr;
}

LoopNestingGraphLoopNode *LoopNestingGraph::getLoopNode(
    LoopStructure *loop) const {
  if (this->loops.find(loop) == this->loops.end()) {
    return nullptr;
  }
  auto n = this->loops.at(loop);

  return n;
}

void LoopNestingGraph::createEdge(LoopStructure *parent,
                                  CallBase *callInst,
                                  LoopStructure *child,
                                  bool isMust) {
  if (this->loops.find(parent) == this->loops.end()) {
    errs() << "unrecognized loop" << parent->getFunction()->getName()
           << parent->getHeader()->getName() << "\n";
    assert(false && "unrecognized loop");
  }
  auto fromNode = this->loops.at(parent);
  fetchOrCreateEdge(fromNode, callInst, child, isMust);
}

LoopNestingGraphLoopLoopEdge *LoopNestingGraph::fetchOrCreateEdge(
    LoopNestingGraphLoopNode *fromNode,
    CallBase *callInst,
    LoopStructure *child,
    bool isMust) {

  /*
   * Fetch the callee node.
   */
  assert(this->loops.find(child) != this->loops.end());
  auto toNode = this->loops.at(child);

  /*
   * Create the sub-edge.
   */
  llvm::noelle::LoopNestingGraphInstructionNode *instNode = nullptr;

  // subedge can have a null instNode if it's not a call edge
  if (callInst != nullptr) {
    auto instNode = this->instructionNodes[callInst];
    if (instNode == nullptr) {
      instNode = new LoopNestingGraphInstructionNode(callInst);
      this->instructionNodes[callInst] = instNode;
    }
  }
  auto subEdge =
      new LoopNestingGraphInstructionLoopEdge(instNode, toNode, isMust);

  /*
   * Check if the edge already exists.
   */
  auto existingEdge = fromNode->getNestingEdgeTo(toNode);
  if (existingEdge == nullptr) {

    /*
     * The edge from @fromNode to @toNode doesn't exist yet.
     *
     * Create a new edge.
     */
    auto newEdge = new LoopNestingGraphLoopLoopEdge(fromNode, toNode, isMust);
    this->edges[fromNode].insert(newEdge);

    /*
     * Add the new edge.
     */
    fromNode->addOutgoingEdge(newEdge);
    toNode->addIncomingEdge(newEdge);

    /*
     * Add the sub-edge.
     */
    newEdge->addSubEdge(subEdge);

    return newEdge;
  }

  /*
   * The edge from @fromNode to @toNode already exists.
   *
   * Check if we need to change its flag to must.
   */
  if (isMust) {

    /*
     * Change the flag to must.
     */
    existingEdge->setMust();
  }

  /*
   * Add the sub-edge.
   */
  existingEdge->addSubEdge(subEdge);

  return existingEdge;
}

} // namespace llvm::noelle
