/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "DGBase.hpp"
#include "SCC.hpp"
#include "PDG.hpp"
#include "SCCDAG.hpp"

using namespace llvm;

namespace llvm {
  /*
   * ElementTraits<NodeType>: Dependence Graph's node/edge level base traits
   */
  template <class T>
  struct ElementTraits : public DefaultDOTGraphTraits {
    explicit ElementTraits(bool isSimple=false) : DefaultDOTGraphTraits(isSimple) {}
    
    std::string getNodeLabel(DGNode<T> *node, DGNode<T> *entry) {
      return node->toString();
    }

    std::string getEdgeSourceLabel(DGNode<T> *node, typename std::vector<DGNode<T> *>::iterator nodeIter) {
      return node->getEdgeInstance(nodeIter - node->begin_outgoing_nodes())->toString();
    }
  };

  /*
   * Strongly Connected Components ElementTraits specialization
   */
  template <>
  struct ElementTraits<SCC> : public DefaultDOTGraphTraits {
    explicit ElementTraits(bool isSimple=false) : DefaultDOTGraphTraits(isSimple) {}

    std::string getNodeLabel(DGNode<SCC> *node, DGNode<SCC> *entry) {
      std::string nodeStr;
      raw_string_ostream ros(nodeStr);
      for (auto nodePair : node->getT()->internalNodePairs()) {
        nodePair.first->print(ros);
        ros << "\n";
      }
      return ros.str();
    }

    std::string getEdgeSourceLabel(DGNode<SCC> *node, typename std::vector<DGNode<SCC> *>::iterator nodeIter) {
      std::string edgeStr;
      raw_string_ostream ros(edgeStr);
      DGEdge<SCC> *edgesBetweenSCC = node->getEdgeInstance(nodeIter - node->begin_outgoing_nodes());
      for (DGEdge<Value> *edge : edgesBetweenSCC->getSubEdges()) {
        edge->getOutgoingT()->printAsOperand(ros);
        edge->getIncomingT()->printAsOperand(ros << " -> ");
        ros << "  ";
      }
      return ros.str();
    }
  };

  /*
   * DGDOTGraphTraits<GraphType, NodeType>: Dependence Graph's base traits
   */
  template <class DG, class T>
  struct DGDOTGraphTraits : public ElementTraits<T> {
    DGDOTGraphTraits (bool isSimple=false) : ElementTraits<T>(isSimple) {}

    std::string getNodeLabel(DGNode<T> *node, DG *dg) {
      return ElementTraits<T>::getNodeLabel(node, dg->getEntryNode());
    }

    static std::string getNodeAttributes(DGNode<T> *node, DG *dg) {
      std::string color = dg->isExternal(node->getT()) ? "color=gray" : "color=black";

      std::string subgraph = "printercluster=";
      if (dg->isExternal(node->getT())) {
        bool isIncoming = node->numOutgoingEdges() > 0;
        subgraph += isIncoming ? "incomingExternal" : "outgoingExternal";
      } else {
        subgraph += "internal";
      }

      return color + "," + subgraph;
    }

    static std::string getEdgeAttributes(DGNode<T> *node, typename std::vector<DGNode<T> *>::iterator nodeIter, DG *dg) {
      const std::string cntColor = "color=blue";
      const std::string memColor = "color=red";
      const std::string varColor = "color=black";
      std::string attrsStr;
      raw_string_ostream ros(attrsStr);
      auto edge = node->getEdgeInstance(nodeIter - node->begin_outgoing_nodes());
      ros << (edge->isControlDependence() ? cntColor : (edge->isMemoryDependence() ? memColor : varColor));
      if (dg->isExternal(edge->getOutgoingT()) || dg->isExternal(edge->getIncomingT())) ros << ",style=dotted";
      return ros.str();
    }
  };
  
  /*
   * Program Dependence Graph DOTGraphTraits specialization
   */
  template<>
  struct DOTGraphTraits<PDG *> : DGDOTGraphTraits<PDG, Value> {
    DOTGraphTraits (bool isSimple=false) : DGDOTGraphTraits<PDG, Value>(isSimple) {}

    static std::string getGraphName(PDG *dg) {
      return "Program Dependence Graph";
    }
  };

  /*
   * Strongly Connected Component DOTGraphTraits specialization
   */
  template<>
  struct DOTGraphTraits<SCC *> : DGDOTGraphTraits<SCC, Value> {
    DOTGraphTraits (bool isSimple=false) : DGDOTGraphTraits<SCC, Value>(isSimple) {}

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
    using nodes_iterator = typename std::set<DGNode<T> *>::iterator;

    static DGNode<T> *getEntryNode(DG *dg) { return dg->getEntryNode(); }

    static nodes_iterator nodes_begin(DG *dg) {
      return dg->begin_nodes();
    }

    static nodes_iterator nodes_end(DG *dg) {
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
   * GraphTraits specializations for PDG, SCC, and SCCDAG
   */
  template<> struct GraphTraits<PDG *> : DGGraphTraits<PDG, Value> {};
  template<> struct GraphTraits<SCC *> : DGGraphTraits<SCC, Value> {};
  template<> struct GraphTraits<SCCDAG *> : DGGraphTraits<SCCDAG, SCC> {};
}
