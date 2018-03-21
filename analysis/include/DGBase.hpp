#pragma once

#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_map>

using namespace std;
using namespace llvm;

namespace llvm {
  
  /*
   * Program Dependence Graph Node and Edge
   */
  template <class T> class DGNode;
  template <class T, class SubT> class DGEdgeBase;
  template <class T> class DGEdge;

  template <class T>
  class DG
  {
    public:
      typedef typename vector<DGNode<T> *>::iterator nodes_iterator;
      typedef typename vector<DGNode<T> *>::const_iterator nodes_const_iterator;
      
      typedef typename vector<DGEdge<T> *>::iterator edges_iterator;
      typedef typename vector<DGEdge<T> *>::const_iterator edges_const_iterator;

      typedef typename unordered_map<T *, DGNode<T> *>::iterator node_map_iterator;

      /*
       * Node and Edge Iterators
       */
      nodes_iterator begin_nodes() { allNodes.begin(); }
      nodes_iterator end_nodes() { allNodes.end(); }
      nodes_const_iterator begin_nodes() const { allNodes.begin(); }
      nodes_const_iterator end_nodes() const { allNodes.end(); }

      node_map_iterator begin_internal_node_map() { internalNodeMap.begin(); }
      node_map_iterator end_internal_node_map() { internalNodeMap.end(); }
      node_map_iterator begin_external_node_map() { externalNodeMap.begin(); }
      node_map_iterator end_external_node_map() { externalNodeMap.end(); }

      edges_iterator begin_edges() { allEdges.begin(); }
      edges_iterator end_edges() { allEdges.end(); }
      edges_const_iterator begin_edges() const { allEdges.begin(); }
      edges_const_iterator end_edges() const { allEdges.end(); }

      /*
       * Node and Edge Properties
       */
      DGNode<T> *getEntryNode() { return entryNode; }

      bool isInternal(T *theT) { return internalNodeMap.find(theT) != internalNodeMap.end(); }
      bool isExternal(T *theT) { return externalNodeMap.find(theT) != externalNodeMap.end(); }
      bool isInGraph(T *theT) { return isInternal(theT) || isExternal(theT); }

      unsigned numNodes() { return std::distance(allNodes.begin(), allNodes.end()); }
      unsigned numInternalNodes() { return std::distance(internalNodeMap.begin(), internalNodeMap.end()); }
      unsigned numExternalNodes() { return std::distance(externalNodeMap.begin(), externalNodeMap.end()); }

      /*
       * Instruction Node Pair Iterator
       */
      iterator_range<node_map_iterator>
      internalNodePairs() { return make_range(internalNodeMap.begin(), internalNodeMap.end()); }

      iterator_range<node_map_iterator>
      externalNodePairs() { return make_range(externalNodeMap.begin(), externalNodeMap.end()); }

      /*
       * Fetching/Creating Nodes and Edges
       */
      DGNode<T> *createNodeFrom(T *theT, bool inclusion);
      DGEdge<T> *createEdgeFromTo(T *from, T *to);
      DGNode<T> *fetchOrCreateNodeOf(T *theT, bool inclusion);

      /*
       * Merging/Extracting Graphs
       */
      void extractNodesFromSelfInto(DG<T> &emptyDG, std::vector<DGNode<T> *> nodesToExtract, DGNode<T> *entry);

      raw_ostream & print(raw_ostream &stream);

    protected:
      DGNode<T> *fetchNodeOf(T *theT);
      inline void connectNodesVia(DGEdge<T> *edge, DGNode<T> *from, DGNode<T> *to);

      std::vector<DGNode<T> *> allNodes;
      std::vector<DGEdge<T> *> allEdges;
      DGNode<T> *entryNode;
      unordered_map<T *, DGNode<T> *> internalNodeMap;
      unordered_map<T *, DGNode<T> *> externalNodeMap;
  };

  // Template DG node to abstract node type
  template <class T> 
  class DGNode {
    public:
      DGNode() : theT(nullptr) {}
      DGNode(T *node) : theT(node) {}

      typedef typename std::vector<DGNode<T> *>::iterator nodes_iterator;
      typedef typename std::vector<DGEdge<T> *>::iterator edges_iterator;

      nodes_iterator begin_outgoing_nodes() { return outgoingNodes.begin(); }
      nodes_iterator end_outgoing_nodes() { return outgoingNodes.end(); }
      nodes_iterator begin_incoming_nodes() { return incomingNodes.begin(); }
      nodes_iterator end_incoming_nodes() { return incomingNodes.end(); }

      edges_iterator begin_outgoing_edges() { return outgoingEdges.begin(); }
      edges_iterator end_outgoing_edges() { return outgoingEdges.end(); }
      edges_iterator begin_incoming_edges() { return incomingEdges.begin(); }
      edges_iterator end_incoming_edges() { return incomingEdges.end(); }

