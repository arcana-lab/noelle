#pragma once

#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "SCCDAGAttrs.hpp"
#include "LoopInfoSummary.hpp"

typedef typename std::set<SCC *> SCCset;

class SCCDAGPartition {
  public:
    SCCDAGPartition (
      SCCDAG *dag,
      SCCDAGAttrs *attrs,
      LoopInfoSummary *LIS,
      std::set<SCCset *> *sets
    );

    void resetPartition (std::set<SCCset *> *subsets);

    SCCset *mergePairAndCycles (SCCset *subsetA, SCCset *subsetB);

    SCCset *mergePair (SCCset *subsetA, SCCset *subsetB, bool doReorder = true);

    bool mergeYieldsCycle (SCCset *subsetA, SCCset *subsetB);

    bool mergeAlongMemoryEdges ();

    uint64_t numberOfPartitions (void);

    std::set<SCCset *> *getSubsets() { return subsets; }
    std::set<SCCset *> *getRoots() { return &roots; }
    std::set<SCCset *> *getParents(SCCset *subset);
    std::set<SCCset *> *getChildren(SCCset *subset);
    std::vector<SCCset *> &getDepthOrderedSubsets () {
      return depthOrderedSubsets;
    }

    raw_ostream &print (raw_ostream &stream, std::string prefix);
    std::string subsetStr (SCCset *subset);
    raw_ostream &printSCCIndices (raw_ostream &stream, std::string prefix);
    raw_ostream &printNodeInGraph (raw_ostream &stream, std::string prefix, SCCset *subset);
    raw_ostream &printGraph (raw_ostream &stream, std::string prefix);

  private:

    void resetSubsetGraph ();
    void collectSubsetGraph ();
    bool hasCycle ();
    void orderSubsets ();

    bool mergeCycles ();
    bool traverseAndMerge (std::vector<SCCset *> &path);

    /*
     * Subset mapping
     */
    std::set<SCCset *> *subsets;
    std::unordered_map<SCC *, SCCset *> SCCToSet;
    std::vector<SCC *> SCCDebugOrder;
    std::unordered_map<SCC *, int> SCCDebugIndex;

    /*
     * Mappings at the end point in history
     */
    std::set<SCCset *> roots;
    std::unordered_map<SCCset *, std::set<SCCset *>> parentSubsets;
    std::unordered_map<SCCset *, std::set<SCCset *>> childrenSubsets;

    std::unordered_map<SCCset *, int> subsetDepths;
    std::vector<SCCset *> depthOrderedSubsets;

    /*
     * Static reference information
     */
    SCCDAG *sccdag;
    SCCDAGAttrs *dagAttrs;
    LoopInfoSummary *LIS;
};
