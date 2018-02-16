#pragma once

#include "DGBase.hpp"

using namespace llvm;

namespace llvm {
  // Not sure if this is still needed, or if it can be completely replaced by DOTGraphTraits<DG*> below
  template<>
  struct DOTGraphTraits<DGNode<Instruction>*> : public DefaultDOTGraphTraits {
    explicit DOTGraphTraits(bool isSimple=false) : DefaultDOTGraphTraits(isSimple) {}
    
    std::string getNodeLabel(DGNode<Instruction> *node, DGNode<Instruction> *entry) {
      return node->toString();
    }
  };

  template<>
  struct DOTGraphTraits<PDG *> : public DOTGraphTraits<DGNode<Instruction>*> {
    DOTGraphTraits (bool isSimple=false) : DOTGraphTraits<DGNode<Instruction>*>(isSimple) {}

    static std::string getGraphName(PDG *dg) {
      return "Program Dependence Graph";
    }

    std::string getNodeLabel(DGNode<Instruction> *node, PDG *dg) {
      return DOTGraphTraits<DGNode<Instruction>*>::getNodeLabel(node, dg->getEntryNode());
    }

    std::string getEdgeSourceLabel(DGNode<Instruction> *node, typename std::vector<DGNode<Instruction> *>::iterator nodeIter) {
      return node->getEdgeFromNodeIterator(nodeIter)->toString();
    }

    static std::string getEdgeAttributes(DGNode<Instruction> *node, typename std::vector<DGNode<Instruction> *>::iterator nodeIter, const PDG *Graph) {
      DGEdge<Instruction> *edge = node->getEdgeFromNodeIterator(nodeIter);
      return edge->isMemoryDependence() ? "color=red" : "color=black";
    }

    bool isNodeHidden(DGNode<Instruction> *node) {
      return false;
    }

    std::string getNodeDescription(DGNode<Instruction> *node, PDG *dg) {
      return "";
    }
  };

  template<>
  struct GraphTraits<PDG *> {
    using NodeRef = DGNode<Instruction> *;
    using ChildIteratorType = typename std::vector<DGNode<Instruction> *>::iterator;
    using nodes_iterator = typename std::vector<DGNode<Instruction> *>::iterator;

    static DGNode<Instruction> *getEntryNode(PDG *dg) { return dg->getEntryNode(); }

    static typename std::vector<DGNode<Instruction> *>::iterator nodes_begin(PDG *dg) {
      return dg->begin_nodes();
    }

    static typename std::vector<DGNode<Instruction> *>::iterator nodes_end(PDG *dg) {
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