      T *getT() const { return theT; }

      std::string toString();
      raw_ostream &print(raw_ostream &stream);
      
      unsigned numOutgoingEdges() { return std::distance(outgoingEdges.begin(), outgoingEdges.end()); }
      unsigned numIncomingEdges() { return std::distance(incomingEdges.begin(), incomingEdges.end()); }

      void addIncomingNode(DGNode<T> *node, DGEdge<T> *edge);
      void addOutgoingNode(DGNode<T> *node, DGEdge<T> *edge);
      
      DGEdge<T> *getEdgeFromNodeIterator(nodes_iterator target, bool incomingEdge = false);

    protected:
      T *theT;
      // To iterate connected nodes directly by node 
      std::vector<DGNode<T> *> outgoingNodes;
      std::vector<DGNode<T> *> incomingNodes;
      // To iterate connected nodes indirectly by edge 
      std::vector<DGEdge<T> *> outgoingEdges;
      std::vector<DGEdge<T> *> incomingEdges;
  };

  template <class T> class DGEdge : public DGEdgeBase<T, T> {
   public:
    DGEdge(DGNode<T> *src, DGNode<T> *dst) : DGEdgeBase<T, T>(src, dst) {}
    DGEdge(const DGEdge<T> &oldEdge) : DGEdgeBase<T, T>(oldEdge) {}
  };

  template <class T, class SubT>
  class DGEdgeBase {
   public:
    DGEdgeBase(DGNode<T> *src, DGNode<T> *dst)
      : from(src), to(dst), memory(false), must(false), readAfterWrite(false), writeAfterWrite(false) {}
    DGEdgeBase(const DGEdgeBase<T, SubT> &oldEdge);

    typedef typename std::vector<DGEdge<SubT> *>::iterator edges_iterator;

    edges_iterator begin_sub_edges() { return subEdges.begin(); }
    edges_iterator end_sub_edges() { return subEdges.end(); }

    std::pair<DGNode<T> *, DGNode<T> *> getNodePair() const { return std::make_pair(from, to); }
    void setNodePair(DGNode<T> *from, DGNode<T> *to) { this->from = from; this->to = to; }

    bool isMemoryDependence() const { return memory; }
    bool isMustDependence() const { return must; }
    bool isRAWDependence() const { return readAfterWrite; }

    void setMemMustRaw(bool mem, bool must, bool raw);

    void addSubEdge(DGEdge<SubT> *edge) { subEdges.push_back(edge); }

    std::string toString();
    raw_ostream &print(raw_ostream &stream);

   protected:
    DGNode<T> *from, *to;
    std::vector<DGEdge<SubT> *> subEdges;
    bool memory, must, readAfterWrite, writeAfterWrite;
  };

  /*
   * DG<T> class method implementations
   */
  template <class T>
  raw_ostream & DG<T>::print(raw_ostream &stream)
  {
    stream << "Total nodes: " << allNodes.size() << "\n";
    stream << "Internal nodes: " << internalNodeMap.size() << "\n";
    for (auto pair : internalNodePairs()) pair.second->print(errs()) << "\n";
    stream << "External nodes: " << externalNodeMap.size() << "\n";
    for (auto pair : externalNodePairs()) pair.second->print(errs()) << "\n";
    stream << "All edges: " << allEdges.size() << "\n";
    for (auto edge : allEdges) edge->print(errs()) << "\n";
  }

  template <class T>
  DGNode<T> *DG<T>::createNodeFrom(T *theT, bool inclusion)
  {
    auto &map = inclusion ? internalNodeMap : externalNodeMap;
    auto *node = new DGNode<T>(theT);
    allNodes.push_back(node);
    map[theT] = node;
    return node;
  }

  template <class T>
  DGEdge<T> *DG<T>::createEdgeFromTo(T *from, T *to)
  {
    auto fromNode = fetchNodeOf(from);
    auto toNode = fetchNodeOf(to);
    auto edge = new DGEdge<T>(fromNode, toNode);
    allEdges.push_back(edge);
    connectNodesVia(edge, fromNode, toNode);
    return edge;
  }

  template <class T>
  DGNode<T> *DG<T>::fetchOrCreateNodeOf(T *theT, bool inclusion)
  {
    auto &map = inclusion ? internalNodeMap : externalNodeMap;
    auto nodeIter = map.find(theT);
    if (nodeIter == map.end()) return createNodeFrom(theT, inclusion);
    return map[theT];
  }

  template <class T>
  DGNode<T> *DG<T>::fetchNodeOf(T *theT)
  {
    auto nodeI = internalNodeMap.find(theT);
    return (nodeI != internalNodeMap.end()) ? nodeI->second : externalNodeMap[theT];
  }

