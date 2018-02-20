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
				nodeMap[node->getNode()] = node;
			}

			/*
			 * Add all edges contained within strongly connected component nodes
			 */
			for (auto *node : allNodes) {
				for (auto edgeI = node->begin_outgoing_edges(); edgeI != node->end_outgoing_edges(); ++edgeI) {
					auto edge = *edgeI;
					Instruction *toI = edge->getNodePair().second->getNode();
					if (nodeMap.find(toI) != nodeMap.end()) {
						allEdges.push_back(edge);
					}
				}
			}
		}

		~SCC() {}
	};
}