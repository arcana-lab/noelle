#pragma once

#include "DGBase.hpp"
#include "SCC.hpp"
#include "SCCDG.hpp"

using namespace llvm;

namespace llvm {
  using SCCNode = DGNode<SCC<DGNode<Instruction>>>;
  using SCCEdge = DGEdge<SCC<DGNode<Instruction>>>;

  template<>
  struct DOTGraphTraits<SCCNode*> : public DefaultDOTGraphTraits {
    explicit DOTGraphTraits(bool isSimple=false) : DefaultDOTGraphTraits(isSimple) {}
    
    std::string getNodeLabel(SCCNode *node, SCCNode *entry) {
      return node->toString();
    }
  };

  template<>
  struct DOTGraphTraits<SCCDG*>  : public DOTGraphTraits<SCCNode*> {
    DOTGraphTraits (bool isSimple=false) : DOTGraphTraits<SCCNode*>(isSimple) {}

    static std::string getGraphName(SCCDG *dg) {
      return "Program Dependence Graph";
    }

    std::string getNodeLabel(SCCNode *node, SCCDG *dg) {
      return DOTGraphTraits<SCCNode*>::getNodeLabel(node, dg->getEntryNode());
    }

    std::string getEdgeSourceLabel(SCCNode *node, typename std::vector<SCCNode *>::iterator nodeIter) {
      return node->getEdgeFromNodeIterator(nodeIter)->toString();
    }

    static std::string getEdgeAttributes(SCCNode *node, typename std::vector<SCCNode *>::iterator nodeIter, const SCCDG *Graph) {
      SCCEdge *edge = node->getEdgeFromNodeIterator(nodeIter);
      return edge->isMemoryDependence() ? "color=red" : "color=black";
    }

    bool isNodeHidden(SCCNode *node) {
      return false;
    }

    std::string getNodeDescription(SCCNode *node, SCCDG *dg) {
      return "";
    }
  };

  template<>
  struct GraphTraits<SCCDG*> {
    using NodeRef = SCCNode *;
    using ChildIteratorType = typename std::vector<SCCNode *>::iterator;
    using nodes_iterator = typename std::vector<SCCNode *>::iterator;

    static SCCNode *getEntryNode(SCCDG *dg) { return dg->getEntryNode(); }

    static typename std::vector<SCCNode *>::iterator nodes_begin(SCCDG *dg) {
      return dg->begin_nodes();
    }

    static typename std::vector<SCCNode *>::iterator nodes_end(SCCDG *dg) {
      return dg->end_nodes();
    }

    static ChildIteratorType child_begin(NodeRef node) { 
      return node->begin_outgoing_nodes(); 
    }

    static ChildIteratorType child_end(NodeRef node) { 
      return node->end_outgoing_nodes();
    }
  };
}