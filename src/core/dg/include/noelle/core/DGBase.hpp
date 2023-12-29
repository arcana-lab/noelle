/*
 * Copyright 2016 - 2023  Angelo Matni, Yian Su, Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_DG_DGBASE_H_
#define NOELLE_SRC_CORE_DG_DGBASE_H_

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/DGNode.hpp"
#include "noelle/core/DGEdge.hpp"
#include "noelle/core/DataDependence.hpp"
#include "noelle/core/MayMemoryDependence.hpp"
#include "noelle/core/MustMemoryDependence.hpp"
#include "noelle/core/VariableDependence.hpp"
#include "noelle/core/ControlDependence.hpp"
#include "noelle/core/UndefinedDependence.hpp"

namespace arcana::noelle {

template <class T>
class DG {
public:
  DG();

  using nodes_iterator = typename std::set<DGNode<T> *>::iterator;
  using nodes_const_iterator = typename std::set<DGNode<T> *>::const_iterator;
  using edges_iterator = typename std::set<DGEdge<T, T> *>::iterator;
  using edges_const_iterator =
      typename std::set<DGEdge<T, T> *>::const_iterator;
  using node_map_iterator = typename std::map<T *, DGNode<T> *>::iterator;
  typedef std::map<DGEdge<T, T> *, uint32_t> DepIdReverseMap_t;

  /*
   * Node and Edge Iterators
   */
  nodes_iterator begin_nodes() {
    auto n = allNodes.begin();
    return n;
  }

  nodes_iterator end_nodes() {
    auto n = allNodes.end();
    return n;
  }

  nodes_const_iterator begin_nodes() const {
    auto n = allNodes.begin();
    return n;
  }

  nodes_const_iterator end_nodes() const {
    auto n = allNodes.end();
    return n;
  }

  node_map_iterator begin_internal_node_map() {
    auto n = internalNodeMap.begin();
    return n;
  }

  node_map_iterator end_internal_node_map() {
    auto n = internalNodeMap.end();
    return n;
  }

  node_map_iterator begin_external_node_map() {
    auto n = externalNodeMap.begin();
    return n;
  }

  node_map_iterator end_external_node_map() {
    auto n = externalNodeMap.end();
    return n;
  }

  edges_iterator begin_edges() {
    auto e = allEdges.begin();
    return e;
  }

  edges_iterator end_edges() {
    auto e = allEdges.end();
    return e;
  }

  edges_const_iterator begin_edges() const {
    auto e = allEdges.begin();
    return e;
  }

  edges_const_iterator end_edges() const {
    auto e = allEdges.end();
    return e;
  }

  /*
   * Node and Edge Properties
   */
  DGNode<T> *getEntryNode() const {
    return entryNode;
  }
  void setEntryNode(DGNode<T> *node) {
    entryNode = node;
  }

  bool isInternal(T *theT) const;
  bool isExternal(T *theT) const;
  bool isInGraph(T *theT) const;

  uint64_t numNodes(void) const;
  uint64_t numInternalNodes(void) const;
  uint64_t numExternalNodes(void) const;
  uint64_t numEdges(void) const;

  /*
   * Iterator ranges
   */
  iterator_range<nodes_iterator> getNodes() {
    return make_range(allNodes.begin(), allNodes.end());
  }
  iterator_range<edges_iterator> getEdges() {
    return make_range(allEdges.begin(), allEdges.end());
  }

  iterator_range<node_map_iterator> internalNodePairs() {
    return make_range(internalNodeMap.begin(), internalNodeMap.end());
  }
  iterator_range<node_map_iterator> externalNodePairs() {
    return make_range(externalNodeMap.begin(), externalNodeMap.end());
  }

  /*
   * Fetching/Creating Nodes and Edges
   */
  DGNode<T> *addNode(T *theT, bool inclusion);
  DGNode<T> *fetchOrAddNode(T *theT, bool inclusion);
  DGNode<T> *fetchNode(T *theT);
  const DGNode<T> *fetchConstNode(T *theT) const;

  DGEdge<T, T> *addVariableDataDependenceEdge(T *from,
                                              T *to,
                                              DataDependenceType t);
  DGEdge<T, T> *addMemoryDataDependenceEdge(T *from,
                                            T *to,
                                            DataDependenceType t,
                                            bool isMust);
  DGEdge<T, T> *addControlDependenceEdge(T *from, T *to);
  DGEdge<T, T> *addUndefinedDependenceEdge(T *from, T *to);
  std::unordered_set<DGEdge<T, T> *> fetchEdges(DGNode<T> *From, DGNode<T> *To);
  DGEdge<T, T> *copyAddEdge(DGEdge<T, T> &edgeToCopy);

  /*
   * Deal with the id for each edge and the corresponding map for debugging
   */
  std::optional<uint32_t> getEdgeID(DGEdge<T, T> *edge) {
    if (depLookupMap && depLookupMap->find(edge) != depLookupMap->end())
      return depLookupMap->at(edge);
    else
      return std::nullopt;
  }

  void setDepLookupMap(std::shared_ptr<DepIdReverseMap_t> depLookupMap) {
    this->depLookupMap = depLookupMap;
  }

  /*
   * Merging/Extracting Graphs
   */
  std::unordered_set<DGNode<T> *> getTopLevelNodes(bool onlyInternal = false);
  std::unordered_set<DGNode<T> *> getLeafNodes(bool onlyInternal = false);
  std::vector<std::unordered_set<DGNode<T> *> *> getDisconnectedSubgraphs(void);
  std::unordered_set<DGNode<T> *> getNextDepthNodes(DGNode<T> *node);
  std::unordered_set<DGNode<T> *> getPreviousDepthNodes(DGNode<T> *node);
  void removeNode(DGNode<T> *node);
  void removeEdge(DGEdge<T, T> *edge);
  void copyNodesIntoNewGraph(DG<T> &newGraph,
                             std::set<DGNode<T> *> nodesToPartition,
                             DGNode<T> *entryNode);
  void clear(void);

  raw_ostream &print(raw_ostream &stream);

  static std::vector<DGEdge<T, T> *> sortDependences(
      const std::set<DGEdge<T, T> *> &set);

