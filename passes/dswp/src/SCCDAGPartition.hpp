#pragma once

class SCCDAGPartition {
  public:
    std::set<SCC *> SCCs;
}

class SCCDAGPartitions {
  public:
    std::set<std::unique_ptr<SCCDAGPartition>> partitions;

    // TODO add methods to map an SCC to its partition

  private:
    std::unordered_map<SCC *, SCCDAGPartition> fromSCCToPartition;
}
