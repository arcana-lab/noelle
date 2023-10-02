/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "llvm/Support/raw_ostream.h"
#include "noelle/core/SCC.hpp"

namespace llvm::noelle {

SCC::SCC(std::set<DGNode<Value> *> internalNodes) {

  /*
   * Collect all internal values
   */
  std::set<Value *> internalValues;
  for (auto node : internalNodes) {
    internalValues.insert(node->getT());
  }

  /*
   * Determine nodes that are not internal
   * Compare by value and NOT node to avoid duplicate classification
   *  if the set of nodes come from 2+ graph contexts
   */
  std::set<DGNode<Value> *> externalNodes;
  for (auto node : internalNodes) {
    for (auto edge : node->getOutgoingEdges()) {
      if (internalValues.find(edge->getIncomingT()) == internalValues.end()) {
        externalNodes.insert(edge->getDstNode());
      }
    }
    for (auto edge : node->getIncomingEdges()) {
      if (internalValues.find(edge->getOutgoingT()) == internalValues.end()) {
        externalNodes.insert(edge->getSrcNode());
      }
    }
  }

  copyNodesAndEdges(internalNodes, externalNodes);
}

SCC::SCC(std::set<DGNode<Value> *> internalNodes,
         std::set<DGNode<Value> *> externalNodes) {
  copyNodesAndEdges(internalNodes, externalNodes);
}

void SCC::copyNodesAndEdges(std::set<DGNode<Value> *> internalNodes,
                            std::set<DGNode<Value> *> externalNodes) {

  /*
   * Add all nodes by classification. Arbitrarily choose entry node from all
   * nodes
   */
  for (auto node : internalNodes) {
    addNode(node->getT(), /*internal=*/true);
  }
  for (auto node : externalNodes) {
    addNode(node->getT(), /*internal=*/false);
  }
  entryNode = (*allNodes.begin());

  /*
   * Add internal edges on this SCC's instructions
   * Note: to avoid edge duplication, ignore incoming edges from internal nodes
   * (they were considered in outgoing edges)
   */
  for (auto node : internalNodes) {
    for (auto edge : node->getOutgoingEdges()) {
      auto incomingT = edge->getIncomingT();
      if (isExternal(incomingT))
        continue;
      copyAddEdge(*edge);
    }
  }

  /*
   * Add external edges on this SCC's instructions
   */
  for (auto node : internalNodes) {
    for (auto edge : node->getOutgoingEdges()) {
      auto incomingT = edge->getDstNode()->getT();
      if (isInternal(incomingT))
        continue;
      copyAddEdge(*edge);
    }
    for (auto edge : node->getIncomingEdges()) {
      auto outgoingT = edge->getSrcNode()->getT();
      if (isInternal(outgoingT))
        continue;
      copyAddEdge(*edge);
    }
  }
}

int64_t SCC::numberOfInstructions(void) const {
  return this->numInternalNodes();
}

raw_ostream &SCC::print(raw_ostream &stream,
                        std::string prefixToUse,
                        int maxEdges) {

  /*
   * Print instructions that compose the SCC.
   */
  stream << prefixToUse << "Nodes within the SCC: " << internalNodeMap.size()
         << "\n";
  for (auto nodePair : internalNodePairs()) {
    nodePair.second->print(stream << prefixToUse << "\t") << "\n";
  }

  /*
   * Print live-in and live-out values.
   */
  stream
      << prefixToUse
      << "Nodes outside the SCC and connected with at least one node within the SCC: "
      << externalNodeMap.size() << "\n";
  std::vector<DGNode<Value> *> externalNodesOfCurrentSCC{};
  for (auto nodePair : externalNodePairs()) {
    externalNodesOfCurrentSCC.push_back(nodePair.second);
  }
  auto sortingFunction = [](DGNode<Value> *n0, DGNode<Value> *n1) {
    auto v0 = n0->getT();
    auto v1 = n1->getT();
    if (v0 < v1) {
      return true;
    }
    return false;
  };
  std::sort(externalNodesOfCurrentSCC.begin(),
            externalNodesOfCurrentSCC.end(),
            sortingFunction);
  for (auto extNode : externalNodesOfCurrentSCC) {
    extNode->print(stream << prefixToUse << "\t") << "\n";
  }

  /*
   * Print the dependences that cross the SCC.
   */
  auto sortedDeps = DG::sortDependences(allEdges);
  stream << prefixToUse << "Edges: " << sortedDeps.size() << "\n";
  auto edgesPrinted = 0;
  for (auto edge : sortedDeps) {
    if (edgesPrinted++ >= maxEdges) {
      stream << prefixToUse << "  ....\n";
      break;
    }
    edge->print(stream, prefixToUse + "  ") << "\n";
  }

  return stream;
}

raw_ostream &SCC::printMinimal(raw_ostream &stream, std::string prefixToUse) {
  stream << prefixToUse << "Internal nodes: " << internalNodeMap.size() << "\n";
  for (auto nodePair : internalNodePairs())
    nodePair.second->print(stream << prefixToUse << "\t") << "\n";
  stream << prefixToUse << "External nodes: " << externalNodeMap.size() << "\n";
  for (auto nodePair : externalNodePairs())
    nodePair.second->print(stream << prefixToUse << "\t") << "\n";
  stream << prefixToUse << "Edges: " << allEdges.size() << "\n";
  return stream;
}

bool SCC::hasCycle(bool ignoreControlDep) {
  std::set<DGNode<Value> *> nodesChecked;
  for (auto nodePair : this->internalNodePairs()) {
    auto node = nodePair.second;
    if (nodesChecked.find(node) != nodesChecked.end())
      continue;

    std::set<DGNode<Value> *> nodesSeen;
    std::queue<DGNode<Value> *> nodesToVisit;
    nodesChecked.insert(node);
    nodesSeen.insert(node);
    nodesToVisit.push(node);

    while (!nodesToVisit.empty()) {
      auto node = nodesToVisit.front();
      nodesToVisit.pop();
      for (auto edge : node->getOutgoingEdges()) {
        if (ignoreControlDep && edge->isControlDependence())
          continue;

        auto otherNode = edge->getDstNode();
        if (nodesSeen.find(otherNode) != nodesSeen.end())
          return true;
        if (nodesChecked.find(otherNode) != nodesChecked.end())
          continue;

        nodesChecked.insert(otherNode);
        nodesSeen.insert(otherNode);
        nodesToVisit.push(otherNode);
      }
    }
  }

  return false;
}

bool SCC::iterateOverInstructions(
    std::function<bool(Instruction *)> funcToInvoke) {

  /*
   * Iterate over the internal instructions of the SCC.
   */
  for (auto nodePair : this->internalNodePairs()) {
    auto v = nodePair.first;
    if (auto i = dyn_cast<Instruction>(v)) {
      if (funcToInvoke(i)) {
        return true;
      }
    }
  }

  return false;
}

std::set<Instruction *> SCC::getInstructions(void) {
  std::set<Instruction *> s{};

  auto f = [&s](Instruction *i) -> bool {
    s.insert(i);
    return false;
  };

  return s;
}

bool SCC::iterateOverAllInstructions(
    std::function<bool(Instruction *)> funcToInvoke) {

  /*
   * Iterate over the nodes of the SCC.
   */
  for (auto node : this->getNodes()) {
    auto v = node->getT();
    if (auto i = dyn_cast<Instruction>(v)) {
      if (funcToInvoke(i)) {
        return true;
      }
    }
  }

  return false;
}

bool SCC::iterateOverValues(std::function<bool(Value *)> funcToInvoke) {

  /*
   * Iterate over the internal instructions of the SCC.
   */
  for (auto nodePair : this->internalNodePairs()) {
    auto v = nodePair.first;
    if (funcToInvoke(v)) {
      return true;
    }
  }

  return false;
}

bool SCC::iterateOverAllValues(std::function<bool(Value *)> funcToInvoke) {

  /*
   * Iterate over the nodes of the SCC.
   */
  for (auto node : this->getNodes()) {
    auto v = node->getT();
    if (funcToInvoke(v)) {
      return true;
    }
  }

  return false;
}

SCC::~SCC() {
  return;
}

} // namespace llvm::noelle
