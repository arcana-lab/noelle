#include "llvm/Support/raw_ostream.h"
#include "../include/SCC.hpp"

using namespace llvm;

llvm::SCC::SCC(std::vector<DGNode<Value> *> nodes) {
	/*
	 * Arbitrarily choose entry node from all nodes
	 */
	allNodes.assign(nodes.begin(), nodes.end());
	entryNode = allNodes[0];

	/*
	 * Register nodes in node map
	 */
	for (auto node : allNodes)
	{
		internalNodeMap[node->getT()] = node;
	}

	/*
	 * Adds external nodes not encountered before to a temporary list
	 */
	std::vector<DGNode<Value> *> newNodes;
	auto addNode = [this](DGNode<Value> *node, std::vector<DGNode<Value> *> &newNodes) -> bool {
		bool newExternalNode = !isInGraph(node->getT());
		if (newExternalNode) {
			newNodes.push_back(node);
			externalNodeMap[node->getT()] = node;
			return true;
		}
		return false;
	};

	/*
	 * Add internal/external edges on this SCC's instructions 
	 * Note, all outgoing edges are added to allEdges, but only external incoming edges are added to allEdges
	 * This is to avoid duplicate additions to allEdges 
	 */
	for (auto node : allNodes) {
		for (auto edgeI = node->begin_outgoing_edges(); edgeI != node->end_outgoing_edges(); ++edgeI) {
			auto edge = *edgeI;
			auto toNode = edge->getNodePair().second;
			addNode(toNode, newNodes);
			allEdges.push_back(edge);
		}
		for (auto edgeI = node->begin_incoming_edges(); edgeI != node->end_incoming_edges(); ++edgeI) {
			auto edge = *edgeI;
			auto fromNode = edge->getNodePair().first;
			if (addNode(fromNode, newNodes)) allEdges.push_back(edge);
		}
	}

	/* 
	 * Add nodes from temporary list into main list
	 */
	for (auto node : newNodes) {
		allNodes.push_back(node);
	}
}

llvm::SCC::~SCC() {}

raw_ostream &llvm::SCC::print(raw_ostream &stream) {
	stream << "\tInternal nodes:\n";
	for (auto nodePair : internalNodePairs())
	{
		nodePair.second->print(stream << "\t") << "\n";
	}
	stream << "\tExternal nodes:\n";
	for (auto nodePair : externalNodePairs())
	{
		nodePair.second->print(stream << "\t") << "\n";
	}
	return stream;
}
