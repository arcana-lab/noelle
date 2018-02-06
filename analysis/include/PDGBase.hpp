#pragma once

#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace llvm;

namespace llvm {
  /*
   * Program Dependence Graph Node and Edge
   */

  class PDGNodePrintState;
  class PDGEdge;
  
  // Template PDG node to abstract node type
  template <class NodeT> 
  class PDGNodeBase {
    public:
      PDGNodeBase() { theNode = NULL; }
      PDGNodeBase(NodeT *node) { theNode = node; }

      typedef typename std::vector<PDGNodeBase *>::iterator nodes_iterator;
      typedef typename std::vector<PDGEdge *>::iterator edges_iterator;

      nodes_iterator begin_outgoing_nodes() { return outgoingNodes.begin(); }
      nodes_iterator end_outgoing_nodes() { return outgoingNodes.end(); }
      nodes_iterator begin_incoming_nodes() { return incomingNodes.begin(); }
      nodes_iterator end_incoming_nodes() { return incomingNodes.end(); }

      edges_iterator begin_outgoing_edges() { return outgoingEdges.begin(); }
      edges_iterator end_outgoing_edges() { return outgoingEdges.end(); }
      edges_iterator begin_incoming_edges() { return incomingEdges.begin(); }
      edges_iterator end_incoming_edges() { return incomingEdges.end(); }

      NodeT *getNode() const { return theNode; }
      std::string toString() { return "node"; }

      void addIncomingNode(PDGNodeBase *node, PDGEdge *edge) {
        incomingNodes.push_back(node);
        incomingEdges.push_back(edge);
      }
      
      void addOutgoingNode(PDGNodeBase *node, PDGEdge *edge) {
        outgoingNodes.push_back(node);
        outgoingEdges.push_back(edge);
      }
      
      PDGEdge *getEdgeFromNodeIterator(nodes_iterator target, bool incomingEdge = false) {
        // Index of edge iterator is equivalent to index of node iterator as the node and edge vectors are aligned
        int edgeIndex = target - (incomingEdge ? incomingNodes.begin() : outgoingNodes.begin());
        return incomingEdge ? incomingEdges[edgeIndex] : outgoingEdges[edgeIndex];
      }

      void addPrintState(PDGNodePrintState *state) {
        printState = state;
      }

      PDGNodePrintState *getPrintState() { return printState; }

    private:
      NodeT *theNode;
      // To iterate connected nodes directly by node 
      std::vector<PDGNodeBase *> outgoingNodes;
      std::vector<PDGNodeBase *> incomingNodes;
      // To iterate connected nodes indirectly by edge 
      std::vector<PDGEdge *> outgoingEdges;
      std::vector<PDGEdge *> incomingEdges;
      // To represent this node's DOT printing state
      PDGNodePrintState *printState;
  };

  template <>
  inline std::string PDGNodeBase<Instruction>::toString() {
    if (!theNode) {
      return "Empty node\n";
    }
    std::string str;
    raw_string_ostream ros(str);
    theNode->print(ros);
    return str;
  }

  class PDGEdge {
   public:
    PDGEdge(PDGNodeBase<Instruction> *src, PDGNodeBase<Instruction> *dst) { from = src; to = dst; }

    std::string toString() { return "edge"; }
    
   private:
    PDGNodeBase<Instruction> *from, *to;
  };

}