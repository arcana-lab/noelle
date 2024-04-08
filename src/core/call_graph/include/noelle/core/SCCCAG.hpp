/*
 * Copyright 2019 - 2024  Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_CALL_GRAPH_SCCCAG_H_
#define NOELLE_SRC_CORE_CALL_GRAPH_SCCCAG_H_

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/CallGraph.hpp"
#include "noelle/core/CallGraphTraits.hpp"
#include "noelle/core/SCCCAGNode.hpp"
#include "noelle/core/SCCCAGEdge.hpp"

namespace arcana::noelle {
class CallGraph;

class SCCCAG {
public:
  SCCCAG(noelle::CallGraph *cg);

  SCCCAG() = delete;

  bool doesItBelongToAnSCC(Function *f);

  SCCCAGNode *getNode(CallGraphFunctionNode *n) const;

  std::set<SCCCAGNode *> getNodes(void) const;

  std::set<SCCCAGEdge *> getEdges(void) const;

  std::set<SCCCAGNode *> getNodesWithInDegree(uint64_t targetInDegree) const;

  std::set<SCCCAGNode *> getNodesWithOutDegree(uint64_t targetOutDegree) const;

  std::unordered_map<SCCCAGNode *, SCCCAGEdge *> getOutgoingEdges(
      SCCCAGNode *n) const;

  std::unordered_map<SCCCAGNode *, SCCCAGEdge *> getIncomingEdges(
      SCCCAGNode *n) const;

private:
  CallGraph *cg;
  std::unordered_map<CallGraphFunctionNode *, SCCCAGNode *> fromCGNodeToSCC;
  std::set<SCCCAGNode *> nodes;
  std::set<SCCCAGEdge *> edges;
  std::unordered_map<SCCCAGNode *,
                     std::unordered_map<SCCCAGNode *, SCCCAGEdge *>>
      outgoingEdges;
  std::unordered_map<SCCCAGNode *,
                     std::unordered_map<SCCCAGNode *, SCCCAGEdge *>>
      incomingEdges;

  void createNodes(CallGraph *cg);

  void createEdges(CallGraph *cg);

  SCCCAGEdge *newEdge(SCCCAGNode *from, SCCCAGNode *to);
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_CALL_GRAPH_SCCCAG_H_
