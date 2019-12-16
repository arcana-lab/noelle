/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "llvm/Support/raw_ostream.h"
#include "../include/SCC.hpp"

using namespace llvm;

SCC::SCC(std::set<DGNode<Value> *> nodes, bool connectToExternalValues) {

	/*
	 * Arbitrarily choose entry node from all nodes
	 */
	for (auto node : nodes) {
    addNode(node->getT(), /*internal=*/ true);
  }
	entryNode = (*allNodes.begin());

	/*
	 * Add internal/external edges on this SCC's instructions 
	 * Note: to avoid edge duplication, ignore incoming edges from internal nodes (they were considered in outgoing edges)
	 */
	for (auto node : nodes)
	{
		auto theT = node->getT();
		for (auto edge : node->getOutgoingEdges())
		{
			auto incomingT = edge->getIncomingNode()->getT();
      if (!connectToExternalValues && !isInternal(incomingT)) continue ;
			fetchOrAddNode(incomingT, /*internal=*/ false);
			copyAddEdge(*edge);
		}
		for (auto edge : node->getIncomingEdges())
		{
			auto outgoingT = edge->getOutgoingNode()->getT();
			if (isInGraph(outgoingT)) continue;
			fetchOrAddNode(outgoingT, /*internal=*/ false);
			copyAddEdge(*edge);
		}
	}
}
        
int64_t SCC::numberOfInstructions (void) const {
  return this->numInternalNodes();
}

raw_ostream &SCC::print (raw_ostream &stream, std::string prefixToUse, int maxEdges) {

  /*
   * Print instructions that compose the SCC.
   */
  stream << prefixToUse << "Internal nodes: " << internalNodeMap.size() << "\n";
  for (auto nodePair : internalNodePairs()) {
    nodePair.second->print(stream << prefixToUse << "\t") << "\n";
  }

  /*
   * Print live-in and live-out values.
   */
  stream << prefixToUse << "External nodes: " << externalNodeMap.size() << "\n";
  for (auto nodePair : externalNodePairs()) {
    nodePair.second->print(stream << prefixToUse << "\t") << "\n";
  }

  /*
   * Print the dependences that cross the SCC.
   */
  stream << prefixToUse << "Edges: " << allEdges.size() << "\n";
  int edgesPrinted = 0;
  for (auto edge : allEdges) {
    if (edgesPrinted++ >= maxEdges) {
      stream << prefixToUse << "\t....\n";
      break;
    }
    edge->print(stream, prefixToUse + "\t") << "\n";
  }

  return stream;
}

raw_ostream &SCC::printMinimal (raw_ostream &stream, std::string prefixToUse) {
  stream << prefixToUse << "Internal nodes: " << internalNodeMap.size() << "\n";
  for (auto nodePair : internalNodePairs()) nodePair.second->print(stream << prefixToUse << "\t") << "\n";
  stream << prefixToUse << "External nodes: " << externalNodeMap.size() << "\n";
  for (auto nodePair : externalNodePairs()) nodePair.second->print(stream << prefixToUse << "\t") << "\n";
  stream << prefixToUse << "Edges: " << allEdges.size() << "\n";
  return stream;
}

bool SCC::hasCycle (bool ignoreControlDep) {
	std::set<DGNode<Value> *> nodesChecked;
	for (auto node : this->getNodes()) {
		if (nodesChecked.find(node) != nodesChecked.end()) continue;

		std::set<DGNode<Value> *> nodesSeen;
		std::queue<DGNode<Value> *> nodesToVisit;
		nodesChecked.insert(node);
		nodesSeen.insert(node);
		nodesToVisit.push(node);

		while (!nodesToVisit.empty()) {
			auto node = nodesToVisit.front();
			nodesToVisit.pop();
			for (auto edge : node->getOutgoingEdges()) {
        if (ignoreControlDep && edge->isControlDependence()) continue;

				auto otherNode = edge->getIncomingNode();
				if (nodesSeen.find(otherNode) != nodesSeen.end()) return true;
				if (nodesChecked.find(otherNode) != nodesChecked.end()) continue;

				nodesChecked.insert(otherNode);
				nodesSeen.insert(otherNode);
				nodesToVisit.push(otherNode);
			}
		}
	}

	return false;
}

bool SCC::iterateOverInstructions (std::function<bool (Instruction *)> funcToInvoke){

  /*
   * Iterate over the internal instructions of the SCC.
   */
  for (auto nodePair : this->internalNodePairs()){
    auto v = nodePair.first;
    auto i = cast<Instruction>(v);
    if (funcToInvoke(i)){
      return true;
    }
  }

  return false;
}

bool SCC::iterateOverAllInstructions (std::function<bool (Instruction *)> funcToInvoke){

  /*
   * Iterate over the nodes of the SCC.
   */
  for (auto node : this->getNodes()){
    auto v = node->getT();
    auto i = cast<Instruction>(v);
    if (funcToInvoke(i)){
      return true;
    }
  }

  return false;
}

SCC::~SCC() {
  return ;
}
