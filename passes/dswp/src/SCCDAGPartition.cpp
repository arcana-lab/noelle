#include "SCCDAGPartition.hpp"

using namespace llvm;

void SCCDAGPartitions::addPartition (SCC * scc) {
	std::set<SCC *> sccs = { scc };
	auto partition = std::make_unique<SCCDAGPartition>(sccs);
	this->fromSCCToPartition[scc] = partition.get();
	this->partitions.insert(std::move(partition));
}

void SCCDAGPartitions::addPartition (std::set<SCC *> &sccs) {
	auto partition = std::make_unique<SCCDAGPartition>(sccs);
	auto partitionPtr = partition.get();
	for (auto scc : partition->SCCs) {
		this->fromSCCToPartition[scc] = partitionPtr;
	}
	this->partitions.insert(std::move(partition));
}

SCCDAGPartition *SCCDAGPartitions::partitionOf (SCC *scc) {
	auto iter = this->fromSCCToPartition.find(scc);
	return (iter == this->fromSCCToPartition.end() ? nullptr : iter->second);
}

bool SCCDAGPartitions::isRemovable (SCC *scc) {
	return (this->removableNodes.find(scc) != this->removableNodes.end());
}