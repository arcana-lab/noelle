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
#include "DOALL.hpp"
#include "HELIX.hpp"
#include "HELIXTask.hpp"

namespace llvm::noelle {

std::vector<SequentialSegment *> HELIX::identifySequentialSegments(
    LoopDependenceInfo *originalLDI,
    LoopDependenceInfo *LDI,
    DataFlowResult *reachabilityDFR) {

  /*
   * Fetch the task.
   */
  auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);

  /*
   * Map from old to new SCCs (for use in determining what SCC can be left out
   * of sequential segments)
   *
   * NOTE: Account for spilled PHIs in particular, because their instruction
   * mapping in the task is to the load in the pre-header. All stores to the
   * spill environment are in the loop and contained in the task's loop SCCDAG,
   * so use one of them.
   */
  std::unordered_map<SCC *, SCC *> taskToOriginalFunctionSCCMap;
  std::unordered_set<SCC *> spillSCCs;
  auto originalSCCManager = originalLDI->getSCCManager();
  auto originalSCCDAG = originalSCCManager->getSCCDAG();
  auto sccManager = LDI->getSCCManager();
  auto taskSCCDAG = sccManager->getSCCDAG();
  for (auto spill : this->spills) {
    auto originalSpillSCC =
        originalSCCDAG->sccOfValue(spill->originalLoopCarriedPHI);
    auto clonedInstructionInLoop = *spill->environmentStores.begin();
    auto clonedSpillSCC = taskSCCDAG->sccOfValue(clonedInstructionInLoop);
    assert(originalSpillSCC && clonedSpillSCC);
    spillSCCs.insert(originalSpillSCC);
    taskToOriginalFunctionSCCMap.insert(
        std::make_pair(clonedSpillSCC, originalSpillSCC));
  }
  for (auto originalNode : originalSCCDAG->getNodes()) {

    /*
     * Skip already mapped spill SCCs
     */
    auto originalSCC = originalNode->getT();
    if (spillSCCs.find(originalSCC) != spillSCCs.end()) {
      continue;
    }

    /*
     * Get clones of original instructions for the given SCC
     */
    Instruction *anyClonedInstInLoop = nullptr;
    auto clonedLoop = LDI->getLoopStructure();
    for (auto nodePair : originalSCC->internalNodePairs()) {

      /*
       * Fetch the original instruction.
       */
      auto originalInst = cast<Instruction>(nodePair.first);

      /*
       * Fetch the cloned one.
       */
      auto clonedInst = helixTask->getCloneOfOriginalInstruction(originalInst);

      /*
       * If there is no clone, then this instruction can be skipped.
       */
      if (clonedInst == nullptr) {
        continue;
      }
      assert(clonedInst != nullptr);
      if (!clonedLoop->isIncluded(clonedInst)) {
        continue;
      }

      /*
       * There is a cloned instruction, so we must consider it.
       */
      anyClonedInstInLoop = clonedInst;
      break;
    }

    /*
     * If there are no cloned instructions of the current SCC in the task, then
     * it means this SCC doesn't need to exist in the parallelized version of
     * the loop (e.g., a call to lifetime.start)
     */
    if (!anyClonedInstInLoop) {
      continue;
    }

    SCC *singleMappingSCC = nullptr;
    for (auto taskNode : taskSCCDAG->getNodes()) {
      auto taskSCC = taskNode->getT();
      auto hasOverlappingInstruction = taskSCC->isInternal(anyClonedInstInLoop);
      if (!hasOverlappingInstruction)
        continue;

      assert(singleMappingSCC == nullptr);
      singleMappingSCC = taskSCC;
    }

    // if (singleMappingSCC == nullptr) {
    //   originalSCC->print(errs() << "Original SCC:\n");
    //   anyClonedInstInLoop->print(errs() << "Any cloned INST: "); errs() <<
    //   "\n";
    // }

    assert(singleMappingSCC != nullptr);
    taskToOriginalFunctionSCCMap.insert(
        std::make_pair(singleMappingSCC, originalSCC));
  }

  /*
   * Prepare the initial partition.
   */
  ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::
      partitionSCCDAG(LDI);

  /*
   * Check whether the original loop was IV governed
   */
  auto wasOriginalLoopIVGoverned =
      originalLDI->getLoopGoverningIVAttribution() != nullptr;

  /*
   * Fetch the subsets.
   */
  auto sets = this->partitioner->getDepthOrderedSets();

  /*
   * Fetch the set of SCCs that have loop-carried data dependences.
   */
  auto depsSCCs = sccManager->getSCCsWithLoopCarriedDataDependencies();

  /*
   * Allocate the sequential segments, one per partition.
   */
  int32_t ssID = 0;
  std::vector<SequentialSegment *> sss;
  for (auto set : sets) {

    /*
     * Check if the current set of SCCs require a sequential segment.
     */
    auto requireSS = false;
    for (auto scc : set->sccs) {

      /*
       * Fetch the SCC metadata.
       * NOTE: If no original SCC mapping exists, default to analyzing the newly
       * constructed SCC
       */
      auto sccToAnalyze = scc;
      auto sccInfo = sccManager->getSCCAttrs(sccToAnalyze);
      if (taskToOriginalFunctionSCCMap.find(scc)
          != taskToOriginalFunctionSCCMap.end()) {
        sccToAnalyze = taskToOriginalFunctionSCCMap.at(scc);
        sccInfo = originalSCCManager->getSCCAttrs(sccToAnalyze);
      }

      /*
       * Do not synchronize induction variables
       */
      if (sccInfo->isInductionVariableSCC()) {
        continue;
      }

      /*
       * If the SCC is due to a control dependence, but the number of iterations
       * can be computed just before executing the loop, then we can skip it.
       */
      if (true && wasOriginalLoopIVGoverned
          && (depsSCCs.find(scc) == depsSCCs.end())) {
        continue;
      }

      /*
       * Only sequential SCC can generate a sequential segment.
       * FIXME: A reducible SCC should not be sequential in nature
       */
      if (sccInfo->mustExecuteSequentially()) {
        requireSS = true;
        break;
      }
    }
    if (!requireSS) {
      continue;
    }

    /*
     * Allocate a sequential segment.
     */
    auto ss = new SequentialSegment(this->noelle,
                                    LDI,
                                    reachabilityDFR,
                                    set,
                                    ssID,
                                    this->verbose);

    /*
     * Insert the new sequential segment to the list.
     */
    ssID++;
    sss.push_back(ss);
  }

  return sss;
}

} // namespace llvm::noelle
