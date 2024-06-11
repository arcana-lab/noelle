/*
 * Copyright 2023 - 2024  Simone Campanoni
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
#include "arcana/noelle/core/LDGGenerator.hpp"
#include "arcana/noelle/core/LoopIterationSpaceAnalysis.hpp"
#include "arcana/noelle/core/LoopCarriedDependencies.hpp"
#include "noelle/core/DataFlow.hpp"
#include "LoopAwareMemDepAnalysis.hpp"

namespace arcana::noelle {

// TODO: Refactor along with HELIX's exact same implementation of this method
DataFlowResult *computeReachabilityFromInstructions(
    LoopStructure *loopStructure) {
  assert(loopStructure != nullptr);

  auto loopHeader = loopStructure->getHeader();
  auto loopFunction = loopStructure->getFunction();

  /*
   * Run the data flow analysis needed to identify the locations where signal
   * instructions will be placed.
   */
  auto dfa = DataFlowEngine{};
  auto computeGEN = [](Instruction *i, DataFlowResult *df) {
    assert(i != nullptr);
    assert(df != nullptr);
    auto &gen = df->GEN(i);
    gen.insert(i);
    return;
  };
  auto computeOUT = [loopHeader](Instruction *inst,
                                 Instruction *succ,
                                 std::set<Value *> &OUT,
                                 DataFlowResult *df) {
    assert(succ != nullptr);
    assert(df != nullptr);

    /*
     * Check if the successor is the header.
     * In this case, we do not propagate the reachable instructions.
     * We do this because we are interested in understanding the reachability of
     * instructions within a single iteration.
     */
    auto succBB = succ->getParent();
    if (succBB == loopHeader) {
      return;
    }

    /*
     * Propagate the data flow values.
     */
    auto &inS = df->IN(succ);
    OUT.insert(inS.begin(), inS.end());
    return;
  };
  auto computeIN =
      [](Instruction *inst, std::set<Value *> &IN, DataFlowResult *df) {
        assert(inst != nullptr);
        assert(df != nullptr);

        auto &genI = df->GEN(inst);
        auto &outI = df->OUT(inst);
        IN.insert(outI.begin(), outI.end());
        IN.insert(genI.begin(), genI.end());
        return;
      };

  return dfa.applyBackward(loopFunction, computeGEN, computeIN, computeOUT);
}

void LDGGenerator::improveDependenceGraph(PDG *loopDG, LoopStructure *loop) {

  /*
   * Remove dependences.
   */
  this->removeDependences(loopDG, loop);

  /*
   * Remove loop-carried flags.
   */
  this->removeLoopCarriedDependences(loopDG, loop);
}

void LDGGenerator::removeDependences(PDG *loopDG, LoopStructure *loop) {

  /*
   * Fetch all dependences.
   */
  auto deps = loopDG->getSortedDependences();

  /*
   * Identify dependences to remove.
   */
  std::set<DGEdge<Value, Value> *> toDelete{};
  for (auto dep : deps) {

    /*
     * Fetch the two instructions that depend on each other.
     */
    auto s = dep->getSrc();
    auto d = dep->getDst();
    auto srcInst = dyn_cast<Instruction>(s);
    auto dstInst = dyn_cast<Instruction>(d);
    if (srcInst == nullptr) {
      continue;
    }
    if (dstInst == nullptr) {
      continue;
    }

    /*
     * Make sure we only check dependences between instructions that are both
     * within the loop.
     */
    if (!loop->isIncluded(srcInst)) {
      continue;
    }
    if (!loop->isIncluded(dstInst)) {
      continue;
    }

    /*
     * We only aim to remove memory dependences.
     */
    if (!isa<MemoryDependence<Value, Value>>(dep)) {
      continue;
    }
    auto memDep = cast<MemoryDependence<Value, Value>>(dep);

    /*
     * We only aim to remove memory dependences that are "may"
     */
    if (isa<MustMemoryDependence<Value, Value>>(memDep)) {
      continue;
    }

    /*
     * Try to remove the current memory dependence.
     */
    for (auto dda : this->ddAnalyses) {
      if (!dda->canThereBeAMemoryDataDependence(srcInst, dstInst, *loop)) {
        toDelete.insert(memDep);
        break;
      }
      auto r = dda->isThereThisMemoryDataDependenceType(
          memDep->getDataDependenceType(),
          srcInst,
          dstInst,
          *loop);
      switch (r) {

        case MemoryDataDependenceStrength::CANNOT_EXIST:
          toDelete.insert(memDep);
          break;

        default:
          break;
      }
    }
  }

  /*
   * Remove dependences.
   */
  for (auto dep : toDelete) {
    loopDG->removeEdge(dep);
  }

  return;
}

