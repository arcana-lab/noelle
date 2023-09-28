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
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/Assumptions.hpp"
#include "noelle/core/DGNode.hpp"
#include "noelle/core/DGEdge.hpp"

namespace llvm::noelle {

template <class T>
class DG {
public:
  DG();

  typedef typename std::set<DGNode<T> *>::iterator nodes_iterator;
  typedef typename std::set<DGNode<T> *>::const_iterator nodes_const_iterator;

  typedef typename std::set<DGEdge<T> *>::iterator edges_iterator;
  typedef typename std::set<DGEdge<T> *>::const_iterator edges_const_iterator;
  typedef std::map<DGEdge<T> *, uint32_t> DepIdReverseMap_t;

  typedef typename std::map<T *, DGNode<T> *>::iterator node_map_iterator;

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

  unsigned numNodes() const {
    return allNodes.size();
  }
  unsigned numInternalNodes() const {
    return internalNodeMap.size();
  }
  unsigned numExternalNodes() const {
    return externalNodeMap.size();
  }
  unsigned numEdges() const {
    return allEdges.size();
  }

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

  DGEdge<T> *addEdge(T *from, T *to);
  std::unordered_set<DGEdge<T> *> fetchEdges(DGNode<T> *From, DGNode<T> *To);
  DGEdge<T> *copyAddEdge(DGEdge<T> &edgeToCopy);

  /*
   * Deal with the id for each edge and the corresponding map for debugging
   */
  std::optional<uint32_t> getEdgeID(DGEdge<T> *edge) {
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
  void removeEdge(DGEdge<T> *edge);
  void copyNodesIntoNewGraph(DG<T> &newGraph,
                             std::set<DGNode<T> *> nodesToPartition,
                             DGNode<T> *entryNode);
  void clear(void);

  raw_ostream &print(raw_ostream &stream);

  static std::vector<DGEdge<T> *> sortDependences(
      const std::set<DGEdge<T> *> &set);

protected:
  int32_t nodeIdCounter;
  std::set<DGNode<T> *> allNodes;
  std::set<DGEdge<T> *> allEdges;
  DGNode<T> *entryNode;
  std::map<T *, DGNode<T> *> internalNodeMap;
  std::map<T *, DGNode<T> *> externalNodeMap;
  std::shared_ptr<DepIdReverseMap_t> depLookupMap;
};

template <class T>
DG<T>::DG() : nodeIdCounter{ 0 },
              depLookupMap{ nullptr } {

  return;
}

/*
 * DG<T> class method implementations
 */
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
DGEdge<T> *DG<T>::addEdge(T *from, T *to) {
  auto fromNode = fetchNode(from);
  auto toNode = fetchNode(to);
  auto edge = new DGEdge<T>(fromNode, toNode);
  allEdges.insert(edge);
  fromNode->addOutgoingEdge(edge);
  toNode->addIncomingEdge(edge);
  return edge;
}

template <class T>
std::unordered_set<DGEdge<T> *> DG<T>::fetchEdges(DGNode<T> *From,
                                                  DGNode<T> *To) {
  std::unordered_set<DGEdge<T> *> edgeSet;

  for (auto &edge : From->getOutgoingEdges()) {
    if (edge->getIncomingNode() == To) {
      edgeSet.insert(edge);
    }
  }

  return edgeSet;
}

template <class T>
DGEdge<T> *DG<T>::copyAddEdge(DGEdge<T> &edgeToCopy) {
  auto edge = new DGEdge<T>(edgeToCopy);
  allEdges.insert(edge);

  /*
   * Point copy of edge to equivalent nodes in this graph
   */
  auto nodePair = edgeToCopy.getNodePair();
  auto fromNode = fetchNode(nodePair.first->getT());
  auto toNode = fetchNode(nodePair.second->getT());
  edge->setNodePair(fromNode, toNode);

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
      bool edgeToSelf = (incomingE->getOutgoingNode() == node);
      bool edgeToExternal =
          onlyInternal && isExternal(incomingE->getOutgoingT());
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
        noChildNode &= (edge->getIncomingNode() == selfNode);
      }
      if (noChildNode)
        leafNodes.insert(selfNode);
    }
  } else {
    for (auto selfNodePair : internalNodePairs()) {
      bool noChildNode = true;
      for (auto edge : selfNodePair.second->getOutgoingEdges()) {
        noChildNode &= (edge->getIncomingNode() == selfNodePair.second);
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
        checkToVisitNode(edge->getIncomingNode());
      for (auto edge : currentNode->getIncomingEdges())
        checkToVisitNode(edge->getOutgoingNode());
    }

    connectedComponents.push_back(component);
  }

  return connectedComponents;
}

template <class T>
std::unordered_set<DGNode<T> *> DG<T>::getNextDepthNodes(DGNode<T> *node) {
  std::unordered_set<DGNode<T> *> incomingNodes;
  for (auto edge : node->getOutgoingEdges())
    incomingNodes.insert(edge->getIncomingNode());

  std::unordered_set<DGNode<T> *> nextDepthNodes;
  for (auto incoming : incomingNodes) {
    /*
     * Check if edge exists from another next to this next node;
     * If so, it isn't the next depth
     */
    bool isNextDepth = true;
    for (auto incomingE : incoming->getIncomingEdges()) {
      isNextDepth &= (incomingNodes.find(incomingE->getOutgoingNode())
                      == incomingNodes.end());
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
    outgoingNodes.insert(edge->getOutgoingNode());

  std::unordered_set<DGNode<T> *> previousDepthNodes;
  for (auto outgoing : outgoingNodes) {
    /*
     * Check if edge exists from this previous to another previous node;
     * If so, it isn't the previous depth
     */
    bool isPrevDepth = true;
    for (auto outgoingE : outgoing->getOutgoingEdges()) {
      isPrevDepth &= (outgoingNodes.find(outgoingE->getIncomingNode())
                      == outgoingNodes.end());
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
  std::unordered_set<DGEdge<T> *> incomingToNode;
  std::unordered_set<DGEdge<T> *> outgoingFromNode;
  std::unordered_set<DGEdge<T> *> allToAndFromNode;
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
    edge->getOutgoingNode()->removeConnectedNode(node);
  for (auto edge : outgoingFromNode)
    edge->getIncomingNode()->removeConnectedNode(node);
  for (auto edge : allToAndFromNode) {
    allEdges.erase(edge);
    delete edge;
  }

  delete node;
}

template <class T>
void DG<T>::removeEdge(DGEdge<T> *edge) {
  edge->getOutgoingNode()->removeConnectedEdge(edge);
  edge->getIncomingNode()->removeConnectedEdge(edge);
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
      auto incomingT = edgeToCopy->getIncomingNode()->getT();
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
std::vector<DGEdge<T> *> DG<T>::sortDependences(
    const std::set<DGEdge<T> *> &set) {
  std::vector<DGEdge<T> *> v;

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
  auto sortingFunction = [](DGEdge<T> *d1, DGEdge<T> *d2) -> bool {
    assert(d1 != nullptr);
    assert(d2 != nullptr);

    auto src1 = d1->getOutgoingT();
    auto src2 = d2->getOutgoingT();
    assert(src1 != nullptr);
    assert(src2 != nullptr);
    if (src1 < src2) {
      return true;
    }
    if (src1 > src2) {
      return false;
    }
    assert(src1 == src2);

    auto dst1 = d1->getIncomingT();
    auto dst2 = d2->getIncomingT();
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

} // namespace llvm::noelle
