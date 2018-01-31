#pragma once

#include "PDGBase.hpp"
#include "PDG.hpp"

using namespace llvm;

namespace llvm {
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

    /// getEdgeSourceLabel - If you want to label the edge source itself,
    /// implement this method.
    std::string getEdgeSourceLabel(PDGNodeBase<Instruction> *node, std::vector<PDGNodeBase<Instruction> *>::iterator edgeIter) {
      return node->getCorrespondingEdge(edgeIter)->toString();
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