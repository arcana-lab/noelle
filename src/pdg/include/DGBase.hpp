/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_map>
#include <queue>
#include <set>

using namespace std;
using namespace llvm;

namespace llvm {
  
  /*
   * Program Dependence Graph Node and Edge
   */
  template <class T> class DGNode;
  template <class T, class SubT> class DGEdgeBase;
  template <class T> class DGEdge;

  enum DataDependenceType { DG_DATA_NONE, DG_DATA_RAW, DG_DATA_WAR, DG_DATA_WAW };

  template <class T>
  class DG {
    public:
      typedef typename set<DGNode<T> *>::iterator nodes_iterator;
      typedef typename set<DGNode<T> *>::const_iterator nodes_const_iterator;
      
      typedef typename set<DGEdge<T> *>::iterator edges_iterator;
      typedef typename set<DGEdge<T> *>::const_iterator edges_const_iterator;

      typedef typename unordered_map<T *, DGNode<T> *>::iterator node_map_iterator;

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
      DGNode<T> *getEntryNode() const { return entryNode; }
      void setEntryNode(DGNode<T> *node) { entryNode = node; }

      bool isInternal(T *theT) const { return internalNodeMap.find(theT) != internalNodeMap.end(); }
      bool isExternal(T *theT) const { return externalNodeMap.find(theT) != externalNodeMap.end(); }
      bool isInGraph(T *theT) const { return isInternal(theT) || isExternal(theT); }

      unsigned numNodes() const { return allNodes.size(); }
      unsigned numInternalNodes() const { return internalNodeMap.size(); }
      unsigned numExternalNodes() const { return externalNodeMap.size(); }
      unsigned numEdges() const { return allEdges.size(); }

      /*
       * Iterator ranges
       */
      iterator_range<nodes_iterator>
      getNodes() { return make_range(allNodes.begin(), allNodes.end()); }
      iterator_range<edges_iterator>
      getEdges() { return make_range(allEdges.begin(), allEdges.end()); }

      iterator_range<node_map_iterator>
      internalNodePairs() { return make_range(internalNodeMap.begin(), internalNodeMap.end()); }
      iterator_range<node_map_iterator>
      externalNodePairs() { return make_range(externalNodeMap.begin(), externalNodeMap.end()); }

      /*
       * Fetching/Creating Nodes and Edges
       */
      DGNode<T> *addNode(T *theT, bool inclusion);
      DGNode<T> *fetchOrAddNode(T *theT, bool inclusion);
      DGNode<T> *fetchNode(T *theT);

      DGEdge<T> *addEdge(T *from, T *to);
      DGEdge<T> *fetchEdge(DGNode<T> *in, DGNode<T> *out);
      DGEdge<T> *copyAddEdge(DGEdge<T> &edgeToCopy);

      /*
       * Merging/Extracting Graphs
       */
      std::set<DGNode<T> *> getTopLevelNodes(bool onlyInternal = false);
      std::set<DGNode<T> *> getLeafNodes(bool onlyInternal = false);
      std::vector<std::set<DGNode<T> *> *> getDisconnectedSubgraphs();
      std::set<DGNode<T> *> getNextDepthNodes(DGNode<T> *node);
      std::set<DGNode<T> *> getPreviousDepthNodes(DGNode<T> *node);
      void removeNode(DGNode<T> *node);
      void removeEdge(DGEdge<T> *edge);
      void addNodesIntoNewGraph(DG<T> &newGraph, std::set<DGNode<T> *> nodesToPartition, DGNode<T> *entryNode);
      void clear();

      raw_ostream & print(raw_ostream &stream);

    protected:
      std::set<DGNode<T> *> allNodes;
      std::set<DGEdge<T> *> allEdges;
      DGNode<T> *entryNode;
      unordered_map<T *, DGNode<T> *> internalNodeMap;
      unordered_map<T *, DGNode<T> *> externalNodeMap;
  };

  template <class T> 
  class DGNode
  {
    public:
      DGNode() : theT(nullptr) {}
      DGNode(T *node) : theT(node) {}

      typedef typename std::vector<DGNode<T> *>::iterator nodes_iterator;
      typedef typename std::set<DGEdge<T> *>::iterator edges_iterator;

