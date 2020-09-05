/*
 * Copyright 2019 - 2020 Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SystemHeaders.hpp"
#include "SCCCAG.hpp"

namespace llvm::noelle {

  SCCCAGNode_SCC::SCCCAGNode_SCC (std::unordered_set<CallGraphNode *> const &nodes)
    : nodes{nodes}
    {
    return ;
  }
          
  bool SCCCAGNode_SCC::isAnSCC (void) const {
    return true;
  }

  SCCCAGNode_Function::SCCCAGNode_Function (Function & F) 
    : func{F}
    {
    return ;
  }

  bool SCCCAGNode_Function::isAnSCC (void) const {
    return false;
  }

  SCCCAG::SCCCAG (CallGraph *cg){

    /*
     * Iterate over all function nodes.
     *
     * NOTE: The use of a call graph wrapper is because additional APIs are needed
     * that don't belong on CallGraph for the use of GraphTraits and scc_iterator
     */
    std::unordered_set<CallGraphNode *> visited;
    CallGraphWrapper cgWrapper(cg);
    for (auto nodeWrapperIter = cgWrapper.nodes_begin(); nodeWrapperIter != cgWrapper.nodes_end(); ++nodeWrapperIter) {

      /*
       * For the next unvisited call graph node, set the entry to the call graph
       * as that node so that scc_iterator can go from there in search for the next SCC
       */
      auto nodeWrapper = *nodeWrapperIter;
      auto functionNode = nodeWrapper->wrappedNode;
      if (visited.find(functionNode) != visited.end()) continue;
      cgWrapper.entryNode = nodeWrapper;

      for (auto cgI = scc_begin(&cgWrapper); cgI != scc_end(&cgWrapper); ++cgI) {

        /*
         * Collect the nodes identified as an SCC. Ensure they haven't been visited before
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
        std::unordered_set<CallGraphNode *> cgNodes{};
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
        auto singleCGNode = static_cast<CallGraphFunctionNode *>(*cgNodes.begin());
        auto thisIsAnSCC = false;
        if (cgNodes.size() > 1) {
          thisIsAnSCC = true;

        } else {
          for (auto edge : singleCGNode->getOutgoingEdges()){
            if (edge->getCallee() == singleCGNode){
              thisIsAnSCC = true;
              break ;
            }
          }
        }

        /*
         * Create the correct node and insert it into the SCCCAG.
         * Possible nodes are an SCC or a single Function.
         */
        if (!thisIsAnSCC){
          auto sccNode = new SCCCAGNode_Function(*singleCGNode->getFunction());
          this->nodes[singleCGNode] = sccNode;
          continue;
        }
        auto sccNode = new SCCCAGNode_SCC(cgNodes);
        for (auto node : cgNodes) {
          this->nodes[node] = sccNode;
        }
      }
    }

    return ;
  }

  SCCCAGNode * SCCCAG::getNode (CallGraphNode *n) const {
    if (this->nodes.find(n) == this->nodes.end()){
      return nullptr;
    }

    auto node = this->nodes.at(n);
    return node;
  }

}
