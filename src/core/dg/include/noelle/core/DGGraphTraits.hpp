/*
 * Copyright 2016 - 2024  Angelo Matni, Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_DG_DGGRAPHTRAITS_H_
#define NOELLE_SRC_CORE_DG_DGGRAPHTRAITS_H_
#pragma once

#include "noelle/core/DGBase.hpp"
#include "noelle/core/PDG.hpp"
#include "noelle/core/SubCFGs.hpp"

namespace arcana::noelle {

template <class T>
class DGElementWrapper {
public:
  DGElementWrapper(T elem) : element{ elem } {};

  void print(llvm::raw_ostream &ros) {
    ros << element;
  }

private:
  T element;
};
using DGString = DGElementWrapper<std::string>;

template <class T>
class DGNodeWrapper;

template <class DG, class T>
class DGGraphWrapper {
public:
  DGGraphWrapper(DG *graph) : wrappedGraph{ graph }, entryNode{ nullptr } {
    std::unordered_map<DGNode<T> *, DGNodeWrapper<T> *> nodeToWrapperMap;
    for (auto node : graph->getNodes()) {
      auto wrappedNode = new DGNodeWrapper<T>(node);
      this->nodes.insert(wrappedNode);
      nodeToWrapperMap.insert(std::make_pair(node, wrappedNode));
    }

    auto unwrappedEntryNode = graph->getEntryNode();
    if (unwrappedEntryNode
        && nodeToWrapperMap.find(unwrappedEntryNode)
               != nodeToWrapperMap.end()) {
      this->entryNode = nodeToWrapperMap.at(unwrappedEntryNode);
    }

    for (auto node : this->nodes) {
      auto wrapped = node->wrappedNode;
      std::set<DGEdge<T, T> *> allOutgoingEdges{
        wrapped->begin_outgoing_edges(),
        wrapped->end_outgoing_edges()
      };
      for (auto edge : allOutgoingEdges) {
        auto unwrappedOtherNode = edge->getDstNode();
        if (nodeToWrapperMap.find(unwrappedOtherNode)
            != nodeToWrapperMap.end()) {
          auto wrappedOtherNode = nodeToWrapperMap.at(unwrappedOtherNode);
          node->outgoingNodeInstances.push_back(wrappedOtherNode);
          node->outgoingEdgeInstances.push_back(edge);
        }
      }
    }
  }

  ~DGGraphWrapper() {
    for (auto node : this->nodes)
      delete node;
    this->nodes.clear();
  }

  using NodeRef = DGNodeWrapper<T> *;
  using ChildIteratorType = typename std::vector<NodeRef>::iterator;
  using nodes_iterator = typename std::unordered_set<NodeRef>::iterator;

  nodes_iterator nodes_begin() {
    return nodes.begin();
  }

  nodes_iterator nodes_end() {
    return nodes.end();
  }

  DG *wrappedGraph;
  NodeRef entryNode;
  std::unordered_set<NodeRef> nodes;
};

template <class T>
class DGNodeWrapper {
public:
  DGNodeWrapper(DGNode<T> *node) : wrappedNode{ node } {}

  void print(llvm::raw_ostream &ros) {
    wrappedNode->print(ros);
  }

  using NodeRef = DGNodeWrapper<T> *;
  using ChildIteratorType = typename std::vector<NodeRef>::iterator;

  ChildIteratorType child_begin() {
    return outgoingNodeInstances.begin();
  }

  ChildIteratorType child_end() {
    return outgoingNodeInstances.end();
  }

  DGNode<T> *wrappedNode;
  std::vector<NodeRef> outgoingNodeInstances;
  std::vector<DGEdge<T, T> *> outgoingEdgeInstances;
};

/***************************************************************************************************
 * ElementTraits templates
 ***************************************************************************************************/

template <class GraphType, class NodeType, class T>
struct ElementTraitsBase : public DefaultDOTGraphTraits {
  using NodeRef = NodeType *;
  using nodes_iterator = typename std::vector<NodeRef>::iterator;

  explicit ElementTraitsBase(bool isSimple = false)
    : DefaultDOTGraphTraits(isSimple) {}

  static std::string getNodeAttributes(NodeRef nodeWrapper,
                                       GraphType *dgWrapper) {
    auto node = nodeWrapper->wrappedNode;
    auto dg = dgWrapper->wrappedGraph;
    std::string color =
        dg->isExternal(node->getT()) ? "color=gray" : "color=black";

    std::string subgraph = "printercluster=";
    if (dg->isExternal(node->getT())) {
      auto isIncoming = node->outDegree() > 0;
      subgraph += isIncoming ? "incomingExternal" : "outgoingExternal";
    } else {
      subgraph += "internal";
    }

    return color + "," + subgraph;
  }