protected:
  int32_t nodeIdCounter;
  std::set<DGNode<T> *> allNodes;
  std::set<DGEdge<T, T> *> allEdges;
  DGNode<T> *entryNode;
  std::map<T *, DGNode<T> *> internalNodeMap;
  std::map<T *, DGNode<T> *> externalNodeMap;
  std::shared_ptr<DepIdReverseMap_t> depLookupMap;
};

/*
 * DG<T> class method implementations
 */
template <class T>
DG<T>::DG() : nodeIdCounter{ 0 },
              depLookupMap{ nullptr } {

  return;
}

template <class T>
DGNode<T> *DG<T>::addNode(T *theT, bool inclusion) {
  auto node = new DGNode<T>(nodeIdCounter++, theT);
  allNodes.insert(node);
  auto &map = inclusion ? internalNodeMap : externalNodeMap;
  map[theT] = node;
  return node;
}

template <class T>
bool DG<T>::isInternal(T *theT) const {
  return internalNodeMap.find(theT) != internalNodeMap.end();
}

template <class T>
bool DG<T>::isExternal(T *theT) const {
  return externalNodeMap.find(theT) != externalNodeMap.end();
}

template <class T>
bool DG<T>::isInGraph(T *theT) const {
  return isInternal(theT) || isExternal(theT);
}

template <class T>
DGNode<T> *DG<T>::fetchOrAddNode(T *theT, bool inclusion) {
  if (isInGraph(theT))
    return fetchNode(theT);
  return addNode(theT, inclusion);
}

template <class T>
DGNode<T> *DG<T>::fetchNode(T *theT) {
  auto nodeI = internalNodeMap.find(theT);
  return (nodeI != internalNodeMap.end()) ? nodeI->second
                                          : externalNodeMap[theT];
}

template <class T>
const DGNode<T> *DG<T>::fetchConstNode(T *theT) const {
  auto nodeI = internalNodeMap.find(theT);
  return (nodeI != internalNodeMap.end()) ? nodeI->second
                                          : externalNodeMap.find(theT)->second;
}

template <class T>
DGEdge<T, T> *DG<T>::addVariableDataDependenceEdge(T *from,
                                                   T *to,
                                                   DataDependenceType t) {
  auto fromNode = this->fetchNode(from);
  auto toNode = this->fetchNode(to);
  auto edge = new VariableDependence<T, T>(fromNode, toNode, t);
  allEdges.insert(edge);
  fromNode->addOutgoingEdge(edge);
  toNode->addIncomingEdge(edge);
  return edge;
}

