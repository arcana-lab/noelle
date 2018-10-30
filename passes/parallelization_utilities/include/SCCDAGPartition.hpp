#pragma once

#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "SCCDAGAttrs.hpp"
#include "LoopInfoSummary.hpp"

typedef typename std::set<SCC *> SCCset;

struct PartitionTransaction {
  std::set<SCCset *> oldSets, newSets;
}

class SCCDAGPartition {
  public:
    SCCDAGPartition () : subsetGraphBuilt{false} {}
    void initialize (SCCDAG *dag);

    raw_ostream &print(raw_ostream &stream, std::string prefixToUse);

    SCCset *createSubset (SCC *scc);
    SCCset *createSubset (SCCset *sccs);

    SCCset *mergeAndSquashCycles (SCCset &subsetA, SCCset &subsetB);

    void mergeSCCsetsRequiringMemSync ();

  private:

    void addSubset (SCCset *subset);

    void validateSubsetOrder ();
    void collectSubsetGraph ();
    void orderSubsets ();

    bool hasCycle ();

    void mergeSCCsetsFormingCycles ();
    int traverseAndCheckToMerge (std::vector<int> &path);

    /*
     * Subset mapping
     */
    std::set<SCCset *> subsets;
    std::unordered_map<SCC *, SCCset *> SCCToSet;

    /*
     * Mappings at the end point in history
     */
    bool subsetGraphBuilt;
    std::set<SCCset *> roots;
    std::unordered_map<SCCset *, std::set<SCCset *>> parentSubsets;
    std::unordered_map<SCCset *, std::set<SCCset *>> childrenSubsets;

    std::unordered_map<SCCset *, int> subsetDepths;
    std::vector<SCCset *> depthOrderedSubsets;

    /*
     * Transactions of merges and splits
     */
    std::vector<PartitionTransaction *> transactions;

    /*
     * Static reference information
     */
    SCCDAG *sccDAG;
};