      edges_iterator begin_edges() { return allConnectedEdges.begin(); }
      edges_iterator end_edges() { return allConnectedEdges.end(); }

      edges_iterator begin_outgoing_edges() { return outgoingEdges.begin(); }
      edges_iterator end_outgoing_edges() { return outgoingEdges.end(); }
      edges_iterator begin_incoming_edges() { return incomingEdges.begin(); }
      edges_iterator end_incoming_edges() { return incomingEdges.end(); }

      nodes_iterator begin_outgoing_nodes() { return outgoingNodeInstances.begin(); }
      nodes_iterator end_outgoing_nodes() { return outgoingNodeInstances.end(); }

      inline iterator_range<edges_iterator>
      getAllConnectedEdges() { return make_range(allConnectedEdges.begin(), allConnectedEdges.end()); }
      inline iterator_range<edges_iterator>
      getOutgoingEdges() { return make_range(outgoingEdges.begin(), outgoingEdges.end()); }
      inline iterator_range<edges_iterator>
      getIncomingEdges() { return make_range(incomingEdges.begin(), incomingEdges.end()); }

      T *getT() const { return theT; }
      std::set<DGEdge<T> *> & getEdgesToAndFromNode(DGNode<T> *node) { return nodeToEdgesMap[node]; }

      unsigned numConnectedEdges() { return allConnectedEdges.size(); }
      unsigned numOutgoingEdges() { return outgoingEdges.size(); }
      unsigned numIncomingEdges() { return incomingEdges.size(); }

      void addIncomingEdge(DGEdge<T> *edge);
      void addOutgoingEdge(DGEdge<T> *edge);
      void removeConnectedEdge(DGEdge<T> *edge);
      void removeConnectedNode(DGNode<T> *node);

      DGEdge<T> *getEdgeInstance(unsigned nodeInstance) { return outgoingEdgeInstances[nodeInstance]; }

      std::string toString();
      raw_ostream &print(raw_ostream &stream);

    protected:
      void removeInstance(DGEdge<T> *edge);
      void removeInstances(DGNode<T> *node);

      T *theT;
      std::set<DGEdge<T> *> allConnectedEdges;
      std::set<DGEdge<T> *> outgoingEdges;
      std::set<DGEdge<T> *> incomingEdges;

      // For use in unconventional graph iteration for LLVM SCCIterator
      std::vector<DGNode<T> *> outgoingNodeInstances;
      std::vector<DGEdge<T> *> outgoingEdgeInstances;

      unordered_map<DGNode<T> *, std::set<DGEdge<T> *>> nodeToEdgesMap;
  };

  template <class T>
  class DGEdge : public DGEdgeBase<T, T>
  {
   public:
    DGEdge(DGNode<T> *src, DGNode<T> *dst) : DGEdgeBase<T, T>(src, dst) {}
    DGEdge(const DGEdge<T> &oldEdge) : DGEdgeBase<T, T>(oldEdge) {}
  };

  template <class T, class SubT>
  class DGEdgeBase
  {
   public:
    DGEdgeBase(DGNode<T> *src, DGNode<T> *dst)
      : from(src), to(dst), memory(false), must(false), dataDepType(DG_DATA_NONE), isControl(false)
      {}
    DGEdgeBase(const DGEdgeBase<T, SubT> &oldEdge);

    typedef typename std::set<DGEdge<SubT> *>::iterator edges_iterator;

    edges_iterator begin_sub_edges() { return subEdges.begin(); }
    edges_iterator end_sub_edges() { return subEdges.end(); }
  
    inline iterator_range<edges_iterator>
    getSubEdges() { return make_range(subEdges.begin(), subEdges.end()); }

    std::pair<DGNode<T> *, DGNode<T> *> getNodePair() const { return std::make_pair(from, to); }
    void setNodePair(DGNode<T> *from, DGNode<T> *to) { this->from = from; this->to = to; }
    DGNode<T> * getOutgoingNode() const { return from; }
    DGNode<T> * getIncomingNode() const { return to; }
    T * getOutgoingT() const { return from->getT(); }
    T * getIncomingT() const { return to->getT(); }

