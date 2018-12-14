#include "DSWP.hpp"

using namespace llvm;

void DSWP::partitionSCCDAG (DSWPLoopDependenceInfo *LDI, Heuristics *h) {

  /*
   * Prepare the initial partition.
   */
  ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::partitionSCCDAG(LDI);

  /*
   * Print the initial partitions.
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DSWP:  Before partitioning the SCCDAG\n";
    partition->print(errs(), "DSWP:   ");
  }

  /*
   * Check if we can cluster SCCs.
   */
  if (this->enableMergingSCC) {

    /*
     * Decide the partition of the SCCDAG by merging the trivial partitions defined above.
     */
    h->adjustParallelizationPartitionForDSWP(
      partition,
      LDI->sccdagAttrs,
      /*numThreads=*/LDI->maximumNumberOfCoresForTheParallelization,
      this->verbose
    );
  }

  /*
   * Print the partitioned SCCDAG.
   */
  if (this->verbose >= Verbosity::Minimal) {
    errs() << "DSWP:  Final number of partitions: " << this->partition->numberOfPartitions() << "\n";
  }
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DSWP:  After partitioning the SCCDAG\n";
    partition->print(errs(), "DSWP:   ");
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
