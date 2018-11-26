#include "DSWP.hpp"

using namespace llvm;

void DSWP::partitionSCCDAG (DSWPLoopDependenceInfo *LDI, Heuristics *h) {

  /*
   * Initial the partition structure with the merged SCCDAG
   */
  assert(subsets == nullptr && "ERROR: Partition should not exist yet\n");
  subsets = new std::set<std::set<SCC *> *>();

  /*
   * Assign SCCs that have no partition to their own partitions.
   */
  for (auto nodePair : LDI->loopSCCDAG->internalNodePairs()) {

    /*
     * Check if the current SCC can be removed (e.g., because it is due to induction variables).
     * If it is, then this SCC has already been assigned to every dependent partition.
     */
    auto currentSCC = nodePair.first;
    if (LDI->sccdagAttrs.canBeCloned(currentSCC)) continue ;
    auto singleSet = new std::set<SCC *>();
    singleSet->insert(nodePair.first);
    subsets->insert(singleSet);
  }

  /*
   * Ensure no memory edges go across subsets so no synchronization is necessary
   */
  partitioner = new SCCDAGPartition(LDI->loopSCCDAG, &LDI->sccdagAttrs, &LDI->liSummary, subsets);
  while (partitioner->mergeAlongMemoryEdges());

  /*
   * Print the initial partitions.
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DSWP:  Before partitioning the SCCDAG\n";
    partitioner->print(errs(), "DSWP:   ");
  }

  /*
   * Check if we can cluster SCCs.
   */
  if (this->enableMergingSCC) {

    /*
     * Decide the partition of the SCCDAG by merging the trivial partitions defined above.
     */
    h->adjustParallelizationPartitionForDSWP(partitioner, LDI->sccdagAttrs, /*idealThreads=*/ 2);
  }

  /*
   * Print the partitioned SCCDAG.
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DSWP:  After partitioning the SCCDAG\n";
    partitioner->print(errs(), "DSWP:   ");
  }

  return ;
}

/*
void DSWP::clusterSubloops (DSWPLoopDependenceInfo *LDI) {
  auto &li = LDI->liSummary;
  auto loop = li.bbToLoop[LDI->header];
  auto loopDepth = loop->depth;

  unordered_map<LoopSummary *, std::set<SCC *>> loopSets;
  for (auto sccNode : LDI->loopSCCDAG->getNodes()) {
    if (LDI->sccdagAttrs.canBeCloned(sccNode->getT())) continue;

    for (auto iNodePair : sccNode->getT()->internalNodePairs()) {
      auto bb = cast<Instruction>(iNodePair.first)->getParent();
      auto subL = li.bbToLoop[bb];
      auto subDepth = subL->depth;
      if (subL == loop) continue;
      assert(loopDepth < subDepth);

      while (subDepth - 1 > loopDepth) {
        subL = subL->parent;
        subDepth--;
      }
      loopSets[subL].insert(sccNode->getT());
      break;
    }
  }

  if (loopSets.size() == 1) return;
  for (auto loopSetPair : loopSets) {
    partition.addSubset(loopSetPair.second);
  }
}
*/
