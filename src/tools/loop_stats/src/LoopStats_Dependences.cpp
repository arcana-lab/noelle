/*
 * Copyright 2019 - 2020  Angelo Matni, Simone Campanoni
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
#include "arcana/noelle/tools/LoopStats.hpp"
#include "arcana/noelle/core/LoopCarriedUnknownSCC.hpp"

namespace arcana::noelle {

void LoopStats::collectStatsOnLLVMSCCs(Hot *profiles,
                                       PDG *loopDG,
                                       Stats *statsForLoop) {

  /*
   * Construct loop internal SCCDAG (it uses LLVM's scc_iterator)
   */
  std::vector<Value *> loopInternals;
  for (auto internalNode : loopDG->internalNodePairs()) {
    loopInternals.push_back(internalNode.first);
  }
  auto loopInternalDG = loopDG->createSubgraphFromValues(loopInternals, false);
  auto loopInternalSCCDAG = SCCDAG(loopInternalDG);
  collectStatsOnSCCDAG(profiles,
                       &loopInternalSCCDAG,
                       nullptr,
                       nullptr,
                       statsForLoop);

  return;
}

void LoopStats::collectStatsOnNoelleSCCs(Hot *profiles,
                                         LoopContent &loopContent,
                                         Stats *statsForLoop,
                                         Loop &llvmLoop) {

  /*
   * HACK: we need to re-compute SCCDAGAttrs instead of using the one provided
   * by loopContent because we do NOT want to merge SCC as a convenience to
   * parallelization schemes
   *
   * Once this hack is removed, this can go away
   */
  auto loopStructure = loopContent.getLoopStructure();
  auto loopDG = loopContent.getLoopDG();
  std::vector<Value *> loopInternals;
  for (auto internalNode : loopDG->internalNodePairs()) {
    loopInternals.push_back(internalNode.first);
  }
  auto loopInternalDG = loopDG->createSubgraphFromValues(loopInternals, false);
  auto loopInternalSCCDAG = SCCDAG(loopInternalDG);

  auto loopHierarchy = loopContent.getLoopHierarchyStructures();
  auto loopFunction = loopStructure->getFunction();
  DominatorTree DT(*loopFunction);
  PostDominatorTree PDT(*loopFunction);
  DominatorSummary DS(DT, PDT);

  auto loopExitBlocks = loopStructure->getLoopExitBasicBlocks();
  auto environment = LoopEnvironment(loopDG, loopExitBlocks, {});
  auto invariantManager = loopContent.getInvariantManager();
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*loopFunction).getSE();
  auto inductionVariables = InductionVariableManager(loopHierarchy,
                                                     *invariantManager,
                                                     SE,
                                                     loopInternalSCCDAG,
                                                     environment,
                                                     llvmLoop);
  auto sccdagAttrs = SCCDAGAttrs(true,
                                 loopDG,
                                 &loopInternalSCCDAG,
                                 loopHierarchy,
                                 inductionVariables,
                                 DS);

  // DGPrinter::writeGraph<SCCDAG, SCC>("sccdag-" +
  // std::to_string(loopContent.getID())
  // + ".dot", &loopInternalSCCDAG);
  collectStatsOnSCCDAG(profiles,
                       &loopInternalSCCDAG,
                       &sccdagAttrs,
                       &loopContent,
                       statsForLoop);

  return;
}

void LoopStats::collectStatsOnSCCDAG(Hot *profiles,
                                     SCCDAG *sccdag,
                                     SCCDAGAttrs *sccdagAttrs,
                                     LoopContent *LC,
                                     Stats *statsForLoop) {

  /*
   * For every SCC object contained in an un-merged SCCDAG, we need to determine
   * whether it is a single independent instruction or a strongly connected
   * component. We do this by ensuring no edge within the SCC object is internal
   */
  for (auto node : sccdag->getNodes()) {
    auto scc = node->getT();

    statsForLoop->numberOfNodesInSCCDAG++;

    auto isSCC = false;
    for (auto edge : scc->getEdges()) {
      if (scc->isInternal(edge->getSrc()) && scc->isInternal(edge->getDst())) {
        isSCC = true;
        break;
      }
    }
    if (!isSCC)
      continue;

    statsForLoop->numberOfSCCs++;

    if (sccdagAttrs) {
      assert(LC != nullptr);

      /*
       * Fetch the SCC attributes
       */
      auto sccAttrs = sccdagAttrs->getSCCAttrs(scc);

      /*
       * Skip SCC that will not be executed sequentially
       */
      if (!isa<LoopCarriedUnknownSCC>(sccAttrs)) {
        continue;
      }
    }
    statsForLoop->numberOfSequentialSCCs++;
    statsForLoop->dynamicInstructionsOfSequentialSCCs +=
        profiles->getTotalInstructions(scc);
  }

  return;
}

} // namespace arcana::noelle
