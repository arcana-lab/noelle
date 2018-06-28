#pragma once

#include "SCC.hpp"
#include "SCCDAG.hpp"

class SCCDAGPartition {
  public:
    std::set<SCC *> SCCs;
    int cost;

    SCCDAGPartition (std::set<SCC *> &sccs);
    SCCDAGPartition (SCCDAGPartition *partA, SCCDAGPartition *partB);
};

class SCCDAGPartitions {
  public:
    std::set<std::unique_ptr<SCCDAGPartition>> partitions;
    std::set<SCC *> removableNodes;

    void addPartition (SCC *node);
    void addPartition (std::set<SCC *> &partition);
    void removePartition (SCCDAGPartition *partition);
    void mergePartitions (SCCDAGPartition *partitionA, SCCDAGPartition *partitionB);
    SCCDAGPartition *partitionOf (SCC *scc);
    bool isRemovable (SCC *scc);

  private:
    void managePartitionInfo (SCCDAGPartition *partition);

    std::unordered_map<SCC *, SCCDAGPartition *> fromSCCToPartition;
};