  static std::string getEdgeAttributes(NodeRef nodeWrapper,
                                       nodes_iterator nodeIter,
                                       GraphType *dgWrapper) {

    auto dg = dgWrapper->wrappedGraph;
    auto edge = nodeWrapper->outgoingEdgeInstances
                    [nodeIter - nodeWrapper->outgoingNodeInstances.begin()];

    const std::string cntColor = "color=blue";
    const std::string memColor = "color=red";
    const std::string varColor = "color=black";
    std::string attrsStr;
    raw_string_ostream ros(attrsStr);
    ros << (edge->isControlDependence()
                ? cntColor
                : (edge->isMemoryDependence() ? memColor : varColor));
    if (edge->isLoopCarriedDependence())
      ros << ", penwidth=2";
    if (dg->isExternal(edge->getSrc()) || dg->isExternal(edge->getDst()))
      ros << ",style=dotted";
    // dump the edge id set by the map
    if (auto edgeId = dg->getEdgeID(edge)) {
      ros << ",label=" << *edgeId;
    }
    return ros.str();
  }
};

template <class GraphType, class NodeType, class T>
struct ElementTraits : public ElementTraitsBase<GraphType, NodeType, T> {
  using NodeRef = NodeType *;
  using nodes_iterator = typename std::vector<NodeRef>::iterator;

  explicit ElementTraits(bool isSimple = false)
    : ElementTraitsBase<GraphType, NodeType, T>(isSimple) {}

  static std::string getNodeLabel(NodeRef nodeWrapper, GraphType *entry) {
    return nodeWrapper->wrappedNode->toString();
  }

  static std::string getEdgeSourceLabel(NodeRef nodeWrapper,
                                        nodes_iterator nodeIter) {
    auto edge = nodeWrapper->outgoingEdgeInstances
                    [nodeIter - nodeWrapper->outgoingNodeInstances.begin()];
    return edge->toString();
  }
};

/*
 * GraphTraitsBase template
 */

template <class DGWrapper, class NodeType, class T>
struct GraphTraitsBase {
  using NodeRef = NodeType *;
  using ChildIteratorType = typename std::vector<NodeRef>::iterator;
  using nodes_iterator = typename std::unordered_set<NodeRef>::iterator;

  static NodeRef getEntryNode(DGWrapper *dg) {
    return dg->entryNode;
  }

  static nodes_iterator nodes_begin(DGWrapper *dg) {
    return dg->nodes_begin();
  }

  static nodes_iterator nodes_end(DGWrapper *dg) {
    return dg->nodes_end();
  }

  static ChildIteratorType child_begin(NodeRef node) {
    return node->child_begin();
  }

  static ChildIteratorType child_end(NodeRef node) {
    return node->child_end();
  }
};

} // namespace arcana::noelle

namespace llvm {

/*
 * Specializations for GraphTraits and DOTGraphTraits using GraphTraitsBase and
 * ElementTraits
 */

template <>
struct DOTGraphTraits<DGGraphWrapper<PDG, Value> *>
  : public ElementTraits<DGGraphWrapper<PDG, Value>,
                         DGNodeWrapper<Value>,
                         Value> {
  DOTGraphTraits(bool isSimple = false)
    : ElementTraits<DGGraphWrapper<PDG, Value>, DGNodeWrapper<Value>, Value>(
        isSimple) {}
};
template <>
struct GraphTraits<DGGraphWrapper<PDG, Value> *>
  : public GraphTraitsBase<DGGraphWrapper<PDG, Value>,
                           DGNodeWrapper<Value>,
                           Value> {};

template <>
struct DOTGraphTraits<DGGraphWrapper<DG<DGString>, DGString> *>
  : public ElementTraits<DGGraphWrapper<DG<DGString>, DGString>,
                         DGNodeWrapper<DGString>,
                         DGString> {
  DOTGraphTraits(bool isSimple = false)
    : ElementTraits<DGGraphWrapper<DG<DGString>, DGString>,
                    DGNodeWrapper<DGString>,
                    DGString>(isSimple) {}
};
template <>
struct GraphTraits<DGGraphWrapper<DG<DGString>, DGString> *>
  : public GraphTraitsBase<DGGraphWrapper<DG<DGString>, DGString>,
                           DGNodeWrapper<DGString>,
                           DGString> {};

template <>
struct DOTGraphTraits<DGGraphWrapper<SubCFGs, BasicBlock> *>
  : public ElementTraits<DGGraphWrapper<SubCFGs, BasicBlock>,
                         DGNodeWrapper<BasicBlock>,
                         BasicBlock> {
  DOTGraphTraits(bool isSimple = false)
    : ElementTraits<DGGraphWrapper<SubCFGs, BasicBlock>,
                    DGNodeWrapper<BasicBlock>,
                    BasicBlock>(isSimple) {}
};
template <>
struct GraphTraits<DGGraphWrapper<SubCFGs, BasicBlock> *>
  : public GraphTraitsBase<DGGraphWrapper<SubCFGs, BasicBlock>,
                           DGNodeWrapper<BasicBlock>,
                           BasicBlock> {};

} // namespace llvm

#endif // NOELLE_SRC_CORE_DG_DGGRAPHTRAITS_H_
