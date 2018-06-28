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

void SCCDAGPartitions::addPartition (SCC * scc) {
	std::set<SCC *> sccs = { scc };
	this->addPartition(sccs);
}

void SCCDAGPartitions::addPartition (std::set<SCC *> &sccs) {
	auto partition = std::make_unique<SCCDAGPartition>(sccs);
	this->managePartitionInfo(partition.get());
	this->partitions.insert(std::move(partition));
}

void SCCDAGPartitions::removePartition (SCCDAGPartition *partition) {
	for (auto &p : this->partitions) {
		if (p.get() == partition) {
			this->partitions.erase(p);
			return;
		}
	}
}

void SCCDAGPartitions::mergePartitions (SCCDAGPartition *partitionA, SCCDAGPartition *partitionB) {
	auto partition = std::make_unique<SCCDAGPartition>(partitionA, partitionB);
	this->managePartitionInfo(partition.get());
	this->partitions.insert(std::move(partition));

	this->removePartition(partitionA);
	this->removePartition(partitionB);
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