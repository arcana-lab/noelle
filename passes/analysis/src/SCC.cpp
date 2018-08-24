#include "llvm/Support/raw_ostream.h"
#include "../include/SCC.hpp"

using namespace llvm;

llvm::SCC::SCC(std::set<DGNode<Value> *> nodes) {
	/*
	 * Arbitrarily choose entry node from all nodes
	 */
	for (auto node : nodes) addNode(node->getT(), /*internal=*/ true);
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

llvm::SCC::~SCC() {
  return ;
}

raw_ostream &llvm::SCC::print(raw_ostream &stream, std::string prefixToUse) {

    stream << prefixToUse << "Internal nodes: " << internalNodeMap.size() << "\n";
	for (auto nodePair : internalNodePairs()) nodePair.second->print(stream << prefixToUse << "\t") << "\n";
    stream << prefixToUse << "External nodes: " << externalNodeMap.size() << "\n";
	for (auto nodePair : externalNodePairs()) nodePair.second->print(stream << prefixToUse << "\t") << "\n";
    stream << prefixToUse << "Edges: " << allEdges.size() << "\n";
    for (auto edge : allEdges) edge->print(stream, prefixToUse + "\t") << "\n";
	return stream;
}

bool llvm::SCC::hasCycle (bool ignoreControlDep) {
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

/*
 * For the execution of an SCC to be associative:
 *  1) All computations in the SCC must be of the same type (add, mul)
 *  2) They must NOT reference their own current values
 */
bool llvm::SCC::executesAssociatively () {
  Instruction *prevVal = nullptr; 
  for (auto nodePair : this->internalNodePairs())  {
    Instruction *val = cast<Instruction>(nodePair.first);
    if (isa<PHINode>(val)) continue;
    if (val->isAssociative() && val->isBinaryOp()) {
      if (prevVal && val->getOpcode() != prevVal->getOpcode()) return false;
      for (auto &op : val->operands()) {
        auto opV = op.get();
        if (!this->isInternal(opV) || isa<PHINode>(opV)) continue;
        if (this->isInternal(opV)) return false;
      }
      prevVal = val;
    } else {
      return false;
    }
  }
  return true;
}

