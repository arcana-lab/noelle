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

raw_ostream &llvm::SCC::print (raw_ostream &stream, std::string prefixToUse) {

    stream << prefixToUse << "The SCC is composed by the following " << internalNodeMap.size() << " nodes\n";
	for (auto nodePair : internalNodePairs()) {
      nodePair.second->print(stream << prefixToUse << "\t") << "\n";
    }

    stream << prefixToUse << "The SCC is connected via dependences with the following " << externalNodeMap.size() << " external nodes\n";
	for (auto nodePair : externalNodePairs()) {
      nodePair.second->print(stream <<  prefixToUse << "\t") << "\n";
    }

    stream << prefixToUse << "The SCC is connected with outside nodes with the following " << allEdges.size() << " edges\n";
    for (auto edge : allEdges) {
      edge->print(stream, prefixToUse + "\t") << "\n";
      stream << prefixToUse << "\n";
    }

	return stream;
}

raw_ostream &llvm::SCC::print (raw_ostream &stream) {
  return print(stream, "");
}
