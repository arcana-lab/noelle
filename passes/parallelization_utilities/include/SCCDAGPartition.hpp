#pragma once

#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "SCCDAGAttrs.hpp"
#include "LoopInfoSummary.hpp"

class SCCDAGSubset {
  public:
    std::set<SCC *> SCCs;
    std::set<LoopSummary *> loopsContained;

    SCCDAGSubset (SCCDAGAttrs *sccdagAttrs, LoopInfoSummary *loopInfo, std::set<SCC *> &sccs);

    raw_ostream &print(raw_ostream &stream, std::string prefixToUse);

  private:
    void collectSubsetLoopInfo(SCCDAGAttrs *sccdagAttrs, LoopInfoSummary *loopInfo);
};

class SCCDAGPartition {
  public:
    std::set<std::unique_ptr<SCCDAGSubset>> subsets;

    void initialize (SCCDAG *dag, SCCDAGAttrs *dagInfo, LoopInfoSummary *lInfo);

    raw_ostream &print(raw_ostream &stream, std::string prefixToUse);

    int getSubsetID (const std::unique_ptr<SCCDAGSubset> &subset);
    int subsetIDOfSCC (SCC *scc);
    bool isValidSubset (int id);
    SCCDAGSubset *subsetOfID (int id);

    int addSubset (SCC *node);
    int addSubset (std::set<SCC *> &subset);
    void removeSubset (int subsetID);
    std::set<SCC *> sccsOfSubsets (int subsetA, int subsetB);
    int mergeSubsets (int subsetA, int subsetB);
    bool canMergeSubsets (int subsetA, int subsetB);

    void mergeSubsetsRequiringMemSync ();
    void mergeSubsetsFormingCycles ();
    int traverseAndCheckToMerge (std::vector<int> &path);

    std::set<int> getDependentIDs (int subsetID);
    std::set<int> getDependentIDs (std::set<DGNode<SCC> *> &sccs);
    std::set<int> getAncestorIDs (int subsetID);
    std::set<int> getAncestorIDs (std::set<DGNode<SCC> *> &sccs);
    std::set<int> getSiblingIDs (int subsetID);
    std::set<int> getSubsetIDsWithNoIncomingEdges ();
    std::set<int> nextLevelSubsetIDs (int subsetID);

  private:
    std::set<int> getRelatedIDs (std::set<DGNode<SCC> *> &sccNodes, std::function<void (std::queue<DGNode<SCC> *> &, DGNode<SCC> *)> addKinFunc);

    std::set<DGNode<SCC> *> getSCCNodes (int subset);

    SCCDAG *sccDAG;
    SCCDAGAttrs *sccdagAttrs;
    LoopInfoSummary *loopInfo;

    std::unordered_map<SCC *, int> SCCToSubsetID;
    std::unordered_map<int, SCCDAGSubset *> subsetIDToSubset;
    int subsetCounter;
};