template <class T>
DGEdge<T, T> *DG<T>::addMemoryDataDependenceEdge(T *from,
                                                 T *to,
                                                 DataDependenceType t,
                                                 bool isMust) {
  auto fromNode = this->fetchNode(from);
  auto toNode = this->fetchNode(to);
  DGEdge<T, T> *edge = nullptr;
  if (isMust) {
    edge = new MustMemoryDependence<T, T>(fromNode, toNode, t);
  } else {
    edge = new MayMemoryDependence<T, T>(fromNode, toNode, t);
  }
  assert(edge != nullptr);

  allEdges.insert(edge);
  fromNode->addOutgoingEdge(edge);
  toNode->addIncomingEdge(edge);
  return edge;
}

template <class T>
DGEdge<T, T> *DG<T>::addControlDependenceEdge(T *from, T *to) {
  auto fromNode = this->fetchNode(from);
  auto toNode = this->fetchNode(to);
  auto edge = new ControlDependence<T, T>(fromNode, toNode);
  allEdges.insert(edge);
  fromNode->addOutgoingEdge(edge);
  toNode->addIncomingEdge(edge);
  return edge;
}

template <class T>
DGEdge<T, T> *DG<T>::addUndefinedDependenceEdge(T *from, T *to) {
  auto fromNode = this->fetchNode(from);
  auto toNode = this->fetchNode(to);
  auto edge = new UndefinedDependence<T, T>(fromNode, toNode);
  allEdges.insert(edge);
  fromNode->addOutgoingEdge(edge);
  toNode->addIncomingEdge(edge);
  return edge;
}

template <class T>
std::unordered_set<DGEdge<T, T> *> DG<T>::fetchEdges(DGNode<T> *From,
                                                     DGNode<T> *To) {
  std::unordered_set<DGEdge<T, T> *> edgeSet;

  for (auto &edge : From->getOutgoingEdges()) {
    if (edge->getDstNode() == To) {
      edgeSet.insert(edge);
    }
  }

  return edgeSet;
}

template <class T>
DGEdge<T, T> *DG<T>::copyAddEdge(DGEdge<T, T> &edgeToCopy) {
  DGEdge<T, T> *edge = nullptr;
  if (isa<ControlDependence<T, T>>(&edgeToCopy)) {
    auto edgeToCopyAsCD = cast<ControlDependence<T, T>>(&edgeToCopy);
    edge = new ControlDependence<T, T>(*edgeToCopyAsCD);
  } else {
    if (isa<VariableDependence<T, T>>(&edgeToCopy)) {
      auto edgeToCopyAsVD = cast<VariableDependence<T, T>>(&edgeToCopy);
      edge = new VariableDependence<T, T>(*edgeToCopyAsVD);
    } else if (isa<MayMemoryDependence<T, T>>(&edgeToCopy)) {
      auto edgeToCopyAsMD = cast<MayMemoryDependence<T, T>>(&edgeToCopy);
      edge = new MayMemoryDependence<T, T>(*edgeToCopyAsMD);
    } else {
      auto edgeToCopyAsMD = cast<MustMemoryDependence<T, T>>(&edgeToCopy);
      edge = new MustMemoryDependence<T, T>(*edgeToCopyAsMD);
    }
  }
  allEdges.insert(edge);

  /*
   * Point copy of edge to equivalent nodes in this graph
   */
  auto fromNode = fetchNode(edgeToCopy.getSrc());
  auto toNode = fetchNode(edgeToCopy.getDst());
  edge->setSrcNode(fromNode);
  edge->setDstNode(toNode);

  fromNode->addOutgoingEdge(edge);
  toNode->addIncomingEdge(edge);
  return edge;
}

template <class T>
std::unordered_set<DGNode<T> *> DG<T>::getTopLevelNodes(bool onlyInternal) {
  std::unordered_set<DGNode<T> *> topLevelNodes;

  /*
   * Add all nodes that have no incoming nodes
   * Exclude self, and external nodes if onlyInternal = true
   */
  for (auto node : allNodes) {
    if (onlyInternal && isExternal(node->getT()))
      continue;

    bool noOtherIncoming = true;
    for (auto incomingE : node->getIncomingEdges()) {
      bool edgeToSelf = (incomingE->getSrcNode() == node);
      bool edgeToExternal = onlyInternal && isExternal(incomingE->getSrc());
      noOtherIncoming &= edgeToSelf || edgeToExternal;
    }
    if (noOtherIncoming)
      topLevelNodes.insert(node);
  }

  return topLevelNodes;
}

