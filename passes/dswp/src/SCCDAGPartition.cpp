#include "SCCDAGPartition.hpp"

using namespace llvm;

SCCDAGPartition::SCCDAGPartition (std::set<SCC *> &sccs) : SCCs{sccs} {
	
	/*
	 * Compute partition cost using heuristic: # instructions
	 */
	this->cost = 0;
	for (auto scc : sccs) {
		for (auto nodePair : scc->internalNodePairs()) {
			if (auto call = dyn_cast<CallInst>(nodePair.first)) {
				this->cost += 100;
			} else {
				this->cost++;
			}
		}
	}
}

SCCDAGPartition::SCCDAGPartition (SCCDAGPartition *partA, SCCDAGPartition *partB) {
	for (auto scc : partA->SCCs) this->SCCs.insert(scc);
	for (auto scc : partB->SCCs) this->SCCs.insert(scc);
	this->cost = partA->cost + partB->cost;
}

SCCDAGPartition *SCCDAGPartitions::addPartition (SCC * scc) {
	std::set<SCC *> sccs = { scc };
	return this->addPartition(sccs);
}

SCCDAGPartition *SCCDAGPartitions::addPartition (std::set<SCC *> &sccs) {
	auto partition = std::make_unique<SCCDAGPartition>(sccs);
	this->managePartitionInfo(partition.get());
	this->totalCost += partition->cost;
	return this->partitions.insert(std::move(partition)).first->get();
}

void SCCDAGPartitions::removePartition (SCCDAGPartition *partition) {
	for (auto &p : this->partitions) {
		if (p.get() == partition) {
			this->partitions.erase(p);
			return;
		}
	}
}

SCCDAGPartition *SCCDAGPartitions::mergePartitions (SCCDAGPartition *partitionA, SCCDAGPartition *partitionB) {
	auto partition = std::make_unique<SCCDAGPartition>(partitionA, partitionB);
	this->managePartitionInfo(partition.get());
	auto newPartition = this->partitions.insert(std::move(partition)).first->get();

	this->removePartition(partitionA);
	this->removePartition(partitionB);
	return newPartition;
}

void SCCDAGPartitions::managePartitionInfo (SCCDAGPartition *partition) {
	for (auto scc : partition->SCCs) this->fromSCCToPartition[scc] = partition;
}

SCCDAGPartition *SCCDAGPartitions::partitionOf (SCC *scc) {
	auto iter = this->fromSCCToPartition.find(scc);
	return (iter == this->fromSCCToPartition.end() ? nullptr : iter->second);
}

bool SCCDAGPartitions::isRemovable (SCC *scc) {
	return (this->removableNodes.find(scc) != this->removableNodes.end());
}

int SCCDAGPartitions::numEdgesBetween (SCCDAGPartition *partitionA, SCCDAGPartition *partitionB) {
	int edgeCount = 0;
	for (auto scc : partitionA->SCCs) {
		for (auto edge : sccDAG->fetchNode(scc)->getOutgoingEdges()) {
			if (partitionB->SCCs.find(edge->getIncomingT()) != partitionB->SCCs.end()) {
				edgeCount++;
			}
		}
	}
	return edgeCount;
}

std::set<SCCDAGPartition *> SCCDAGPartitions::descendantsOf (SCCDAGPartition *partition) {
	std::set<SCCDAGPartition *> descendants;
	for (auto scc : partition->SCCs) {
		for (auto edge : sccDAG->fetchNode(scc)->getOutgoingEdges()) {
			auto childPart = this->partitionOf(edge->getIncomingT());
			if (childPart != nullptr && childPart != partition) descendants.insert(childPart);
		}
	}
	return descendants;
}