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

namespace arcana::noelle {

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
    canBeAppliedToLoop(LoopContent *loopContent, Heuristics *h) const {

  /*
   * We do not handle loops with no successors.
   */
  auto ls = loopContent->getLoopStructure();
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
    partitionSCCDAG(LoopContent *loopContent) {
  auto f = [](GenericSCC *scc) -> bool { return false; };
  this->partitionSCCDAG(loopContent, f);
}

void ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::
    partitionSCCDAG(LoopContent *loopContent,
                    std::function<bool(GenericSCC *scc)> skipSCC) {

  /*
   * Fetch the SCC manager.
   */
  auto sccManager = loopContent->getSCCManager();

  /*
   * Print
   */
  auto sccdag = sccManager->getSCCDAG();
  if (this->verbose >= Verbosity::Minimal) {
    errs()
        << "ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences: Start\n";
  }

  /*
   * Initial the partition structure with the merged SCCDAG
   */
  auto initialSets = std::unordered_set<SCCSet *>();

  /*
   * Fetch the SCCs that must be considered in the partitioning algorithm.
   */
  std::set<SCC *> notClonableSCCs{};
  for (auto nodePair : sccdag->internalNodePairs()) {

    /*
     * Fetch the current node in the SCCDAG.
     */
    auto currentSCC = nodePair.first;
    auto currentSCCInfo = sccManager->getSCCAttrs(currentSCC);
    assert(currentSCCInfo != nullptr);

    /*
     * Check if the current SCC can be removed (e.g., because it is due to
     * induction variables). If it is, then this SCC has already been assigned
     * to every dependent partition.
     */
    if (skipSCC(currentSCCInfo)) {
      continue;
    }

    /*
     * The current SCC must be considered.
     */
    notClonableSCCs.insert(currentSCC);
  }

  /*
   * Print the SCCs.
   */
  if (this->verbose >= Verbosity::Maximal) {
    /*
    auto sortedSCCs = this->noelle.sortByHotness(notClonableSCCs);
    errs() << "ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences:
    SCCs to consider when partitioning the SCCDAG:\n"; for (auto currentSCC :
    sortedSCCs){ currentSCC->print(errs());
    }
    */
  }

  /*
   * Assign SCCs that have no partition to their own partitions.
   */
  for (auto currentSCC : notClonableSCCs) {
    auto singleSet = new SCCSet();
    singleSet->sccs.insert(currentSCC);
    initialSets.insert(singleSet);
  }

  auto newSCCDAGWithoutIgnoredSCCs =
      sccManager->computeSCCDAGWhenSCCsAreIgnored(skipSCC);
  this->partitioner =
      new SCCDAGPartitioner(sccdag,
                            initialSets,
                            newSCCDAGWithoutIgnoredSCCs.first,
                            loopContent->getLoopHierarchyStructures());

  /*
   * HACK: For correctness, we enforce that SCCs with LCDs between them belong
   * to the same set
   */
  // auto loopHierarchy = &loopContent->getLoopHierarchyStructures();
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
                        LoopContent *loopContent,
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
  auto sccManager = loopContent->getSCCManager();
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
      auto fromInst = dep->getSrc();
      auto toInst = dep->getDst();

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
      if (isa<ControlDependence<Value, Value>>(dep)) {
        stream << " control\n";
        continue;
      }

      /*
       * Data dependences.
       */
      if (isa<MemoryDependence<Value, Value>>(dep)) {
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
  auto loopDG = loopContent->getLoopDG();
  auto loopDependences = loopDG->getSortedDependences();
  for (auto dep : loopDependences) {
    if (!dep->isLoopCarriedDependence()) {
      continue;
    }
    auto fromInst = dep->getSrc();
    auto toInst = dep->getDst();
    stream << prefixString << "  " << *fromInst << " ---> " << *toInst;

    /*
     * Control dependences.
     */
    if (isa<ControlDependence<Value, Value>>(dep)) {
      stream << " control\n";
      continue;
    }

    /*
     * Data dependences.
     */
    if (isa<MemoryDependence<Value, Value>>(dep)) {
      stream << " via memory\n";
    } else {
      stream << " via variable\n";
    }
  }
}

} // namespace arcana::noelle