    bool isMemoryDependence() const { return memory; }
    bool isMustDependence() const { return must; }
    bool isRAWDependence() const { return dataDepType == DG_DATA_RAW; }
    bool isWARDependence() const { return dataDepType == DG_DATA_WAR; }
    bool isWAWDependence() const { return dataDepType == DG_DATA_WAW; }
    bool isControlDependence() const { return isControl; }
    bool isDataDependence() const { return !isControl; }
    DataDependenceType dataDependenceType() const { return dataDepType; }

    void setControl(bool ctrl) { isControl = ctrl; }
    void setMemMustType(bool mem, bool must, DataDependenceType dataDepType);

    void addSubEdge(DGEdge<SubT> *edge) { subEdges.insert(edge); }
    void removeSubEdge(DGEdge<SubT> *edge) { subEdges.erase(edge); }
    void clearSubEdges() { subEdges.clear(); }

    std::string toString();
    raw_ostream &print(raw_ostream &stream, std::string linePrefix = "");
    std::string dataDepToString();

   protected:
    DGNode<T> *from, *to;
    std::set<DGEdge<SubT> *> subEdges;
    bool memory, must, isControl;
    DataDependenceType dataDepType;
  };

  /*
   * DG<T> class method implementations
   */
  template <class T>
  DGNode<T> *DG<T>::addNode(T *theT, bool inclusion)
  {
    auto *node = new DGNode<T>(theT);
    allNodes.insert(node);
    auto &map = inclusion ? internalNodeMap : externalNodeMap;
    map[theT] = node;
    return node;
  }

  template <class T>
  DGNode<T> *DG<T>::fetchOrAddNode(T *theT, bool inclusion)
  {
    if (isInGraph(theT)) return fetchNode(theT);
    return addNode(theT, inclusion);
  }

  template <class T>
  DGNode<T> *DG<T>::fetchNode(T *theT)
  {
    auto nodeI = internalNodeMap.find(theT);
    return (nodeI != internalNodeMap.end()) ? nodeI->second : externalNodeMap[theT];
  }

  template <class T>
  DGEdge<T> *DG<T>::addEdge(T *from, T *to)
  {
    auto fromNode = fetchNode(from);
    auto toNode = fetchNode(to);
    auto edge = new DGEdge<T>(fromNode, toNode);
    allEdges.insert(edge);
    fromNode->addOutgoingEdge(edge);
    toNode->addIncomingEdge(edge);
    return edge;
  }

  template <class T>
  DGEdge<T> *DG<T>::fetchEdge(DGNode<T> *From, DGNode<T> *To) {
    return *std::find_if(From->begin_outgoing_edges(), From->end_outgoing_edges(),
        [To](auto edge)
        { return edge->getIncomingNode() == To; });
  }

