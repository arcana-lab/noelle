#pragma once

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
  };

  template <> struct GraphTraits<PDG*> {
    using NodeRef = PDGNodeBase<Instruction> *;
    using ChildIteratorType = std::vector<PDGNodeBase<Instruction> *>::iterator;
    using nodes_iterator = std::vector<PDGNodeBase<Instruction> *>::iterator;

    static PDGNodeBase<Instruction> *getEntryNode(PDG *pdg) { return pdg->getEntryNode(); }

    static std::vector<PDGNodeBase<Instruction> *>::iterator nodes_begin(PDG *pdg) {
      errs() << "Invoked nodes begin on PDG\n";
      return pdg->begin_nodes();
    }

    static std::vector<PDGNodeBase<Instruction> *>::iterator nodes_end(PDG *pdg) {
      errs() << "Invoked nodes end on PDG\n";
      return pdg->end_nodes();
    }

    // GraphTraits requires a child iterator, but for now I'm just using the PDG iterator
    // TODO: Create an iterator on PDGNodeBase<Instruction> to use here instead of this:
    static ChildIteratorType child_begin(NodeRef node) { return node->begin_nodes(); }
    static ChildIteratorType child_end(NodeRef node) { return node->end_nodes(); }
  };

  /*
  // Template PDG graph traits for use in graph node iteration
  template <class Node, class ChildIterator> struct PDGGraphTraitsBase {
    using nodes_iterator = df_iterator<Node*, df_iterator_default_set<Node*>>;
    using NodeRef = Node *;
    using ChildIteratorType = ChildIterator;

    static NodeRef getEntryNode(NodeRef N) { return N; }
    static ChildIteratorType child_begin(NodeRef N) { return N->begin(); }
    static ChildIteratorType child_end(NodeRef N) { return N->end(); }

    static nodes_iterator nodes_begin(NodeRef N) {
      return df_begin(getEntryNode(N));
    }
    static nodes_iterator nodes_end(NodeRef N) { return df_end(getEntryNode(N)); }
  };

  template <> struct GraphTraits<PDGNode *>
    : public PDGGraphTraitsBase<PDGNode, PDGNode::iterator> {};

  template <> struct GraphTraits<const PDGNode *>
    : public PDGGraphTraitsBase<const PDGNode, PDGNode::const_iterator> {};
  */
}