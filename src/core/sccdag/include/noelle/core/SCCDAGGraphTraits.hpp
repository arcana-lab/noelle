/*
 * Copyright 2016 - 2023  Angelo Matni, Simone Campanoni
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
#pragma once

#include "noelle/core/DGGraphTraits.hpp"
#include "noelle/core/SCCDAG.hpp"

namespace llvm::noelle {

template <class GraphType, class NodeType>
struct ElementTraits<GraphType, NodeType, SCC>
  : public ElementTraitsBase<GraphType, NodeType, SCC> {
  using NodeRef = NodeType *;
  using nodes_iterator = typename std::vector<NodeRef>::iterator;

  explicit ElementTraits(bool isSimple = false)
    : ElementTraitsBase<GraphType, NodeType, SCC>(isSimple) {}

  static std::string getNodeLabel(NodeRef nodeWrapper, GraphType *entry) {
    std::string nodeStr;
    raw_string_ostream ros(nodeStr);
    for (auto nodePair :
         nodeWrapper->wrappedNode->getT()->internalNodePairs()) {
      nodePair.first->print(ros);
      ros << "\n";
    }
    return ros.str();
  }

  static std::string getEdgeSourceLabel(NodeRef nodeWrapper,
                                        nodes_iterator nodeIter) {
    std::string edgeStr;
    raw_string_ostream ros(edgeStr);
    auto edge = nodeWrapper->outgoingEdgeInstances
                    [nodeIter - nodeWrapper->outgoingNodeInstances.begin()];
    for (DGEdge<Value, Value> *edge : edge->getSubEdges()) {
      printValueStr(edge->getSrc(), ros);
      printValueStr(edge->getDst(), ros << " -> ");
      ros << " ; ";
    }
    return ros.str();
  }

  static void printValueStr(Value *value, raw_ostream &ros) {
    if (auto brI = dyn_cast<BranchInst>(value)) {
      if (brI->isUnconditional()) {
        value->print(ros);
      } else {
        ros << "br ";
        printValueStr(brI->getCondition(), ros);
      }
    } else {
      value->printAsOperand(ros);
    }
  }
};

} // namespace llvm::noelle

namespace llvm {

template <>
struct DOTGraphTraits<DGGraphWrapper<SCC, Value> *>
  : public ElementTraits<DGGraphWrapper<SCC, Value>,
                         DGNodeWrapper<Value>,
                         Value> {
  DOTGraphTraits(bool isSimple = false)
    : ElementTraits<DGGraphWrapper<SCC, Value>, DGNodeWrapper<Value>, Value>(
        isSimple) {}
};
template <>
struct GraphTraits<DGGraphWrapper<SCC, Value> *>
  : public GraphTraitsBase<DGGraphWrapper<SCC, Value>,
                           DGNodeWrapper<Value>,
                           Value> {};

template <>
struct DOTGraphTraits<DGGraphWrapper<SCCDAG, SCC> *>
  : public ElementTraits<DGGraphWrapper<SCCDAG, SCC>, DGNodeWrapper<SCC>, SCC> {
  DOTGraphTraits(bool isSimple = false)
    : ElementTraits<DGGraphWrapper<SCCDAG, SCC>, DGNodeWrapper<SCC>, SCC>(
        isSimple) {}
};
template <>
struct GraphTraits<DGGraphWrapper<SCCDAG, SCC> *>
  : public GraphTraitsBase<DGGraphWrapper<SCCDAG, SCC>,
                           DGNodeWrapper<SCC>,
                           SCC> {};

} // namespace llvm