  template <class T>
  DGEdge<T> *DG<T>::copyAddEdge(DGEdge<T> &edgeToCopy)
  {
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
  std::set<DGNode<T> *> DG<T>::getTopLevelNodes(bool onlyInternal)
  {
    std::set<DGNode<T> *> topLevelNodes;

    /*
     * Add all nodes that have no incoming nodes (other than self)
     */
    for (auto node : allNodes)
    {
      if (onlyInternal && isExternal(node->getT())) continue;

      bool noOtherIncoming = true;
      for (auto incomingE : node->getIncomingEdges())
      {
        bool edgeToSelf = (incomingE->getOutgoingNode() == node);
        bool edgeToExternal = onlyInternal && isExternal(incomingE->getOutgoingT());
        noOtherIncoming &= edgeToSelf || edgeToExternal;
      }
      if (noOtherIncoming) topLevelNodes.insert(node);
    }
    if (topLevelNodes.size() > 0) return topLevelNodes;

    /*
     * Add a node in the top cycle of the graph
     * 1) By the time every node is visited, the node that was capable
     * of fulfilling this requirement must be in the top cycle
     * 2) Should internal nodes only be requested, if there is an
     * internal cycle, "visiting" all external nodes beforehand has
     * no bearing on this method
     */
    std::set<DGNode<T> *> visitedNodes;
    if (onlyInternal) {
      for (auto nodePair : externalNodePairs()) {
        visitedNodes.insert(nodePair.second);
      }
    }

    for (auto node : allNodes)
    {
      if (onlyInternal && isExternal(node->getT())) continue;

      if (visitedNodes.find(node) != visitedNodes.end()) continue;

      std::queue<DGNode<T> *> nodeToTraverse;
      nodeToTraverse.push(node);
      while (!nodeToTraverse.empty())
      {
        auto traverseN = nodeToTraverse.front();
        visitedNodes.insert(traverseN);
        nodeToTraverse.pop();

        for (auto outgoingE : traverseN->getOutgoingEdges())
        {
          auto incomingN = outgoingE->getIncomingNode();
          if (visitedNodes.find(incomingN) != visitedNodes.end()) continue;
          nodeToTraverse.push(incomingN);
        }
      }

      if (visitedNodes.size() == allNodes.size())
      {
        topLevelNodes.insert(node);
        break;
      }
    }

    return topLevelNodes;
  }

  template <class T>
  std::set<DGNode<T> *> DG<T>::getLeafNodes(bool onlyInternal)
  {
    std::set<DGNode<T> *> leafNodes;
    if (onlyInternal) {
      for (auto node : allNodes)
        if (node->numOutgoingEdges() == 0)
          leafNodes.insert(node);
    } else {
      for (auto nodePair : internalNodePairs()) {
        if (nodePair.second->numOutgoingEdges() == 0)
          leafNodes.insert(nodePair.second);
      }
    }
    return leafNodes;
  }

  template <class T>
  std::vector<std::set<DGNode<T> *> *> DG<T>::getDisconnectedSubgraphs()
  {
    std::vector<std::set<DGNode<T> *> *> connectedComponents;
    std::set<DGNode<T> *> visitedNodes;

    for (auto node : allNodes)
    {
      if (visitedNodes.find(node) != visitedNodes.end()) continue;

      /*
       * Perform BFS to find the connected component this node belongs to
       */
      auto component = new std::set<DGNode<T> *>();
      std::queue<DGNode<T> *> connectedNodes;

      visitedNodes.insert(node);
      connectedNodes.push(node);
      while (!connectedNodes.empty())
      {
        auto currentNode = connectedNodes.front();
        connectedNodes.pop();
        component->insert(currentNode);

        auto checkToVisitNode = [&](DGNode<T> *node) -> void {
          if (visitedNodes.find(node) != visitedNodes.end()) return;
          visitedNodes.insert(node);
          connectedNodes.push(node);
        };

        for (auto edge : currentNode->getOutgoingEdges()) checkToVisitNode(edge->getIncomingNode());
        for (auto edge : currentNode->getIncomingEdges()) checkToVisitNode(edge->getOutgoingNode());
      }

      connectedComponents.push_back(component);
    }

    return connectedComponents;
  }

  template <class T>
  std::set<DGNode<T> *> DG<T>::getNextDepthNodes(DGNode<T> *node)
  {
    std::set<DGNode<T> *> incomingNodes;
    for (auto edge : node->getOutgoingEdges()) incomingNodes.insert(edge->getIncomingNode());

    std::set<DGNode<T> *> nextDepthNodes;
    for (auto incoming : incomingNodes)
    {
      /*
       * Check if edge exists from another next to this next node;
       * If so, it isn't the next depth
       */
      bool isNextDepth = true;
      for (auto incomingE : incoming->getIncomingEdges())
      {
        isNextDepth &= (incomingNodes.find(incomingE->getOutgoingNode()) == incomingNodes.end());
      }

      if (!isNextDepth) continue;
      nextDepthNodes.insert(incoming);
    }
    return nextDepthNodes;
  }

  template <class T>
  std::set<DGNode<T> *> DG<T>::getPreviousDepthNodes(DGNode<T> *node)
  {
    std::set<DGNode<T> *> outgoingNodes;
    for (auto edge : node->getIncomingEdges()) outgoingNodes.insert(edge->getOutgoingNode());

    std::set<DGNode<T> *> previousDepthNodes;
    for (auto outgoing : outgoingNodes)
    {
      /*
       * Check if edge exists from this previous to another previous node;
       * If so, it isn't the previous depth
       */
      bool isPrevDepth = true;
      for (auto outgoingE : outgoing->getOutgoingEdges())
      {
        isPrevDepth &= (outgoingNodes.find(outgoingE->getIncomingNode()) == outgoingNodes.end());
      }

      if (!isPrevDepth) continue;
      previousDepthNodes.insert(outgoing);
    }
    return previousDepthNodes;
  }

  template <class T>
  void DG<T>::removeNode(DGNode<T> *node)
  {
    auto theT = node->getT();
    auto &map = isInternal(theT) ? internalNodeMap : externalNodeMap;
    map.erase(theT);
    allNodes.erase(node);

    for (auto edge : node->getIncomingEdges()) edge->getOutgoingNode()->removeConnectedNode(node);
    for (auto edge : node->getOutgoingEdges()) edge->getIncomingNode()->removeConnectedNode(node);
    for (auto edge : node->getAllConnectedEdges()) allEdges.erase(edge);
  }

  template <class T>
  void DG<T>::removeEdge(DGEdge<T> *edge)
  {
    edge->getOutgoingNode()->removeConnectedEdge(edge);
    edge->getIncomingNode()->removeConnectedEdge(edge);
    allEdges.erase(edge);
  }

  template <class T>
  void DG<T>::addNodesIntoNewGraph(DG<T> &newGraph, std::set<DGNode<T> *> nodesToPartition, DGNode<T> *entryNode)
  {
    newGraph.entryNode = entryNode;

    for (auto node : nodesToPartition)
    {
      auto theT = node->getT();
      newGraph.addNode(theT, isInternal(theT));
    }

    /*
     * Only add edges that connect between two nodes in the partition
     */
    for (auto node : nodesToPartition)
    {
      for (auto edgeToCopy : node->getOutgoingEdges())
      {
        auto incomingT = edgeToCopy->getIncomingNode()->getT();
        if (!newGraph.isInGraph(incomingT)) continue;
        newGraph.copyAddEdge(*edgeToCopy);
      }
    }
  }

  template <class T>
  void DG<T>::clear()
  {
    allNodes.clear();
    allEdges.clear();
    entryNode = nullptr;
    internalNodeMap.clear();
    externalNodeMap.clear();
  }

  template <class T>
  raw_ostream & DG<T>::print(raw_ostream &stream)
  {
    stream << "Total nodes: " << allNodes.size() << "\n";
    stream << "Internal nodes: " << internalNodeMap.size() << "\n";
    for (auto pair : internalNodePairs()) pair.second->print(stream) << "\n";
    stream << "External nodes: " << externalNodeMap.size() << "\n";
    for (auto pair : externalNodePairs()) pair.second->print(stream) << "\n";
    stream << "Edges: " << allEdges.size() << "\n";
    for (auto edge : allEdges) edge->print(stream) << "\n";
  }

  /*
   * DGNode<T> class method implementations
   */
  template <class T>
  void DGNode<T>::addIncomingEdge(DGEdge<T> *edge)
  {
    incomingEdges.insert(edge);
    allConnectedEdges.insert(edge);
    auto node = edge->getOutgoingNode();
    nodeToEdgesMap[node].insert(edge);
  }
  
  template <class T>
  void DGNode<T>::addOutgoingEdge(DGEdge<T> *edge)
  {
    outgoingEdges.insert(edge);
    allConnectedEdges.insert(edge);
    auto node = edge->getIncomingNode();
    outgoingNodeInstances.push_back(node);
    outgoingEdgeInstances.push_back(edge);
    nodeToEdgesMap[node].insert(edge);
  }

  template <class T>
  void DGNode<T>::removeInstance(DGEdge<T> *edge)
  {
    auto instanceIter = std::find(outgoingEdgeInstances.begin(), outgoingEdgeInstances.end(), edge);
    auto nodeIter = outgoingNodeInstances.begin() + (instanceIter - outgoingEdgeInstances.begin());
    outgoingEdgeInstances.erase(instanceIter);
    outgoingNodeInstances.erase(nodeIter);
  }

  template <class T>
  void DGNode<T>::removeInstances(DGNode<T> *node)
  {
    for (int i = outgoingNodeInstances.size() - 1; i >= 0; --i)
    {
      if (outgoingNodeInstances[i] != node) continue;
      outgoingNodeInstances.erase(outgoingNodeInstances.begin() + i);
      outgoingEdgeInstances.erase(outgoingEdgeInstances.begin() + i);
    }
  }

  template <class T>
  void DGNode<T>::removeConnectedEdge(DGEdge<T> *edge)
  {
    DGNode<T> *node; 
    if (outgoingEdges.find(edge) != outgoingEdges.end())
    {
      outgoingEdges.erase(edge);
      node = edge->getIncomingNode();
      removeInstance(edge);
    }
    else
    {
      incomingEdges.erase(edge);
      node = edge->getOutgoingNode();
    }

    allConnectedEdges.erase(edge);
    nodeToEdgesMap[node].erase(edge);
    if (nodeToEdgesMap[node].empty()) nodeToEdgesMap.erase(node);
  }

  template <class T>
  void DGNode<T>::removeConnectedNode(DGNode<T> *node)
  {
    for (auto edge : nodeToEdgesMap[node])
    {
      outgoingEdges.erase(edge);
      incomingEdges.erase(edge);
      allConnectedEdges.erase(edge);
    }
    nodeToEdgesMap.erase(node);
    removeInstances(node);
  }

  template <class T>
  std::string DGNode<T>::toString()
  {
    std::string nodeStr;
    raw_string_ostream ros(nodeStr);
    theT->print(ros);
		ros.flush();
    return nodeStr;
  }

  template <>
  inline std::string DGNode<Instruction>::toString()
  {
    if (!theT) return "Empty node";
    std::string str;
    raw_string_ostream instStream(str);
    theT->print(instStream << theT->getFunction()->getName() << ": ");
    return str;
  }

  template <class T>
  raw_ostream & DGNode<T>::print(raw_ostream &stream)
  { 
    theT->print(stream);
    return stream;
  }

  /*
   * DGEdge<T> class method implementations
   */
  template <class T, class SubT>
  DGEdgeBase<T, SubT>::DGEdgeBase(const DGEdgeBase<T, SubT> &oldEdge)
  {
    auto nodePair = oldEdge.getNodePair();
    from = nodePair.first;
    to = nodePair.second;
    setMemMustType(oldEdge.isMemoryDependence(), oldEdge.isMustDependence(), oldEdge.dataDependenceType());
    setControl(oldEdge.isControlDependence());
    for (auto subEdge : oldEdge.subEdges) addSubEdge(subEdge);
  }

  template <class T, class SubT>
  void DGEdgeBase<T, SubT>::setMemMustType(bool mem, bool must, DataDependenceType dataDepType)
  {
    this->memory = mem;
    this->must = must;
    this->dataDepType = dataDepType;
  }

  template <class T, class SubT>
  std::string DGEdgeBase<T, SubT>::dataDepToString()
  {
    if (this->isRAWDependence()) return "RAW";
    else if (this->isWARDependence()) return "WAR";
    else if (this->isWAWDependence()) return "WAW";
    else return "NONE";    
  }

  template <class T, class SubT>
  std::string DGEdgeBase<T, SubT>::toString()
  {
    if (this->subEdges.size() > 0) {
      std::string edgesStr;
      raw_string_ostream ros(edgesStr);
      for (auto edge : this->subEdges) ros << edge->toString();
      return ros.str();
    }
    if (this->isControlDependence()) return "CTRL";
    std::string edgeStr;
    raw_string_ostream ros(edgeStr);
    ros << this->dataDepToString();
    ros << (must ? " (must)" : " (may)");
    ros << (memory ? " from memory " : "");
    ros.flush();
    return edgeStr;
  }
  
  template <class T, class SubT>
  raw_ostream & DGEdgeBase<T, SubT>::print(raw_ostream &stream, std::string linePrefix)
  {
    from->print(stream << linePrefix << "From:\t") << "\n";
    to->print(stream << linePrefix << "To:\t") << "\n";
    stream << linePrefix << this->toString();
    return stream;
  }
}
