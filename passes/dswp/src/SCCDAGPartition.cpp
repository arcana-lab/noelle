#include "SCCDAGPartition.hpp"

using namespace llvm;

SCCDAGPartition::SCCDAGPartition (SCCDAGInfo *sccdagInfo, LoopInfoSummary *loopInfo, std::set<SCC *> &sccs)
    : SCCs{sccs}, cost{0}, hasLoopCarriedDep{true} {

    /*
     * Collect all potentially fully-contained loops in the partition
     */
    std::unordered_map<LoopSummary *, std::set<BasicBlock *>> loopToBBContainedMap;
    for (auto scc : sccs) {
        for (auto bb : sccdagInfo->sccToInfo[scc]->bbs) {
            loopToBBContainedMap[loopInfo->bbToLoop[bb]].insert(bb);
        }
    }

    /*
     * Determine which loops are fully contained
     */
    for (auto loopBBs : loopToBBContainedMap) {
        bool fullyContained = true;
        for (auto bb : loopBBs.first->bbs) {
            fullyContained &= loopBBs.second.find(bb) != loopBBs.second.end();
        }
        if (fullyContained) this->loopsContained.insert(loopBBs.first);
    }

    /*
     * Collect total partition cost (TODO: Use info on contained loops to partially determine total cost)
     * Determine whether partition is DOALL or SEQuential
     */
    for (auto scc : sccs) {
        auto &sccInfo = sccdagInfo->sccToInfo[scc];
        this->cost += sccInfo->cost;
        this->hasLoopCarriedDep &= sccInfo->hasLoopCarriedDep;
    }
}

SCCDAGPartition::SCCDAGPartition (SCCDAGPartition *partA, SCCDAGPartition *partB) {
    for (auto scc : partA->SCCs) this->SCCs.insert(scc);
    for (auto scc : partB->SCCs) this->SCCs.insert(scc);
    this->cost = partA->cost + partB->cost;
}

raw_ostream &printMinimalSCCs (raw_ostream &stream, std::string prefixToUse, std::set<SCC *> &sccs) {
    for (auto &removableSCC : sccs) {
        stream << prefixToUse << "Internal nodes: " << "\n";
        for (auto nodePair : removableSCC->internalNodePairs()) {
            nodePair.first->print(stream << prefixToUse << "\t");
            stream << "\n";
        }
    }
    return stream;
}

raw_ostream &SCCDAGPartition::print (raw_ostream &stream, std::string prefixToUse) {
    return printMinimalSCCs(stream, prefixToUse, this->SCCs);
}

SCCDAGPartition *SCCDAGPartitions::addPartition (SCC * scc) {
    std::set<SCC *> sccs = { scc };
    return this->addPartition(sccs);
}

SCCDAGPartition *SCCDAGPartitions::addPartition (std::set<SCC *> &sccs) {
    auto partition = std::make_unique<SCCDAGPartition>(sccdagInfo, loopInfo, sccs);
    this->managePartitionInfo(partition.get());
    this->totalCost += partition->cost;
    return this->partitions.insert(std::move(partition)).first->get();
}

void SCCDAGPartitions::initialize (SCCDAG *dag, SCCDAGInfo *dagInfo, LoopInfoSummary *lInfo, int threads) {
    sccDAG = dag;
    sccdagInfo = dagInfo;
    loopInfo = lInfo;
    idealThreads = threads;
    totalCost = 0;
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

std::set<SCCDAGPartition *> SCCDAGPartitions::getDependents (SCCDAGPartition *partition) {
    std::set<DGNode<SCC> *> sccNodes;
    for (auto scc : partition->SCCs) sccNodes.insert(this->sccDAG->fetchNode(scc));
    std::set<SCCDAGPartition *> depParts = this->getDependents(sccNodes);
    if (depParts.find(partition) != depParts.end()) depParts.erase(partition);
    return depParts;
}

std::set<SCCDAGPartition *> SCCDAGPartitions::getAncestors (SCCDAGPartition *partition) {
    std::set<DGNode<SCC> *> sccNodes;
    for (auto scc : partition->SCCs) sccNodes.insert(this->sccDAG->fetchNode(scc));
    std::set<SCCDAGPartition *> preParts = this->getAncestors(sccNodes);
    if (preParts.find(partition) != preParts.end()) preParts.erase(partition);
    return preParts;
}

std::set<SCCDAGPartition *> SCCDAGPartitions::getRelated (std::set<DGNode<SCC> *> &sccNodes,
    std::function<void (std::queue<DGNode<SCC> *> &, DGNode<SCC> *)> addKinFunc) {

    std::set<SCCDAGPartition *> related;
    for (auto sccNode : sccNodes) {
        auto selfPartition = this->partitionOf(sccNode->getT());
        std::queue<DGNode<SCC> *> sccToCheck;
        sccToCheck.push(sccNode);
        while (!sccToCheck.empty()) {
            auto sccNode = sccToCheck.front();
            sccToCheck.pop();

            auto part = this->partitionOf(sccNode->getT());
            if (part && part != selfPartition) {
                if (related.find(part) == related.end()) {
                    related.insert(part);
                }
                continue;
            }

            addKinFunc(sccToCheck, sccNode);
        }
    }
    return related;
}

std::set<SCCDAGPartition *> SCCDAGPartitions::getDependents (std::set<DGNode<SCC> *> &sccNodes) {
    auto addDependents = [&](std::queue<DGNode<SCC> *> &sccToCheck, DGNode<SCC> *sccNode) -> void {
        for (auto edge : sccNode->getOutgoingEdges()) {
            sccToCheck.push(edge->getIncomingNode());
        }
    };
    return getRelated(sccNodes, addDependents);
}

std::set<SCCDAGPartition *> SCCDAGPartitions::getAncestors (std::set<DGNode<SCC> *> &sccNodes) {
    auto addAncestors = [&](std::queue<DGNode<SCC> *> &sccToCheck, DGNode<SCC> *sccNode) -> void {
        for (auto edge : sccNode->getIncomingEdges()) {
            sccToCheck.push(edge->getOutgoingNode());
        }
    };
    return getRelated(sccNodes, addAncestors);
}

raw_ostream &SCCDAGPartitions::print (raw_ostream &stream, std::string prefixToUse) {
    for (auto &partition : this->partitions) {
        partition->print(stream << prefixToUse << "Partition:\n", prefixToUse);
    }
    return printMinimalSCCs(stream << prefixToUse << "Removable nodes:\n", prefixToUse, this->removableNodes);
}