LDGGenerator::LDGGenerator() {
  return;
}

void LDGGenerator::addAnalysis(DependenceAnalysis *a) {
  this->ddAnalyses.insert(a);
}

PDG *LDGGenerator::generateLoopDependenceGraph(PDG *functionDG,
                                               ScalarEvolution &scalarEvolution,
                                               DominatorSummary &DS,
                                               CompilationOptionsManager *com,
                                               Loop *l,
                                               LoopTree &loopNode) {

  /*
   * Create the loop dependence graph.
   */
  for (auto edge : functionDG->getEdges()) {
    assert(!edge->isLoopCarriedDependence() && "Flag was already set");
  }
  auto loopDG = functionDG->createLoopsSubgraph(l);
  for (auto edge : loopDG->getEdges()) {
    assert(!edge->isLoopCarriedDependence() && "Flag was already set");
  }

  /*
   * Remove dependences thanks to compilation options.
   */
  if (com->arePRVGsNonDeterministic()) {
    std::set<DGEdge<Value, Value> *> toDelete;
    for (auto edge : loopDG->getEdges()) {
      if (!isa<MemoryDependence<Value, Value>>(edge)) {
        continue;
      }
      auto vo = edge->getSrc();
      auto vi = edge->getDst();
      if (!isa<CallBase>(vo)) {
        continue;
      }
      if (!isa<CallBase>(vi)) {
        continue;
      }
      auto voCall = cast<CallBase>(vo);
      auto viCall = cast<CallBase>(vi);

      /*
       * Fetch the callees.
       */
      auto voCallee = voCall->getCalledFunction();
      auto viCallee = viCall->getCalledFunction();
      if ((viCallee == nullptr) && (voCallee == nullptr)) {
        continue;
      }

      /*
       * Check if one of the calls is a PRVGs.
       */
      if (((voCallee != nullptr) && (voCallee->getName() != "rand"))
          && ((viCallee != nullptr) && (viCallee->getName() != "rand"))) {
        continue;
      }

      /*
       * One of the call is a PRVG.
       * Hence, all memory dependences with a PRVG can be removed.
       *
       * We can remove this dependence as PRVGs are non-deterministics.
       */
      toDelete.insert(edge);
    }

    /*
     * Remove dependences.
     */
    for (auto edge : toDelete) {
      loopDG->removeEdge(edge);
    }
  }

  /*
   * Fetch the set of instructions that compose the loop.
   */
  std::vector<Value *> loopInternals;
  for (auto internalNode : loopDG->internalNodePairs()) {
    loopInternals.push_back(internalNode.first);
  }

  /*
   * Compute the SCCDAG using only variable-related dependences.
   * This will be used to detect induction variables.
   */
  auto loopInternalDG = loopDG->createSubgraphFromValues(loopInternals, false);
  auto loopSCCDAGWithoutMemoryDeps =
      this->computeSCCDAGWithOnlyVariableAndControlDependences(loopInternalDG);

  /*
   * Detect the loop-carried data dependences.
   *
   * HACK: The reason LoopCarriedDependencies is constructed SPECIFICALLY with
   * the DG that is used to query it is because it holds references to edges
   * copied to that specific instance of the DG. Edges are NOT referential to a
   * single DG source. When they are, this won't need to be done
   *
   * HACK: The SCCDAG is constructed with a loop internal DG to avoid external
   * nodes in the loop DG which provide context (live-ins/live-outs) but which
   * complicate analyzing the resulting SCCDAG
   */
  LoopCarriedDependencies::setLoopCarriedDependencies(&loopNode, DS, *loopDG);

  /*
   * Detect loop invariants and induction variables.
   */
  auto loopStructure = loopNode.getLoop();
  auto loopExitBlocks = loopStructure->getLoopExitBasicBlocks();
  auto env = LoopEnvironment(loopDG, loopExitBlocks, {});
  auto invManager = InvariantManager(loopStructure, loopDG);
  auto ivManager = InductionVariableManager(&loopNode,
                                            invManager,
                                            scalarEvolution,
                                            *loopSCCDAGWithoutMemoryDeps,
                                            env,
                                            *l);

  /*
   * Check if loop-centric dependence analyses are enabled.
   */
  if (this->areLoopDependenceAnalysesEnabled()) {

    /*
     * Run SCAF.
     */
    refinePDGWithSCAF(loopDG, loopNode);

    /*
     * Run the iteration space analysis.
     */
    this->runAffineAnalysis(*loopDG, scalarEvolution, ivManager, loopNode);

    /*
     * Run the loop-centric dependence analyses.
     */
    this->improveDependenceGraph(loopDG, loopStructure);
  }

  return loopDG;
}

