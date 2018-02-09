#pragma once

#include "PDGBase.hpp"
#include "PDG.hpp"
#include "PDGPrintState.hpp"

using namespace llvm;

namespace llvm {
  // Not sure if this is still needed, or if it can be completely replaced by DOTGraphTraits<PDG*> below
  template<> struct DOTGraphTraits<PDGNodeBase<Instruction>*> : public DefaultDOTGraphTraits {
    explicit DOTGraphTraits(bool isSimple=false) : DefaultDOTGraphTraits(isSimple) {}
    
    std::string getNodeLabel(PDGNodeBase<Instruction> *node, PDGNodeBase<Instruction> *entry) {
      return node->toString();
    }
  };

  template<>
  struct DOTGraphTraits<PDG*> : public DOTGraphTraits<PDGNodeBase<Instruction>*> {
    DOTGraphTraits (bool isSimple=false) : DOTGraphTraits<PDGNodeBase<Instruction>*>(isSimple) {}

    static std::string getGraphName(PDG *pdg) {
      return "Program Dependence Graph";
    }

    std::string getNodeLabel(PDGNodeBase<Instruction> *node, PDG *pdg) {
      return DOTGraphTraits<PDGNodeBase<Instruction>*>::getNodeLabel(node, pdg->getEntryNode());
    }

    std::string getEdgeSourceLabel(PDGNodeBase<Instruction> *node, std::vector<PDGNodeBase<Instruction> *>::iterator nodeIter) {
      return node->getEdgeFromNodeIterator(nodeIter)->toString();
    }

    static std::string getEdgeAttributes(PDGNodeBase<Instruction> *node, std::vector<PDGNodeBase<Instruction> *>::iterator nodeIter, const PDG *Graph) {
      PDGEdge *edge = node->getEdgeFromNodeIterator(nodeIter);
      return edge->isMemoryDependence() ? "color=red" : "color=black";
    }

    bool isNodeHidden(PDGNodeBase<Instruction> *node) {
      return false;
      //return node->getPrintState()->isNodeHidden();
    }

    std::string getNodeDescription(PDGNodeBase<Instruction> *node, PDG *pdg) {
      return "";
      //return node->getPrintState()->getNodeDescription();
    }
  };

  template <> struct GraphTraits<PDG*> {
    using NodeRef = PDGNodeBase<Instruction> *;
    using ChildIteratorType = std::vector<PDGNodeBase<Instruction> *>::iterator;
    using nodes_iterator = std::vector<PDGNodeBase<Instruction> *>::iterator;

    static PDGNodeBase<Instruction> *getEntryNode(PDG *pdg) { return pdg->getEntryNode(); }

    static std::vector<PDGNodeBase<Instruction> *>::iterator nodes_begin(PDG *pdg) {
      return pdg->begin_nodes();
    }

    static std::vector<PDGNodeBase<Instruction> *>::iterator nodes_end(PDG *pdg) {
      return pdg->end_nodes();
    }

    static ChildIteratorType child_begin(NodeRef node) { 
      return node->begin_outgoing_nodes(); 
    }

    static ChildIteratorType child_end(NodeRef node) { 
      return node->end_outgoing_nodes();
    }
  };
}