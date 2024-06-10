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
#ifndef NOELLE_SRC_CORE_LOOP_NESTING_GRAPH_LOOPNESTINGGRAPHNODE_H_
#define NOELLE_SRC_CORE_LOOP_NESTING_GRAPH_LOOPNESTINGGRAPHNODE_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "noelle/core/LoopStructure.hpp"

namespace arcana::noelle {
class LoopNestingGraphLoopLoopEdge;

class LoopNestingGraphNode {
public:
  LoopNestingGraphNode() = default;

  virtual void print(void) = 0;
  virtual ~LoopNestingGraphNode() = default;
};

class LoopNestingGraphInstructionNode : public LoopNestingGraphNode {
public:
  LoopNestingGraphInstructionNode(Instruction *i);

  Instruction *getInstruction(void) const;

  void print(void) override;

private:
  Instruction *i;
};

class LoopNestingGraphLoopNode : public LoopNestingGraphNode {
public:
  LoopNestingGraphLoopNode(LoopStructure *l);

  LoopStructure *getLoop(void) const;

  LoopNestingGraphLoopLoopEdge *getNestingEdgeTo(
      LoopNestingGraphLoopNode *child) const;

  LoopNestingGraphLoopLoopEdge *getNestingEdgeFrom(
      LoopNestingGraphLoopNode *parent) const;

  void addOutgoingEdge(LoopNestingGraphLoopLoopEdge *edge);

  void addIncomingEdge(LoopNestingGraphLoopLoopEdge *edge);

  std::unordered_set<LoopNestingGraphLoopLoopEdge *> getIncomingEdges(
      void) const;

  std::unordered_set<LoopNestingGraphLoopLoopEdge *> getOutgoingEdges(
      void) const;

  std::unordered_set<LoopNestingGraphLoopLoopEdge *> getEdges(void) const;

  void print(void) override;

private:
  LoopStructure *loop;
  std::unordered_set<LoopNestingGraphLoopLoopEdge *> outgoingEdges;
  std::unordered_set<LoopNestingGraphLoopLoopEdge *> incomingEdges;
  std::unordered_map<LoopNestingGraphLoopNode *, LoopNestingGraphLoopLoopEdge *>
      outgoingEdgesMap;
  std::unordered_map<LoopNestingGraphLoopNode *, LoopNestingGraphLoopLoopEdge *>
      incomingEdgesMap;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_LOOP_NESTING_GRAPH_LOOPNESTINGGRAPHNODE_H_
