#pragma once

#include "DGBase.hpp"
#include "SCC.hpp"
#include "PDG.hpp"
#include "SCCDAG.hpp"

using namespace llvm;

namespace llvm {
  /*
   * DGDOTNodeTraits<NodeType>: Dependence Graph's node level base traits
   */
  template <class T>
  struct DGDOTNodeTraits : public DefaultDOTGraphTraits {
    explicit DGDOTNodeTraits(bool isSimple=false) : DefaultDOTGraphTraits(isSimple) {}
    
    std::string getNodeLabel(DGNode<T> *node, DGNode<T> *entry) {
      return node->toString();
    }
  };

  /*
   * DGDOTGraphTraits<GraphType, NodeType>: Dependence Graph's base traits
   */
  template <class DG, class T>
  struct DGDOTGraphTraits : public DGDOTNodeTraits<T> {
    DGDOTGraphTraits (bool isSimple=false) : DGDOTNodeTraits<T>(isSimple) {}

    std::string getNodeLabel(DGNode<T> *node, DG *dg) {
      return DGDOTNodeTraits<T>::getNodeLabel(node, dg->getEntryNode());
    }

    static std::string getNodeAttributes(DGNode<T> *node, DG *dg) {
      return dg->isExternal(node->getT()) ? "color=gray" : "color=black";
    }

    std::string getEdgeSourceLabel(DGNode<T> *node, typename std::vector<DGNode<T> *>::iterator nodeIter) {
      return node->getEdgeFromNodeIterator(nodeIter)->toString();
    }

    static std::string getEdgeAttributes(DGNode<T> *node, typename std::vector<DGNode<T> *>::iterator nodeIter, DG *dg) {
      DGEdge<T> *edge = node->getEdgeFromNodeIterator(nodeIter);
      return edge->isMemoryDependence() ? "color=red" : "color=black";
    }

    bool isNodeHidden(DGNode<T> *node) {
      return false;
    }

    std::string getNodeDescription(DGNode<T> *node, DG *dg) {
      return "";
    }
  };
  
  /*
   * Program Dependence Graph DOTGraphTraits specialization
   */
  template<>
  struct DOTGraphTraits<PDG *> : DGDOTGraphTraits<PDG, Instruction> {
    DOTGraphTraits (bool isSimple=false) : DGDOTGraphTraits<PDG, Instruction>(isSimple) {}

    static std::string getGraphName(PDG *dg) {
      return "Program Dependence Graph";
    }
  };

  /*
   * Strongly Connected Component DOTGraphTraits specialization
   */
  template<>
  struct DOTGraphTraits<SCC *> : DGDOTGraphTraits<SCC, Instruction> {
    DOTGraphTraits (bool isSimple=false) : DGDOTGraphTraits<SCC, Instruction>(isSimple) {}

    static std::string getGraphName(SCC *dg) {
      return "Strongly Connected Component";
    }
  };

  /*
   * Strongly Connected Components Graph DOTGraphTraits specialization
   */
  template<>
  struct DOTGraphTraits<SCCDAG *>  : DGDOTGraphTraits<SCCDAG, SCC> {
    DOTGraphTraits (bool isSimple=false) : DGDOTGraphTraits<SCCDAG, SCC>(isSimple) {}

    static std::string getGraphName(SCCDAG *dg) {
      return "Strongly Connected Component Graph";
    }
  };

  /*
   * DGGraphTraits<GraphType, NodeType>: Dependence Graph's node iteration traits
   */
  template <class DG, class T>
  struct DGGraphTraits {
    using NodeRef = DGNode<T> *;
    using ChildIteratorType = typename std::vector<DGNode<T> *>::iterator;
    using nodes_iterator = typename std::vector<DGNode<T> *>::iterator;

    static DGNode<T> *getEntryNode(DG *dg) { return dg->getEntryNode(); }

    static typename std::vector<DGNode<T> *>::iterator nodes_begin(DG *dg) {
      return dg->begin_nodes();
    }

    static typename std::vector<DGNode<T> *>::iterator nodes_end(DG *dg) {
      return dg->end_nodes();
    }

    static ChildIteratorType child_begin(NodeRef node) { 
      return node->begin_outgoing_nodes(); 
    }

    static ChildIteratorType child_end(NodeRef node) { 
      return node->end_outgoing_nodes();
    }
  };

  /*
   * DGGraphTraits specializations for PDG, SCC, and SCCDAG
   */
  template<> struct GraphTraits<PDG *> : DGGraphTraits<PDG, Instruction> {};
  template<> struct GraphTraits<SCC *> : DGGraphTraits<SCC, Instruction> {};
  template<> struct GraphTraits<SCCDAG *> : DGGraphTraits<SCCDAG, SCC> {};
}
