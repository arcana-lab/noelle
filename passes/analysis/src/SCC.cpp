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
			auto outgoingT = edge->getNodePair().second->getT();
			fetchOrAddNode(outgoingT, /*internal=*/ false);
			addEdge(theT, outgoingT);
		}
		for (auto edge : node->getIncomingEdges())
		{
			auto incomingT = edge->getNodePair().first->getT();
			if (isInGraph(incomingT)) continue;
			fetchOrAddNode(incomingT, /*internal=*/ false);
			addEdge(theT, incomingT);
		}
	}
}

llvm::SCC::~SCC() {}

raw_ostream &llvm::SCC::print(raw_ostream &stream) {
	stream << "\tInternal nodes:\n";
	for (auto nodePair : internalNodePairs()) nodePair.second->print(stream << "\t") << "\n";
	stream << "\tExternal nodes:\n";
	for (auto nodePair : externalNodePairs()) nodePair.second->print(stream << "\t") << "\n";
	return stream;
}