  template <class T>
  inline void DG<T>::connectNodesVia(DGEdge<T> *edge, DGNode<T> *from, DGNode<T> *to)
  {
    from->addOutgoingNode(to, edge);
    to->addIncomingNode(from, edge);
  }

  template <class T>
  void DG<T>::extractNodesFromSelfInto(DG<T> &emptyDG, std::vector<DGNode<T> *> nodesToExtract, DGNode<T> *entry)
  {
    emptyDG.entryNode = entry;
    for (auto node : nodesToExtract)
    {
      auto theT = node->getT();
      auto internal = isInternal(theT);
      errs() << "Is: " << isInternal(theT) << ", " << isExternal(theT) << "\n"; 
      auto &map = internal ? internalNodeMap : externalNodeMap;
      auto &emptyDGMap = internal ? emptyDG.internalNodeMap : emptyDG.externalNodeMap;
      
      emptyDGMap[theT] = node;
      emptyDG.allNodes.push_back(node);

      map.erase(theT);
      allNodes.erase(std::find(allNodes.begin(), allNodes.end(), node));

      auto checkToExtractNode = [&](DGNode<T> *node) -> void {
        if (emptyDG.isInGraph(node->getT())) return;
        emptyDG.allNodes.push_back(node);

        bool canRemoveFromSelfDG = true;
        for (auto outgoingNode : make_range(node->begin_outgoing_nodes(), node->end_outgoing_nodes()))
        {
          canRemoveFromSelfDG &= !isInGraph(outgoingNode->getT());
        }
        for (auto incomingNode : make_range(node->begin_incoming_nodes(), node->end_incoming_nodes()))
        {
          canRemoveFromSelfDG &= !isInGraph(incomingNode->getT());
        }

        if (canRemoveFromSelfDG)
        {
          allNodes.erase(std::find(allNodes.begin(), allNodes.end(), node));
        }
      };

      for (auto edge : make_range(node->begin_outgoing_edges(), node->end_outgoing_edges()))
      {
        emptyDG.allEdges.push_back(new DGEdge<T>(*edge));
        checkToExtractNode(edge->getNodePair().second);
      }
      for (auto edge: make_range(node->begin_incoming_edges(), node->end_incoming_edges()))
      {
        emptyDG.allEdges.push_back(new DGEdge<T>(*edge));
        checkToExtractNode(edge->getNodePair().second);
      }
    }
  }

  /*
   * DGNode<T> class method implementations
   */
  template <class T>
  std::string DGNode<T>::toString()
  {
    std::string nodeStr;
    raw_string_ostream ros(nodeStr);
    theT->print(ros);
    return nodeStr;
  }

  template <>
  inline std::string DGNode<Instruction>::toString()
  {
    if (!theT) return "Empty node\n";
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

  template <class T>
  void DGNode<T>::addIncomingNode(DGNode<T> *node, DGEdge<T> *edge)
  {
    incomingNodes.push_back(node);
    incomingEdges.push_back(edge);
  }
  
  template <class T>
  void DGNode<T>::addOutgoingNode(DGNode<T> *node, DGEdge<T> *edge)
  {
    outgoingNodes.push_back(node);
    outgoingEdges.push_back(edge);
  }
  
  template <class T>
  DGEdge<T> * DGNode<T>::getEdgeFromNodeIterator(nodes_iterator target, bool incomingEdge)
  {
    // Index of edge iterator is equivalent to index of node iterator as the node and edge vectors are aligned
    int edgeIndex = target - (incomingEdge ? incomingNodes.begin() : outgoingNodes.begin());
    return incomingEdge ? incomingEdges[edgeIndex] : outgoingEdges[edgeIndex];
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
    setMemMustRaw(oldEdge.isMemoryDependence(), oldEdge.isMustDependence(), oldEdge.isRAWDependence());
    for (auto subEdge : oldEdge.subEdges) addSubEdge(subEdge);
  }

  template <class T, class SubT>
  void DGEdgeBase<T, SubT>::setMemMustRaw(bool mem, bool must, bool raw)
  {
    this->memory = mem;
    this->must = must;
    this->readAfterWrite = raw;
    this->writeAfterWrite = !raw;
  }

  template <class T, class SubT>
  std::string DGEdgeBase<T, SubT>::toString()
  {
    std::string edgeStr;
    raw_string_ostream ros(edgeStr);
    ros << (readAfterWrite ? "RAW " : (writeAfterWrite ? "WAW " : ""));
    ros << (must ? "(must) " : "(may) ");
    ros << (memory ? "from memory " : "") << "\n";
    return edgeStr;
  }
  
  template <class T, class SubT>
  raw_ostream & DGEdgeBase<T, SubT>::print(raw_ostream &stream)
  {
    from->print(stream << "From:\t") << "\n";
    to->print(stream << "To:\t");
    return stream;
  }
}