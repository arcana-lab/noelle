#pragma once

#include "SCC.hpp"
#include "SCCDAG.hpp"

class SCCDAGPartition {
  public:
    std::set<SCC *> SCCs;

    SCCDAGPartition(std::set<SCC *> &sccs) : SCCs{sccs} {};
};

class SCCDAGPartitions {
  public:
    std::set<std::unique_ptr<SCCDAGPartition>> partitions;
    std::set<SCC *> removableNodes;

    void addPartition (SCC *node);
    void addPartition (std::set<SCC *> &partition);
    SCCDAGPartition *partitionOf (SCC *scc);
    bool isRemovable (SCC *scc);

  private:
    std::unordered_map<SCC *, SCCDAGPartition *> fromSCCToPartition;
};
