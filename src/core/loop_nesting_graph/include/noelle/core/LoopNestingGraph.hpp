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
#ifndef NOELLE_SRC_CORE_LOOP_NESTING_GRAPH_LOOPNESTINGGRAPH_H_
#define NOELLE_SRC_CORE_LOOP_NESTING_GRAPH_LOOPNESTINGGRAPH_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/LoopStructure.hpp"
#include "noelle/core/FunctionsManager.hpp"
#include "noelle/core/LoopNestingGraphNode.hpp"
#include "noelle/core/LoopNestingGraphEdge.hpp"

namespace arcana::noelle {
/*
 * Loop nesting graph.
 */
class LoopNestingGraph {
public:
  LoopNestingGraph(FunctionsManager &fncsM,
                   std::vector<LoopStructure *> const &loops);

  std::unordered_set<LoopNestingGraphLoopNode *> getLoopNodes(void) const;

  std::unordered_set<LoopNestingGraphEdge *> getEdges(void) const;

  LoopNestingGraphLoopNode *getEntryNode(void) const;

  LoopNestingGraphLoopNode *getLoopNode(LoopStructure *l) const;

  void createEdge(LoopStructure *fromNode,
                  CallBase *callInst,
                  LoopStructure *child,
                  bool isMust);

private:
  FunctionsManager &fm;
  std::unordered_map<LoopStructure *, LoopNestingGraphLoopNode *> loops;
  std::unordered_map<Instruction *, LoopNestingGraphInstructionNode *>
      instructionNodes;
  std::map<LoopNestingGraphLoopNode *, std::set<LoopNestingGraphEdge *>> edges;

  LoopNestingGraphLoopLoopEdge *fetchOrCreateEdge(
      LoopNestingGraphLoopNode *fromNode,
      CallBase *callInst,
      LoopStructure *child,
      bool isMust);
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_LOOP_NESTING_GRAPH_LOOPNESTINGGRAPH_H_
