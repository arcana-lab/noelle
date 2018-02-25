#pragma once

#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace llvm {

	/*
	* Strongly Connected Component
	*/
	class SCC : public DG<Instruction> {
	public:
		SCC(std::vector<DGNode<Instruction> *> nodes) {
			/*
			 * Arbitrarily choose entry node from all nodes
			 */
			allNodes = nodes;
			entryNode = allNodes[0];

			/*
			 * Register nodes in node map
			 */
			for (auto *node : allNodes) {
				internalNodeMap[node->getNode()] = node;
			}

			/*
			 * Adds external nodes not encountered before to a temporary list
			 */
			std::vector<DGNode<Instruction> *> newNodes;
			auto addNode = [this](DGNode<Instruction> *node, std::vector<DGNode<Instruction> *> &newNodes) {
				bool newExternalNode = !isInternalNode(node) && !isExternalNode(node);
				if (newExternalNode) {
					newNodes.push_back(node);
					externalNodeMap[node->getNode()] = node;
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

		~SCC() {}
	};
}