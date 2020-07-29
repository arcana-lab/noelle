/*
 * Copyright 2019 - 2020 Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "CallGraphTraits.hpp"

using namespace llvm;
using namespace noelle;

CallGraphWrapper::CallGraphWrapper (CallGraph *graph)
  : wrappedGraph{graph}, entryNode{nullptr} {

  std::unordered_map<CallGraphNode *, CallGraphNodeWrapper *> nodeToWrapperMap;
  for (auto node : graph->getFunctionNodes()) {
    auto wrappedNode = new CallGraphNodeWrapper(node);
    this->nodes.insert(wrappedNode);
    nodeToWrapperMap.insert(std::make_pair(node, wrappedNode));
  }

  auto unwrappedEntryNode = graph->getEntryNode();
  if (nodeToWrapperMap.find(unwrappedEntryNode) != nodeToWrapperMap.end()) {
    this->entryNode = nodeToWrapperMap.at(unwrappedEntryNode);
  }

  for (auto caller : graph->getFunctionNodes()) {
    auto callerWrapper = nodeToWrapperMap.at(caller);
    for (auto edge : caller->getOutgoingEdges()) {
      auto callee = edge->getCallee();
      auto calleeWrapper = nodeToWrapperMap.at(callee);
      callerWrapper->outgoingNodeInstances.push_back(calleeWrapper);
    }
  }
}

CallGraphWrapper::~CallGraphWrapper () {
  for (auto node : this->nodes) delete node;
  this->nodes.clear();
}