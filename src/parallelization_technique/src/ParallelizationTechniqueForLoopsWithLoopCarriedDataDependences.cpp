/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences.hpp"

using namespace llvm;

ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences (
  Module &module, 
  Hot &p,
  Verbosity v
  )
  : ParallelizationTechnique{module, p, v}, partition{nullptr}
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
  for (auto nodePair : LDI->sccdagAttrs.getSCCDAG()->internalNodePairs()) {

    /*
     * Fetch the current node in the SCCDAG.
     */
    auto currentSCC = nodePair.first;
    auto currentSCCInfo = LDI->sccdagAttrs.getSCCAttrs(currentSCC);

    /*
     * Check if the current SCC can be removed (e.g., because it is due to induction variables).
     * If it is, then this SCC has already been assigned to every dependent partition.
     */
    if (currentSCCInfo->canBeCloned()) {
      continue ;
    }

    /*
     * The current SCC cannot be removed.
     */
    auto singleSet = new std::set<SCC *>();
    singleSet->insert(currentSCC);
    subsets->insert(singleSet);
  }

  /*
   * Ensure no memory edges go across subsets so no synchronization is necessary
   */
  this->partition = new SCCDAGPartition(LDI->sccdagAttrs.getSCCDAG(), &LDI->sccdagAttrs, &LDI->liSummary, subsets);
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
