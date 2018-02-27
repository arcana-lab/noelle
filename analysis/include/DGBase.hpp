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
  template <class T> class DGEdge;
  
  template <class T>
  class DG {
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

      DGNode<T> *getEntryNode() {
        return entryNode;
      }
      
      bool isInternal(T *theT) {
        return internalNodeMap.find(theT) != internalNodeMap.end(); 
      }

      bool isInternalNode(DGNode<T> *node) {
        return isInternal(node->getNode());
      }

      bool isExternal(T *theT) {
        return externalNodeMap.find(theT) != externalNodeMap.end();
      }

      bool isExternalNode(DGNode<T> *node) {
        return isExternal(node->getNode());
      }

      unsigned numInternalNodes() {
        return std::distance(internalNodeMap.begin(), internalNodeMap.end());
      }

      unsigned numExternalNodes() {
        return std::distance(externalNodeMap.begin(), externalNodeMap.end());
      }

      /*
       * Instruction Node Pair Iterator
       */

      iterator_range<node_map_iterator> internalNodePairs() {
        return make_range(internalNodeMap.begin(), internalNodeMap.end());
      }

      iterator_range<node_map_iterator> externalNodePairs() {
        return make_range(externalNodeMap.begin(), externalNodeMap.end());
      }

      /*
       * Fetching/Creating Nodes and Edges
       */
      DGNode<T> *createNodeFrom(T *theT, bool inclusion) {
        auto &map = inclusion ? internalNodeMap : externalNodeMap;
        auto *node = new DGNode<T>(theT);
        allNodes.push_back(node);
        map[theT] = node;
        return node;
      }

      DGEdge<T> *createEdgeFromTo(T *from, T *to) {
        auto fromNode = fetchNodeOf(from);
        auto toNode = fetchNodeOf(to);
        auto edge = new DGEdge<T>(fromNode, toNode);
        allEdges.push_back(edge);
        connectNodesVia(edge, fromNode, toNode);
        return edge;
      }

      DGNode<T> *fetchOrCreateNodeOf(T *theT, bool inclusion) {
        auto &map = inclusion ? internalNodeMap : externalNodeMap;
        auto nodeIter = map.find(theT);
        if (nodeIter == map.end()) return createNodeFrom(theT, inclusion);
        return map[theT];
      }

    protected:
      /*
       * Fetches node assuming it already exists for the T 
       */
      DGNode<T> *fetchNodeOf(T *theT) {
        auto nodeI = internalNodeMap.find(theT);
        return (nodeI != internalNodeMap.end()) ? nodeI->second : externalNodeMap[theT];
      }

      inline void connectNodesVia(DGEdge<T> *edge, DGNode<T> *from, DGNode<T> *to) {
        from->addOutgoingNode(to, edge);
        to->addIncomingNode(from, edge);
      }

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
      DGNode() : theNode(nullptr) {}
      DGNode(T *node) : theNode(node) {}

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

      T *getNode() const { return theNode; }

      std::string toString() {
        std::string nodeStr;
        raw_string_ostream ros(nodeStr);
        theNode->print(ros);
        return nodeStr;
      }

      void print(raw_ostream &stream) { 
        theNode->print(stream);
        stream << "\n";
      }

      void addIncomingNode(DGNode<T> *node, DGEdge<T> *edge) {
        incomingNodes.push_back(node);
        incomingEdges.push_back(edge);
      }
      
      void addOutgoingNode(DGNode<T> *node, DGEdge<T> *edge) {
        outgoingNodes.push_back(node);
        outgoingEdges.push_back(edge);
      }
      
      DGEdge<T> *getEdgeFromNodeIterator(nodes_iterator target, bool incomingEdge = false) {
        // Index of edge iterator is equivalent to index of node iterator as the node and edge vectors are aligned
        int edgeIndex = target - (incomingEdge ? incomingNodes.begin() : outgoingNodes.begin());
        return incomingEdge ? incomingEdges[edgeIndex] : outgoingEdges[edgeIndex];
      }

    protected:
      T *theNode;
      // To iterate connected nodes directly by node 
      std::vector<DGNode<T> *> outgoingNodes;
      std::vector<DGNode<T> *> incomingNodes;
      // To iterate connected nodes indirectly by edge 
      std::vector<DGEdge<T> *> outgoingEdges;
      std::vector<DGEdge<T> *> incomingEdges;
  };

  template <class T>
  class DGEdge {
   public:
    DGEdge(DGNode<T> *src, DGNode<T> *dst)
      : from(src), to(dst), memory(false), must(false), readAfterWrite(false), writeAfterWrite(false) {}
    
    DGEdge(const DGEdge<T> &oldEdge) {
      auto nodePair = oldEdge.getNodePair();
      from = nodePair.first;
      to = nodePair.second;
      setMemMustRaw(oldEdge.isMemoryDependence(), oldEdge.isMustDependence(), oldEdge.isRAWDependence());
    }

    std::pair<DGNode<T> *, DGNode<T> *> getNodePair() const {
      return std::make_pair(from, to);
    }
    
    void setNodePair(DGNode<T> *from, DGNode<T> *to) {
      this->from = from; this->to = to;
    }

    bool isMemoryDependence() const { return memory; }
    bool isMustDependence() const { return must; }
    bool isRAWDependence() const { return readAfterWrite; }

    void setMemMustRaw(bool mem, bool must, bool raw) {
      this->memory = mem;
      this->must = must;
      this->readAfterWrite = raw;
      this->writeAfterWrite = !raw;
    }

    std::string toString() {
      std::string edgeStr;
      raw_string_ostream ros(edgeStr);
      ros << (readAfterWrite ? "RAW " : (writeAfterWrite ? "WAW " : ""));
      ros << (must ? "(must) " : "(may) ");
      ros << (memory ? "from memory " : "") << "\n";
      return edgeStr;
    }
    
    void print(raw_ostream &stream) {
      from->print(stream << "From:\n");
      to->print(stream << "To:\n");
      stream << "\n";
    }

   protected:
    DGNode<T> *from, *to;
    bool memory, must, readAfterWrite, writeAfterWrite;
  };

  template <>
  inline std::string DGNode<Instruction>::toString() {
    if (!theNode)
      return "Empty node\n";
    std::string str;
    raw_string_ostream instStream(str);
    theNode->print(instStream << theNode->getFunction()->getName() << ": ");
    return str;
  }
}