#include "SCCDAGPartition.hpp"

using namespace llvm;

SCCDAGPartition::SCCDAGPartition (SCCDAGInfo *sccdagInfo, LoopInfoSummary *loopInfo, std::set<SCC *> &sccs)
    : SCCs{sccs}, cost{0}, hasLoopCarriedDep{true} {
    collectPartitionLoopInfo(sccdagInfo, loopInfo);
    collectPartitionSCCInfo(sccdagInfo);
}

SCCDAGPartition::SCCDAGPartition (SCCDAGInfo *sccdagInfo, LoopInfoSummary *loopInfo, SCCDAGPartition *partA, SCCDAGPartition *partB) 
    : cost{0}, hasLoopCarriedDep{true} {
    for (auto scc : partA->SCCs) this->SCCs.insert(scc);
    for (auto scc : partB->SCCs) this->SCCs.insert(scc);
    collectPartitionLoopInfo(sccdagInfo, loopInfo);
    collectPartitionSCCInfo(sccdagInfo);
}

void SCCDAGPartition::collectPartitionLoopInfo (SCCDAGInfo *sccdagInfo, LoopInfoSummary *loopInfo) {
    /*
     * Collect all potentially fully-contained loops in the partition
     */
    std::unordered_map<LoopSummary *, std::set<BasicBlock *>> loopToBBContainedMap;
    for (auto scc : SCCs) {
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
}

/*
 * TODO: Use info on contained loops to partially determine total cost
 * TODO: Determine whether partition is DOALL or SEQuential
 */
void SCCDAGPartition::collectPartitionSCCInfo (SCCDAGInfo *sccdagInfo) {
    for (auto scc : SCCs) {
        auto &sccInfo = sccdagInfo->sccToInfo[scc];

        /*
         * Collect scc internal information 
         */
        this->cost += sccInfo->internalCost;
        this->hasLoopCarriedDep &= sccInfo->hasLoopCarriedDep;

        /*
         * Collect scc external cost (through edges)
         */
        for (auto sccCostPair : sccdagInfo->sccToInfo[scc]->sccToExternalCost) {
            if (SCCs.find(sccCostPair.first) != SCCs.end()) continue;
            this->cost += sccCostPair.second;
        }
    }
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
    auto partition = std::make_unique<SCCDAGPartition>(sccdagInfo, loopInfo, partitionA, partitionB);
    this->managePartitionInfo(partition.get());
    auto newPartition = this->partitions.insert(std::move(partition)).first->get();

    this->removePartition(partitionA);
    this->removePartition(partitionB);
    return newPartition;
}

SCCDAGPartition *SCCDAGPartitions::demoMergePartitions (SCCDAGPartition *partitionA, SCCDAGPartition *partitionB) {
    return new SCCDAGPartition(sccdagInfo, loopInfo, partitionA, partitionB);
}

bool SCCDAGPartitions::canMergePartitions (SCCDAGPartition *partitionA, SCCDAGPartition *partitionB) {
    std::set<SCC *> incomingToB;
    for (auto scc : partitionB->SCCs) {
        for (auto edge : sccDAG->fetchNode(scc)->getIncomingEdges()) {
            incomingToB.insert(edge->getOutgoingT());
        }
    }

    /*
     * Check that no cycle would form by merging the partitions
     */
    for (auto scc : partitionA->SCCs) {
        for (auto edge : sccDAG->fetchNode(scc)->getOutgoingEdges()) {
            if (incomingToB.find(edge->getIncomingT()) != incomingToB.end()) return false;
        }
    }
    return true;
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

std::set<DGNode<SCC> *> SCCDAGPartitions::getSCCNodes (SCCDAGPartition *partition) {
    std::set<DGNode<SCC> *> sccNodes;
    for (auto scc : partition->SCCs) sccNodes.insert(this->sccDAG->fetchNode(scc));
    return sccNodes;    
}

std::set<SCCDAGPartition *> SCCDAGPartitions::getDependents (SCCDAGPartition *partition) {
    auto sccNodes = this->getSCCNodes(partition);
    std::set<SCCDAGPartition *> parts = this->getDependents(sccNodes);
    if (parts.find(partition) != parts.end()) parts.erase(partition);
    return parts;
}

std::set<SCCDAGPartition *> SCCDAGPartitions::getAncestors (SCCDAGPartition *partition) {
    auto sccNodes = this->getSCCNodes(partition);
    std::set<SCCDAGPartition *> parts = this->getAncestors(sccNodes);
    if (parts.find(partition) != parts.end()) parts.erase(partition);
    return parts;
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

std::set<SCCDAGPartition *> SCCDAGPartitions::getCousins (SCCDAGPartition *partition) {
    auto sccNodes = this->getSCCNodes(partition);
    std::set<SCCDAGPartition *> parts = this->getAncestors(sccNodes);
    if (parts.find(partition) != parts.end()) parts.erase(partition);
    
    std::set<SCCDAGPartition *> neighbors;
    for (auto part : parts) {
        auto partSCCNodes = this->getSCCNodes(part);
        auto otherParts = this->getDependents(partSCCNodes);
        if (otherParts.find(part) != otherParts.end()) otherParts.erase(part);
        if (otherParts.find(partition) != otherParts.end()) otherParts.erase(partition);
        neighbors.insert(otherParts.begin(), otherParts.end());
    }
    return neighbors;
}

std::set<SCCDAGPartition *> SCCDAGPartitions::topLevelPartitions () {
    std::set<SCCDAGPartition *> topLevelParts;
    auto topLevelNodes = sccDAG->getTopLevelNodes();
    for (auto node : topLevelNodes) {
        auto part = this->partitionOf(node->getT());
        if (part) topLevelParts.insert(part);
    }

    /*
     * Should the top level nodes be removable, grab their descendants which belong to partitions
     */
    if (topLevelParts.size() == 0) {
        topLevelParts = this->getDependents(topLevelNodes);
    }

    std::set<SCCDAGPartition *> rootParts;
    for (auto part : topLevelParts) {
        if (this->getAncestors(part).size() > 0) continue;
        rootParts.insert(part);
    }

    errs() << "TOP LEVEL PARTITION CHECK:   SIZE OF NODES: " << topLevelNodes.size() << "\n";
    errs() << "TOP LEVEL PARTITION CHECK:   SIZE OF PARTS: " << topLevelParts.size() << "\n";
    errs() << "TOP LEVEL PARTITION CHECK:   SIZE OF ROOTS: " << rootParts.size() << "\n";
    return rootParts;
}

std::set<SCCDAGPartition *> SCCDAGPartitions::nextLevelPartitions (SCCDAGPartition *partition) {
    auto parts = this->getDependents(partition);
    std::set<SCCDAGPartition *> nextParts;
    for (auto part : parts) {
        auto prevParts = this->getAncestors(part);
        bool noPresentAncestors = true;
        for (auto prevPart : prevParts) {
            if (parts.find(prevPart) != parts.end()) {
                noPresentAncestors = false;
                break;
            }
        }
        if (noPresentAncestors) nextParts.insert(part);
    }
    return nextParts;
}

raw_ostream &SCCDAGPartitions::print (raw_ostream &stream, std::string prefixToUse) {
    for (auto &partition : this->partitions) {
        partition->print(stream << prefixToUse << "Partition:\n", prefixToUse);
    }
    return printMinimalSCCs(stream << prefixToUse << "Removable nodes:\n", prefixToUse, this->removableNodes);
}