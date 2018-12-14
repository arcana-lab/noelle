#include "ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences.hpp"

using namespace llvm;

ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences (
  Module &module, 
  Verbosity v
  )
  : ParallelizationTechnique{module, v}, partition{nullptr} 
  {

  return ;
}

ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::~ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences () {
  this->reset();

  return ;
}

void ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::reset () {
  if (partition != nullptr){
    delete partition;
    partition = nullptr;
  }

  ParallelizationTechnique::reset();

  return ;
}

void ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::partitionSCCDAG (LoopDependenceInfo *LDI) {

  /*
   * Print
   */
  if (this->verbose >= Verbosity::Minimal) {
    errs() << "ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences: Start\n";
  }

  /*
   * Initial the partition structure with the merged SCCDAG
   */
  auto subsets = new std::set<std::set<SCC *> *>();

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
    singleSet->insert(currentSCC);
    subsets->insert(singleSet);
  }

  /*
   * Ensure no memory edges go across subsets so no synchronization is necessary
   */
  this->partition = new SCCDAGPartition(LDI->loopSCCDAG, &LDI->sccdagAttrs, &LDI->liSummary, subsets);
  while (partition->mergeAlongMemoryEdges());

  /*
   * Print the number of partitions.
   */
  if (this->verbose >= Verbosity::Minimal) {
    errs() << "ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences:  Initial number of partitions: " << subsets->size() << "\n";
    errs() << "ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences: Exit\n";
  }

  return ;
}
