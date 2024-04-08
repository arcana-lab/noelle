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
#include "noelle/core/SCCCAG.hpp"

namespace arcana::noelle {

SCCCAG::SCCCAG(CallGraph *cg) {

  /*
   * Create nodes.
   */
  this->createNodes(cg);

  /*
   * Create edges.
   */
  this->createEdges(cg);

  return;
}

void SCCCAG::createNodes(CallGraph *cg) {

  /*
   * Iterate over all function nodes.
   *
   * NOTE: The use of a call graph wrapper is because additional APIs are needed
   * that don't belong on CallGraph for the use of GraphTraits and scc_iterator
   */
  std::unordered_set<CallGraphFunctionNode *> visited;
  CallGraphWrapper cgWrapper(cg);
  for (auto nodeWrapperIter = cgWrapper.nodes_begin();
       nodeWrapperIter != cgWrapper.nodes_end();
       ++nodeWrapperIter) {

    /*
     * For the next unvisited call graph node, set the entry to the call graph
     * as that node so that scc_iterator can go from there in search for the
     * next SCC
     */
    auto nodeWrapper = *nodeWrapperIter;
    auto functionNode = nodeWrapper->wrappedNode;
    if (visited.find(functionNode) != visited.end()) {
      continue;
    }
    cgWrapper.entryNode = nodeWrapper;

    for (auto cgI = scc_begin(&cgWrapper); cgI != scc_end(&cgWrapper); ++cgI) {

      /*
       * Collect the nodes identified as an SCC. Ensure they haven't been
       * visited before
       */
      const std::vector<CallGraphNodeWrapper *> &wrappedCGNodes = *cgI;
      auto firstNodeWrapper = *wrappedCGNodes.begin();
      auto firstNode = firstNodeWrapper->wrappedNode;
      if (visited.find(firstNode) != visited.end()) {
        continue;
      }

      /*
       * Unwrap the nodes
       */
      std::unordered_set<CallGraphFunctionNode *> cgNodes{};
      for (auto wrappedCGNode : wrappedCGNodes) {
        cgNodes.insert(wrappedCGNode->wrappedNode);
      }

      /*
       * Track visited CG nodes.
       * Create the SCC node
       */
      visited.insert(cgNodes.begin(), cgNodes.end());

      /*
       * Check if the current node is an SCC.
       */
      auto singleCGNode =
          static_cast<CallGraphFunctionNode *>(*cgNodes.begin());
      auto thisIsAnSCC = false;
      if (cgNodes.size() > 1) {
        thisIsAnSCC = true;

      } else {
        for (auto edge : cg->getOutgoingEdges(singleCGNode)) {
          if (edge->getCallee() == singleCGNode) {
            thisIsAnSCC = true;
            break;
          }
        }
      }

      /*
       * Create the correct node and insert it into the SCCCAG.
       * Possible nodes are an SCC or a single Function.
       */
      if (!thisIsAnSCC) {
        auto sccNode = new SCCCAGNode_Function(singleCGNode);
        this->fromCGNodeToSCC[singleCGNode] = sccNode;
        this->nodes.insert(sccNode);
        continue;
      }
      auto sccNode = new SCCCAGNode_SCC(cgNodes);
      for (auto node : cgNodes) {
        this->fromCGNodeToSCC[node] = sccNode;
        this->nodes.insert(sccNode);
      }
    }
  }

  return;
}

SCCCAGNode *SCCCAG::getNode(CallGraphFunctionNode *n) const {
  if (this->fromCGNodeToSCC.find(n) == this->fromCGNodeToSCC.end()) {
    return nullptr;
  }

  auto node = this->fromCGNodeToSCC.at(n);
  return node;
}

void SCCCAG::createEdges(CallGraph *cg) {

  /*
   * Check every outgoing edges for every call-graph node within every SCC we
   * identified.
   */
  for (auto sccNode : this->nodes) {

    /*
     * Fetch all outgoing edges of the current SCCCAG node.
     */
    auto &sccNodeOutEdges = this->outgoingEdges[sccNode];

    /*
     * Add all edges.
     *
     * Case 0: The node is not an SCC.
     */
    if (!sccNode->isAnSCC()) {
      auto sccFuncNode = static_cast<SCCCAGNode_Function *>(sccNode);
      auto cgFuncNode = sccFuncNode->getNode();

      /*
       * Iterate over all outgoing edges.
       */
      for (auto outgoingEdge : cg->getOutgoingEdges(cgFuncNode)) {

        /*
         * Get the destination of the edge.
         */
        auto dstCGNode = outgoingEdge->getCallee();
        auto dstSCCNode = this->fromCGNodeToSCC.at(dstCGNode);
        auto &dstSCCNodeInEdges = this->incomingEdges[dstSCCNode];

        /*
         * Add the edge (@sccFuncNode, @dstSCCNode)
         */
        sccNodeOutEdges.insert(dstSCCNode);
        dstSCCNodeInEdges.insert(sccNode);
      }

      continue;
    }

    /*
     * Case 1: the node is an SCC.
     */
    auto sccSCCNode = static_cast<SCCCAGNode_SCC *>(sccNode);
    for (auto cgFuncNode : sccSCCNode->getInternalNodes()) {

      /*
       * Iterate over all outgoing edges.
       */
      for (auto outgoingEdge : cg->getOutgoingEdges(cgFuncNode)) {

        /*
         * Get the destination of the edge.
         */
        auto dstCGNode = outgoingEdge->getCallee();
        auto dstSCCNode = this->fromCGNodeToSCC.at(dstCGNode);
        if (dstSCCNode == sccNode) {

          /*
           * This is an edge within the SCC.
           */
          continue;
        }

        /*
         * We found an edge from an internal node of @sccNode to another node of
         * SCCCAG.
         */
        auto &dstSCCNodeInEdges = this->incomingEdges[dstSCCNode];

        /*
         * Add the edge (@sccFuncNode, @dstSCCNode)
         */
        sccNodeOutEdges.insert(dstSCCNode);
        dstSCCNodeInEdges.insert(sccNode);
      }
    }
  }

  return;
}

std::set<SCCCAGNode *> SCCCAG::getNodes(void) const {
  return this->nodes;
}

std::set<SCCCAGNode *> SCCCAG::getNodesWithInDegree(
    uint64_t targetInDegree) const {
  std::set<SCCCAGNode *> selectedNodes;

  for (auto node : this->nodes) {
    auto inEdges = this->getIncomingEdges(node);
    if (inEdges.size() == targetInDegree) {
      selectedNodes.insert(node);
    }
  }

  return selectedNodes;
}

std::set<SCCCAGNode *> SCCCAG::getNodesWithOutDegree(
    uint64_t targetOutDegree) const {
  std::set<SCCCAGNode *> selectedNodes;

  for (auto node : this->nodes) {
    auto outEdges = this->getOutgoingEdges(node);
    if (outEdges.size() == targetOutDegree) {
      selectedNodes.insert(node);
    }
  }

  return selectedNodes;
}

std::set<SCCCAGNode *> SCCCAG::getOutgoingEdges(SCCCAGNode *n) const {
  if (this->outgoingEdges.find(n) == this->outgoingEdges.end()) {
    return {};
  }
  auto &outEdges = this->outgoingEdges.at(n);

  return outEdges;
}

std::set<SCCCAGNode *> SCCCAG::getIncomingEdges(SCCCAGNode *n) const {
  if (this->incomingEdges.find(n) == this->incomingEdges.end()) {
    return {};
  }
  auto &inEdges = this->incomingEdges.at(n);

  return inEdges;
}

} // namespace arcana::noelle
