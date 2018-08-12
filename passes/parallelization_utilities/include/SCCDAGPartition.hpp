#pragma once

#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "SCCDAGAttrs.hpp"
#include "LoopInfoSummary.hpp"

class SCCDAGSubset {
  public:
    std::set<SCC *> SCCs;
    int cost;
    std::set<LoopSummary *> loopsContained;

    SCCDAGSubset (SCCDAGAttrs *sccdagAttrs, LoopInfoSummary *loopInfo, std::set<SCC *> &sccs);
    SCCDAGSubset (SCCDAGAttrs *sccdagAttrs, LoopInfoSummary *loopInfo, SCCDAGSubset *subsetA, SCCDAGSubset *subsetB);

    raw_ostream &print(raw_ostream &stream, std::string prefixToUse);

  private:
    void collectSubsetLoopInfo(SCCDAGAttrs *sccdagAttrs, LoopInfoSummary *loopInfo);
};

class SCCDAGPartition {
  public:
    std::set<std::unique_ptr<SCCDAGSubset>> subsets;
    std::set<SCC *> removableNodes;

    void initialize (SCCDAG *dag, SCCDAGAttrs *dagInfo, LoopInfoSummary *lInfo);

    SCCDAGSubset *addSubset (SCC *node);
    SCCDAGSubset *addSubset (std::set<SCC *> &subset);
    void removeSubset (SCCDAGSubset *subset);
    SCCDAGSubset *mergeSubsets (SCCDAGSubset *subsetA, SCCDAGSubset *subsetB);
    bool canMergeSubsets (SCCDAGSubset *subsetA, SCCDAGSubset *subsetB);
    SCCDAGSubset *demoMergeSubsets (SCCDAGSubset *subsetA, SCCDAGSubset *subsetB);

    void mergeSubsetsRequiringMemSync ();
    void mergeSubsetsFormingCycles ();
    SCCDAGSubset *traverseAndCheckToMerge (std::vector<SCCDAGSubset *> &path);

    SCCDAGSubset *subsetOf (SCC *scc);
    bool isRemovable (SCC *scc);
    std::set<SCCDAGSubset *> getDependents (SCCDAGSubset *subset);
    std::set<SCCDAGSubset *> getDependents (std::set<DGNode<SCC> *> &sccs);
    std::set<SCCDAGSubset *> getAncestors (SCCDAGSubset *subset);
    std::set<SCCDAGSubset *> getAncestors (std::set<DGNode<SCC> *> &sccs);
    std::set<SCCDAGSubset *> getCousins (SCCDAGSubset *subset);
    std::set<SCCDAGSubset *> topLevelSubsets ();
    std::set<SCCDAGSubset *> nextLevelSubsets (SCCDAGSubset *subset);

    raw_ostream &print(raw_ostream &stream, std::string prefixToUse);

  private:
    std::set<SCCDAGSubset *> getRelated (std::set<DGNode<SCC> *> &sccNodes, std::function<void (std::queue<DGNode<SCC> *> &, DGNode<SCC> *)> addKinFunc);

    int numEdgesBetween (SCCDAGSubset *subsetA, SCCDAGSubset *subsetB);

    void manageAddedSubsetInfo (SCCDAGSubset *subset);

    std::set<DGNode<SCC> *> getSCCNodes (SCCDAGSubset *subset);

    SCCDAG *sccDAG;
    SCCDAGAttrs *sccdagAttrs;
    LoopInfoSummary *loopInfo;
    std::unordered_map<SCC *, SCCDAGSubset *> fromSCCToSubset;
};
