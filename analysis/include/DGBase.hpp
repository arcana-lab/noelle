#pragma once

#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

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

      /*
       * Node and Edge Iterators
       */
      nodes_iterator begin_nodes() { allNodes.begin(); }
      nodes_iterator end_nodes() { allNodes.end(); }
      nodes_const_iterator begin_nodes() const { allNodes.begin(); }
      nodes_const_iterator end_nodes() const { allNodes.end(); }

      nodes_iterator begin_edges() { allEdges.begin(); }
      nodes_iterator end_edges() { allEdges.end(); }
      nodes_const_iterator begin_edges() const { allEdges.begin(); }
      nodes_const_iterator end_edges() const { allEdges.end(); }

      DGNode<T> *getEntryNode() {
        return entryNode;
      }

      bool isExternalNode(DGNode<T> *node) {
        return externalNodeMap.find(node->getNode()) != externalNodeMap.end();
      }

      /*
       * Instruction Node Pair Iterator
       */
      iterator_range<typename std::map<T *, DGNode<T> *>::iterator>
      nodePairs() {
        return make_range(nodeMap.begin(), nodeMap.end());
      }

      /*
       * Creating Nodes and Edges
       */
      DGNode<T> *createNodeFrom(T *theT) {
        auto *node = new DGNode<T>(theT);
        allNodes.push_back(node);
        nodeMap[theT] = node;
      }

      DGNode<T> *createExternalNodeFrom(T *theT) {
        auto *node = new DGNode<T>(theT);
        allNodes.push_back(node);
        externalNodeMap[theT] = node;
      }

      DGEdge<T> *createEdgeFromTo(T *from, T *to) {
        auto fromNode = nodeMap[from];
        auto toNode = nodeMap[to];
        auto edge = new DGEdge<T>(fromNode, toNode);
        allEdges.push_back(edge);
        fromNode->addOutgoingNode(toNode, edge);
        toNode->addIncomingNode(fromNode, edge);
        return edge;
      }

    protected:
      std::vector<DGNode<T> *> allNodes;
      std::vector<DGEdge<T> *> allEdges;
      DGNode<T> *entryNode;
      std::map<T *, DGNode<T> *> nodeMap;
      std::map<T *, DGNode<T> *> externalNodeMap;
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
      std::string toString() { return "node"; }

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