/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/tools/ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences.hpp"

namespace llvm::noelle {

ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::
    ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences(
        Noelle &n,
        bool forceParallelization)
  : ParallelizationTechnique{ n },
    partitioner{ nullptr },
    forceParallelization{ forceParallelization } {

  return;
}

bool ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::
    canBeAppliedToLoop(LoopDependenceInfo *LDI, Heuristics *h) const {

  /*
   * We do not handle loops with no successors.
   */
  auto ls = LDI->getLoopStructure();
  auto exits = ls->getLoopExitBasicBlocks();
  if (exits.size() == 0) {
    return false;
  }

  /*
   * We do not handle loops with invoke instructions.
   * This is because one of the successor will be a landingpad, which cannot
   * have normal basic blocks as predecessors; this breaks assumptions done for
   * the parallelization.
   */
  for (auto i : ls->getInstructions()) {
    if (isa<InvokeInst>(i)) {
      return false;
    }
  }

  return true;
}

ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::
    ~ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences() {
  return;
}

void ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::
    partitionSCCDAG(LoopDependenceInfo *LDI) {

  /*
   * Fetch the SCC manager.
   */
  auto sccManager = LDI->getSCCManager();

  /*
   * Print
   */
  auto sccdag = sccManager->getSCCDAG();
  if (this->verbose >= Verbosity::Minimal) {
    errs()
        << "ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences: Start\n";
    // DGPrinter::writeGraph<SCCDAG, SCC>("sccdag-to-partition-" +
    // std::to_string(LDI->getID()) + ".dot", sccdag);
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
     * Check if the current SCC can be removed (e.g., because it is due to
     * induction variables). If it is, then this SCC has already been assigned
     * to every dependent partition.
     */
    if (currentSCCInfo->canBeCloned()) {
      continue;
    }

    /*
     * The current SCC cannot be removed.
     */
    auto singleSet = new SCCSet();
    singleSet->sccs.insert(currentSCC);
    initialSets.insert(singleSet);
  }

  this->partitioner = new SCCDAGPartitioner(sccdag,
                                            initialSets,
                                            sccManager->parentsViaClones,
                                            LDI->getLoopHierarchyStructures());

  /*
   * HACK: For correctness, we enforce that SCCs with LCDs between them belong
   * to the same set
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
    errs()
        << "ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences:  Initial number of partitions: "
        << initialSets.size() << "\n";
    errs()
        << "ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences: Exit\n";
  }

  for (auto set : initialSets) {
    delete set;
  }

  return;
}

void ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::
    printSequentialCode(raw_ostream &stream,
                        const std::string &prefixString,
                        LoopDependenceInfo *LDI,
                        const std::set<SCC *> &sequentialSCCs) {

  /*
   * Fetch the sequential SCCs.
   */
  if (sequentialSCCs.size() > 0) {
    stream
        << prefixString << "There are " << sequentialSCCs.size()
        << " SCCs that have loop-carried dependences that cannot be broken\n";
  }

  /*
   * Print the sequential SCCs.
   */
  auto sccManager = LDI->getSCCManager();
  for (auto scc : sequentialSCCs) {
    stream << prefixString << "  SCC:\n";

    /*
     * Fetch the SCC metadata.
     */
    auto sccInfo = cast<LoopCarriedSCC>(sccManager->getSCCAttrs(scc));

    /*
     * The current SCC is sequential.
     */
    stream << prefixString << "    Loop-carried dependences\n";
    for (auto dep : sccInfo->getLoopCarriedDependences()) {

      /*
       * Fetch the instructions involved in the dependence
       */
      auto fromInst = dep->getOutgoingT();
      auto toInst = dep->getIncomingT();

      /*
       * Check that both instructions belong to the SCC.
       */
      std::string fromInstClarification{};
      if (scc->fetchNode(fromInst) == nullptr) {
        fromInstClarification.append(" (outside the SCC) ");
      }
      std::string toInstClarification{};
      if (scc->fetchNode(toInst) == nullptr) {
        toInstClarification.append(" (outside the SCC) ");
      }
      stream << prefixString << "      " << *fromInst << fromInstClarification
             << " ---> " << *toInst << toInstClarification;

      /*
       * Control dependences.
       */
      if (dep->isControlDependence()) {
        stream << " control\n";
        continue;
      }

      /*
       * Data dependences.
       */
      if (dep->isMemoryDependence()) {
        stream << " via memory\n";
      } else {
        stream << " via variable\n";
      }
    }

    /*
     * Print the content of the SCC.
     */
    auto prefixStringWithIndentation = std::string(prefixString);
    prefixStringWithIndentation.append("    ");
    scc->print(errs(), prefixStringWithIndentation);
  }

  /*
   * Print loop-carried dependences.
   */
  stream << prefixString
         << "Next are all loop-carried dependences of the loop\n";
  auto loopDG = LDI->getLoopDG();
  auto loopDependences = loopDG->getSortedDependences();
  for (auto dep : loopDependences) {
    if (!dep->isLoopCarriedDependence()) {
      continue;
    }
    auto fromInst = dep->getOutgoingT();
    auto toInst = dep->getIncomingT();
    stream << prefixString << "  " << *fromInst << " ---> " << *toInst;

    /*
     * Control dependences.
     */
    if (dep->isControlDependence()) {
      stream << " control\n";
      continue;
    }

    /*
     * Data dependences.
     */
    if (dep->isMemoryDependence()) {
      stream << " via memory\n";
    } else {
      stream << " via variable\n";
    }
  }
}

} // namespace llvm::noelle
