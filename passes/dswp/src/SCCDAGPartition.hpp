#pragma once

#include "SCC.hpp"
#include "SCCDAG.hpp"

class SCCDAGPartition {
  public:
    std::set<SCC *> SCCs;
    int cost;

    SCCDAGPartition (std::set<SCC *> &sccs);
    SCCDAGPartition (SCCDAGPartition *partitionA, SCCDAGPartition *partitionB);
};

class SCCDAGPartitions {
  public:
    std::set<std::unique_ptr<SCCDAGPartition>> partitions;
    std::set<SCC *> removableNodes;

    SCCDAGPartitions (SCCDAG *dag) : sccDAG{dag}, idealThreads{2}, totalCost{0} {};

    SCCDAGPartition *addPartition (SCC *node);
    SCCDAGPartition *addPartition (std::set<SCC *> &partition);
    void removePartition (SCCDAGPartition *partition);
    SCCDAGPartition *mergePartitions (SCCDAGPartition *partitionA, SCCDAGPartition *partitionB);
    
    SCCDAGPartition *partitionOf (SCC *scc);
    bool isRemovable (SCC *scc);
    std::set<SCCDAGPartition *> descendantsOf (SCCDAGPartition *partition);

    int numEdgesBetween (SCCDAGPartition *partitionA, SCCDAGPartition *partitionB);
    int maxPartitionCost () { return totalCost / idealThreads; }

  private:
    void managePartitionInfo (SCCDAGPartition *partition);

    SCCDAG *sccDAG;
    std::unordered_map<SCC *, SCCDAGPartition *> fromSCCToPartition;
    int totalCost;
    int idealThreads;
};