void LDGGenerator::runAffineAnalysis(PDG &loopDG,
                                     ScalarEvolution &scalarEvolution,
                                     InductionVariableManager &ivManager,
                                     LoopTree &loopNode) {

  /*
   * Fetch the loop.
   */
  auto loopStructure = loopNode.getLoop();

  /*
   * Create the analysis.
   */
  auto domainSpace =
      LoopIterationSpaceAnalysis(&loopNode, ivManager, scalarEvolution);

  /*
   * Compute the reachability of instructions within the loop.
   */
  auto dfr = computeReachabilityFromInstructions(loopStructure);

  std::unordered_set<DGEdge<Value, Value> *> edgesToRemove;
  for (auto dependency :
       LoopCarriedDependencies::getLoopCarriedDependenciesForLoop(
           *loopStructure,
           &loopNode,
           loopDG)) {

    /*
     * Do not waste time on edges that aren't memory dependencies
     */
    if (!isa<MemoryDependence<Value, Value>>(dependency)) {
      continue;
    }

    auto fromInst = dyn_cast<Instruction>(dependency->getSrc());
    auto toInst = dyn_cast<Instruction>(dependency->getDst());
    if (!fromInst || !toInst)
      continue;

    /*
     * Loop carried dependencies are conservatively marked as such; we can only
     * remove dependencies between a producer and consumer where we know the
     * producer can NEVER reach the consumer during the same iteration
     */
    auto &afterInstructions = dfr->OUT(fromInst);
    if (afterInstructions.find(toInst) != afterInstructions.end()) {
      continue;
    }

    if (domainSpace
            .areInstructionsAccessingDisjointMemoryLocationsBetweenIterations(
                fromInst,
                toInst)) {
      edgesToRemove.insert(dependency);
    }
  }

  for (auto edge : edgesToRemove) {
    edge->setLoopCarried(false);
    loopDG.removeEdge(edge);
  }

  /*
   * Free the memory
   */
  delete dfr;

  return;
}

void LDGGenerator::removeLoopCarriedDependences(PDG *loopDG,
                                                LoopStructure *loop) {

  /*
   * Fetch all dependences.
   */
  auto deps = loopDG->getSortedDependences();

  /*
   * Identify dependences to remove.
   */
  for (auto dep : deps) {

    /*
     * Fetch the two instructions that depend on each other.
     */
    auto s = dep->getSrc();
    auto d = dep->getDst();
    auto srcInst = dyn_cast<Instruction>(s);
    auto dstInst = dyn_cast<Instruction>(d);
    if (srcInst == nullptr) {
      continue;
    }
    if (dstInst == nullptr) {
      continue;
    }

    /*
     * Make sure we only check dependences between instructions that are both
     * within the loop.
     */
    if (!loop->isIncluded(srcInst)) {
      continue;
    }
    if (!loop->isIncluded(dstInst)) {
      continue;
    }

    /*
     * We only target loop-carried dependences.
     */
    if (!dep->isLoopCarriedDependence()) {
      continue;
    }

    /*
     * Try to disprove the loop-carried property of the dependence.
     */
    for (auto dda : this->ddAnalyses) {
      if (!dda->canThisDependenceBeLoopCarried(dep, *loop)) {
        dep->setLoopCarried(false);
        break;
      }
    }
  }

  return;
}

SCCDAG *LDGGenerator::computeSCCDAGWithOnlyVariableAndControlDependences(
    PDG *loopDG) {

  /*
   * Compute the set of internal instructions of the loop.
   */
  std::vector<Value *> loopInternals;
  for (auto internalNode : loopDG->internalNodePairs()) {
    loopInternals.push_back(internalNode.first);
  }

  /*
   * Collect the dependences that we want to ignore.
   */
  std::unordered_set<DGEdge<Value, Value> *> memDeps{};
  for (auto currentDependence : loopDG->getSortedDependences()) {
    if (isa<MemoryDependence<Value, Value>>(currentDependence)) {
      memDeps.insert(currentDependence);
    }
  }

  /*
   * Compute the new loop dependence graph
   */
  auto loopDGWithoutMemoryDeps =
      loopDG->createSubgraphFromValues(loopInternals, false, memDeps);

  /*
   * Compute the SCCDAG
   */
  auto loopSCCDAGWithoutMemoryDeps = new SCCDAG(loopDGWithoutMemoryDeps);

  return loopSCCDAGWithoutMemoryDeps;
}

void LDGGenerator::enableLoopDependenceAnalyses(bool enabled) {
  this->loopDependenceAnalysesEnabled = enabled;
}

bool LDGGenerator::areLoopDependenceAnalysesEnabled(void) const {
  return this->loopDependenceAnalysesEnabled;
}

} // namespace arcana::noelle