template <class T>
std::unordered_set<DGNode<T> *> DG<T>::getLeafNodes(bool onlyInternal) {
  std::unordered_set<DGNode<T> *> leafNodes;
  if (onlyInternal) {
    for (auto selfNode : allNodes) {
      bool noChildNode = true;
      for (auto edge : selfNode->getOutgoingEdges()) {
        noChildNode &= (edge->getDstNode() == selfNode);
      }
      if (noChildNode)
        leafNodes.insert(selfNode);
    }
  } else {
    for (auto selfNodePair : internalNodePairs()) {
      bool noChildNode = true;
      for (auto edge : selfNodePair.second->getOutgoingEdges()) {
        noChildNode &= (edge->getDstNode() == selfNodePair.second);
      }
      if (noChildNode)
        leafNodes.insert(selfNodePair.second);
    }
  }
  return leafNodes;
}

template <class T>
std::vector<std::unordered_set<DGNode<T> *> *> DG<T>::getDisconnectedSubgraphs(
    void) {
  std::vector<std::unordered_set<DGNode<T> *> *> connectedComponents;
  std::unordered_set<DGNode<T> *> visitedNodes;

  for (auto node : allNodes) {
    if (visitedNodes.find(node) != visitedNodes.end())
      continue;

    /*
     * Perform BFS to find the connected component this node belongs to
     */
    auto component = new std::unordered_set<DGNode<T> *>();
    std::queue<DGNode<T> *> connectedNodes;

    visitedNodes.insert(node);
    connectedNodes.push(node);
    while (!connectedNodes.empty()) {
      auto currentNode = connectedNodes.front();
      connectedNodes.pop();
      component->insert(currentNode);

      auto checkToVisitNode = [&](DGNode<T> *node) -> void {
        if (visitedNodes.find(node) != visitedNodes.end())
          return;
        visitedNodes.insert(node);
        connectedNodes.push(node);
      };

      for (auto edge : currentNode->getOutgoingEdges())
        checkToVisitNode(edge->getDstNode());
      for (auto edge : currentNode->getIncomingEdges())
        checkToVisitNode(edge->getSrcNode());
    }

    connectedComponents.push_back(component);
  }

  return connectedComponents;
}

template <class T>
std::unordered_set<DGNode<T> *> DG<T>::getNextDepthNodes(DGNode<T> *node) {
  std::unordered_set<DGNode<T> *> incomingNodes;
  for (auto edge : node->getOutgoingEdges())
    incomingNodes.insert(edge->getDstNode());

  std::unordered_set<DGNode<T> *> nextDepthNodes;
  for (auto incoming : incomingNodes) {
    /*
     * Check if edge exists from another next to this next node;
     * If so, it isn't the next depth
     */
    bool isNextDepth = true;
    for (auto incomingE : incoming->getIncomingEdges()) {
      isNextDepth &=
          (incomingNodes.find(incomingE->getSrcNode()) == incomingNodes.end());
    }

    if (!isNextDepth)
      continue;
    nextDepthNodes.insert(incoming);
  }
  return nextDepthNodes;
}

template <class T>
std::unordered_set<DGNode<T> *> DG<T>::getPreviousDepthNodes(DGNode<T> *node) {
  std::unordered_set<DGNode<T> *> outgoingNodes;
  for (auto edge : node->getIncomingEdges())
    outgoingNodes.insert(edge->getSrcNode());

  std::unordered_set<DGNode<T> *> previousDepthNodes;
  for (auto outgoing : outgoingNodes) {
    /*
     * Check if edge exists from this previous to another previous node;
     * If so, it isn't the previous depth
     */
    bool isPrevDepth = true;
    for (auto outgoingE : outgoing->getOutgoingEdges()) {
      isPrevDepth &=
          (outgoingNodes.find(outgoingE->getDstNode()) == outgoingNodes.end());
    }

    if (!isPrevDepth)
      continue;
    previousDepthNodes.insert(outgoing);
  }
  return previousDepthNodes;
}

template <class T>
void DG<T>::removeNode(DGNode<T> *node) {
  auto theT = node->getT();
  auto &map = isInternal(theT) ? internalNodeMap : externalNodeMap;
  map.erase(theT);
  allNodes.erase(node);

  /*
   * Collect edges to operate on before doing deletes
   */
  std::unordered_set<DGEdge<T, T> *> incomingToNode;
  std::unordered_set<DGEdge<T, T> *> outgoingFromNode;
  std::unordered_set<DGEdge<T, T> *> allToAndFromNode;
  for (auto edge : node->getIncomingEdges())
    incomingToNode.insert(edge);
  for (auto edge : node->getOutgoingEdges())
    outgoingFromNode.insert(edge);
  for (auto edge : node->getAllEdges())
    allToAndFromNode.insert(edge);

  /*
   * Delete relations to edges and edges themselves
   */
  for (auto edge : incomingToNode)
    edge->getSrcNode()->removeConnectedNode(node);
  for (auto edge : outgoingFromNode)
    edge->getDstNode()->removeConnectedNode(node);
  for (auto edge : allToAndFromNode) {
    allEdges.erase(edge);
    delete edge;
  }

  delete node;
}

