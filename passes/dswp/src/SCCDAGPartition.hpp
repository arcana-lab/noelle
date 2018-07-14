#pragma once

#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "SCCDAGInfo.hpp"
#include "LoopInfoSummary.hpp"

class SCCDAGPartition {
  public:
    std::set<SCC *> SCCs;
    int cost;
    std::set<LoopSummary *> loopsContained;
    bool hasLoopCarriedDep;

    SCCDAGPartition (SCCDAGInfo *sccdagInfo, LoopInfoSummary *loopInfo, std::set<SCC *> &sccs);
    SCCDAGPartition (SCCDAGInfo *sccdagInfo, LoopInfoSummary *loopInfo, SCCDAGPartition *partitionA, SCCDAGPartition *partitionB);

    void collectPartitionLoopInfo(SCCDAGInfo *sccdagInfo, LoopInfoSummary *loopInfo);
    void collectPartitionSCCInfo(SCCDAGInfo *sccdagInfo);

    raw_ostream &print(raw_ostream &stream, std::string prefixToUse);
};

class SCCDAGPartitions {
  public:
    std::set<std::unique_ptr<SCCDAGPartition>> partitions;
    std::set<SCC *> removableNodes;

    void initialize (SCCDAG *dag, SCCDAGInfo *dagInfo, LoopInfoSummary *lInfo, int idealThreads);

    SCCDAGPartition *addPartition (SCC *node);
    SCCDAGPartition *addPartition (std::set<SCC *> &partition);
    void removePartition (SCCDAGPartition *partition);
    SCCDAGPartition *mergePartitions (SCCDAGPartition *partitionA, SCCDAGPartition *partitionB);
    bool canMergePartitions (SCCDAGPartition *partitionA, SCCDAGPartition *partitionB);
    SCCDAGPartition *demoMergePartitions (SCCDAGPartition *partitionA, SCCDAGPartition *partitionB);

    SCCDAGPartition *partitionOf (SCC *scc);
    bool isRemovable (SCC *scc);
    std::set<SCCDAGPartition *> getDependents (SCCDAGPartition *partition);
    std::set<SCCDAGPartition *> getDependents (std::set<DGNode<SCC> *> &sccs);
    std::set<SCCDAGPartition *> getAncestors (SCCDAGPartition *partition);
    std::set<SCCDAGPartition *> getAncestors (std::set<DGNode<SCC> *> &sccs);

    std::set<SCCDAGPartition *> getCousins (SCCDAGPartition *partition);

    int numEdgesBetween (SCCDAGPartition *partitionA, SCCDAGPartition *partitionB);
    int maxPartitionCost () { return totalCost / idealThreads; }

    raw_ostream &print(raw_ostream &stream, std::string prefixToUse);

  private:
    std::set<SCCDAGPartition *> getRelated (std::set<DGNode<SCC> *> &sccNodes, std::function<void (std::queue<DGNode<SCC> *> &, DGNode<SCC> *)> addKinFunc);
    void managePartitionInfo (SCCDAGPartition *partition);
    std::set<DGNode<SCC> *> getSCCNodes (SCCDAGPartition *partition);

    SCCDAG *sccDAG;
    SCCDAGInfo *sccdagInfo;
    LoopInfoSummary *loopInfo;
    std::unordered_map<SCC *, SCCDAGPartition *> fromSCCToPartition;
    int totalCost;
    int idealThreads;
};