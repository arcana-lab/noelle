/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/tools/ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences.hpp"

namespace llvm::noelle {

ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences (
  Noelle &n,
  bool forceParallelization
  )
  : ParallelizationTechnique{n}, partitioner{nullptr}, forceParallelization{forceParallelization}
  {

  return ;
}

bool ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::canBeAppliedToLoop (
  LoopDependenceInfo *LDI, 
  Heuristics *h
  ) const {

  /*
   * We do not handle loops with no successors.
   */
  auto ls = LDI->getLoopStructure();
  auto exits = ls->getLoopExitBasicBlocks();
  if (exits.size() == 0){
    return false;
  }

  return true;
}

ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::~ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences () {
  return ;
}

void ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::partitionSCCDAG (LoopDependenceInfo *LDI) {

  /*
   * Fetch the SCC manager.
   */
  auto sccManager = LDI->getSCCManager();

  /*
   * Print
   */
  auto sccdag = sccManager->getSCCDAG();
  if (this->verbose >= Verbosity::Minimal) {
    errs() << "ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences: Start\n";
    // DGPrinter::writeGraph<SCCDAG, SCC>("sccdag-to-partition-" + std::to_string(LDI->getID()) + ".dot", sccdag);
  }

  /*
   * Initial the partition structure with the merged SCCDAG
   */
  auto initialSets = std::unordered_set<SCCSet *>();

  /*
   * Assign SCCs that have no partition to their own partitions.
   */
  for (auto nodePair : sccdag->internalNodePairs()) {

    /*
     * Fetch the current node in the SCCDAG.
     */
    auto currentSCC = nodePair.first;
    auto currentSCCInfo = sccManager->getSCCAttrs(currentSCC);

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
    auto singleSet = new SCCSet();
    singleSet->sccs.insert(currentSCC);
    initialSets.insert(singleSet);
  }

  this->partitioner = new SCCDAGPartitioner(
    sccdag,
    initialSets,
    sccManager->parentsViaClones,
    LDI->getLoopHierarchyStructures()
  );

  /*
   * HACK: For correctness, we enforce that SCCs with LCDs between them belong to the same set  
   */
  // auto loopHierarchy = &LDI->getLoopHierarchyStructures();
  // auto function = loopHierarchy->getLoopNestingTreeRoot()->getFunction();
  // DominatorTree DT(*function);
  // PostDominatorTree PDT(*function);
  // DominatorSummary DS(DT, PDT);
  // LoopCarriedDependencies lcd(*loopHierarchy, DS, *sccManager->getSCCDAG());
  // this->partitioner->mergeLoopCarriedDependencies(&lcd);

  /*
   * Print the number of partitions.
   */
  if (this->verbose >= Verbosity::Minimal) {
    errs() << "ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences:  Initial number of partitions: "
      << initialSets.size() << "\n";
    errs() << "ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences: Exit\n";
  }

  for (auto set : initialSets) {
    delete set; 
  }

  return ;
}

}