template <class T>
void DG<T>::removeEdge(DGEdge<T, T> *edge) {
  edge->getSrcNode()->removeConnectedEdge(edge);
  edge->getDstNode()->removeConnectedEdge(edge);
  allEdges.erase(edge);
  delete edge;
}

template <class T>
void DG<T>::copyNodesIntoNewGraph(DG<T> &newGraph,
                                  std::set<DGNode<T> *> nodesToPartition,
                                  DGNode<T> *entryNode) {

  for (auto node : nodesToPartition) {
    auto theT = node->getT();
    auto clonedNode = newGraph.addNode(theT, isInternal(theT));
    if (theT == entryNode->getT())
      newGraph.setEntryNode(clonedNode);
  }

  /*
   * Only add edges that connect between two nodes in the partition
   */
  for (auto node : nodesToPartition) {
    for (auto edgeToCopy : node->getOutgoingEdges()) {
      auto incomingT = edgeToCopy->getDstNode()->getT();
      if (!newGraph.isInGraph(incomingT))
        continue;
      newGraph.copyAddEdge(*edgeToCopy);
    }
  }
}

template <class T>
void DG<T>::clear(void) {
  allNodes.clear();
  allEdges.clear();
  entryNode = nullptr;
  internalNodeMap.clear();
  externalNodeMap.clear();
}

template <class T>
raw_ostream &DG<T>::print(raw_ostream &stream) {
  stream << "Total node count: " << allNodes.size() << "\n";
  stream << "Internal node count: " << internalNodeMap.size() << "\n";
  for (auto pair : internalNodePairs())
    pair.second->print(stream) << "\n";
  stream << "External node count: " << externalNodeMap.size() << "\n";
  for (auto pair : externalNodePairs())
    pair.second->print(stream) << "\n";
  stream << "Edge count: " << allEdges.size() << "\n";
  for (auto edge : allEdges)
    edge->print(stream) << "\n";
  return stream;
}

template <class T>
std::string DGNode<T>::toString(void) const {
  std::string nodeStr;
  raw_string_ostream ros(nodeStr);
  theT->print(ros);
  ros.flush();
  return nodeStr;
}

template <>
inline std::string DGNode<Instruction>::toString(void) const {
  if (!theT)
    return "Empty node";
  std::string str;
  raw_string_ostream instStream(str);
  theT->print(instStream << theT->getFunction()->getName() << ": ");
  return str;
}

template <class T>
std::vector<DGEdge<T, T> *> DG<T>::sortDependences(
    const std::set<DGEdge<T, T> *> &set) {
  std::vector<DGEdge<T, T> *> v;

  /*
   * Fetch all edges.
   */
  for (auto edge : set) {
    assert(edge != nullptr);
    v.push_back(edge);
  }

  /*
   * Sort
   */
  auto sortingFunction = [](DGEdge<T, T> *d1, DGEdge<T, T> *d2) -> bool {
    assert(d1 != nullptr);
    assert(d2 != nullptr);

    auto src1 = d1->getSrc();
    auto src2 = d2->getSrc();
    assert(src1 != nullptr);
    assert(src2 != nullptr);
    if (src1 < src2) {
      return true;
    }
    if (src1 > src2) {
      return false;
    }
    assert(src1 == src2);

    auto dst1 = d1->getDst();
    auto dst2 = d2->getDst();
    assert(dst1 != nullptr);
    assert(dst2 != nullptr);
    if (dst1 < dst2) {
      return true;
    }
    if (dst1 > dst2) {
      return false;
    }
    assert(dst1 == dst2);

    return false;
  };
  std::sort(v.begin(), v.end(), sortingFunction);

  return v;
}

template <class T>
uint64_t DG<T>::numNodes(void) const {
  return allNodes.size();
}

template <class T>
uint64_t DG<T>::numInternalNodes(void) const {
  return internalNodeMap.size();
}

template <class T>
uint64_t DG<T>::numExternalNodes(void) const {
  return externalNodeMap.size();
}

template <class T>
uint64_t DG<T>::numEdges(void) const {
  return allEdges.size();
}

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_DG_DGBASE_H_
