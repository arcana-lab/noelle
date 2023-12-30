/*
 * Copyright 2016 - 2023  Angelo Matni, Simone Campanoni
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
#include "noelle/tools/HELIX.hpp"

namespace arcana::noelle {

static void constructEdgesFromUseDefs(PDG *pdg);

static void constructEdgesFromControlForFunction(
    PDG *pdg,
    Function &F,
    const DominatorForest &postDominatorForest);

PDG *HELIX::constructTaskInternalDependenceGraphFromOriginalLoopDG(
    LoopContent *LDI) {

  /*
   * Fetch the task.
   */
  auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);
  auto taskBody = helixTask->getTaskBody();

  /*
   * Fetch the dominators.
   */
  auto doms = this->noelle.getDominators(taskBody);

  /*
   * Create a new PDG for the internals of the task.
   */
  auto taskFunctionDG = new PDG(*taskBody);
  constructEdgesFromUseDefs(taskFunctionDG);

  constructEdgesFromControlForFunction(taskFunctionDG, *taskBody, doms->PDT);

  auto copyEdgeUsingTaskClonedValues =
      [&](DGEdge<Value, Value> *originalEdge) -> void {
    /*
     * Fetch the clones of the instructions related to @originalEdge that are in
     * the task.
     */
    auto cloneOutgoingInst = helixTask->getCloneOfOriginalInstruction(
        cast<Instruction>(originalEdge->getSrc()));
    assert(cloneOutgoingInst != nullptr);
    auto cloneIncomingInst = helixTask->getCloneOfOriginalInstruction(
        cast<Instruction>(originalEdge->getDst()));
    assert(cloneIncomingInst != nullptr);
    auto cloneOutgoingNode = taskFunctionDG->fetchNode(cloneOutgoingInst);
    assert(cloneOutgoingNode != nullptr);
    auto cloneIncomingNode = taskFunctionDG->fetchNode(cloneIncomingInst);
    assert(cloneIncomingNode != nullptr);

    /*
     * Allocate the new dependence within the task to be the clone of
     * @originalEdge.
     */
    DGEdge<Value, Value> *edgeToPointToClones = nullptr;
    if (isa<ControlDependence<Value, Value>>(originalEdge)) {
      auto originalEdgeAsCD =
          cast<ControlDependence<Value, Value>>(originalEdge);
      edgeToPointToClones =
          new ControlDependence<Value, Value>(*originalEdgeAsCD);
    } else {
      auto originalEdgeAsDD = cast<DataDependence<Value, Value>>(originalEdge);
      if (auto originalEdgeAsVD =
              dyn_cast<VariableDependence<Value, Value>>(originalEdgeAsDD)) {
        edgeToPointToClones =
            new VariableDependence<Value, Value>(*originalEdgeAsVD);
      } else if (auto originalEdgeAsMD =
                     dyn_cast<MayMemoryDependence<Value, Value>>(
                         originalEdgeAsDD)) {
        edgeToPointToClones =
            new MayMemoryDependence<Value, Value>(*originalEdgeAsMD);
      } else {
        auto originalEdgeAsMustMD =
            cast<MustMemoryDependence<Value, Value>>(originalEdgeAsDD);
        edgeToPointToClones =
            new MustMemoryDependence<Value, Value>(*originalEdgeAsMustMD);
      }
    }
    edgeToPointToClones->setSrcNode(cloneOutgoingNode);
    edgeToPointToClones->setDstNode(cloneIncomingNode);

    /*
     * Loop carry dependencies will be recomputed
     */
    edgeToPointToClones->setLoopCarried(false);

    /*
     * Add the edge to the task internal dependence graph
     */
    taskFunctionDG->copyAddEdge(*edgeToPointToClones);
  };

  /*
   * Derive intra-iteration memory dependencies from original dependence graph
   */
  for (auto nodePair : LDI->getLoopDG()->internalNodePairs()) {

    /*
     * Fetch the value.
     */
    auto value = nodePair.first;

    /*
     * We only care about instructions that can generate memory dependences.
     */
    if ((!isa<StoreInst>(value)) && (!isa<LoadInst>(value))
        && (!isa<CallInst>(value))) {
      continue;
    }

    /*
     * Check the dependence.
     */
    for (auto edge : nodePair.second->getOutgoingEdges()) {

      /*
       * We only care about memory dependences.
       */
      if (!isa<MemoryDependence<Value, Value>>(edge)) {
        continue;
      }

      /*
       * This is a memory dependence.
       *
       * Check if it is due to lifetime intrinsics.
       */
      auto srcValue = edge->getDst();
      auto dstValue = edge->getSrc();
      if (auto valueAsCallInst = dyn_cast<CallInst>(srcValue)) {
        if (valueAsCallInst->isLifetimeStartOrEnd()) {
          continue;
        }
      }
      if (auto valueAsCallInst = dyn_cast<CallInst>(dstValue)) {
        if (valueAsCallInst->isLifetimeStartOrEnd()) {
          continue;
        }
      }

      /*
       * We only care about instruction within the parallelized loop.
       */
      if (!LDI->getLoopDG()->isInternal(srcValue)) {
        continue;
      }

      /*
       * Copy the dependence.
       */
      copyEdgeUsingTaskClonedValues(edge);
    }
  }

  auto aliasStoresAndLoadsOfMemoryLocation =
      [&](std::unordered_set<StoreInst *> &stores,
          std::unordered_set<LoadInst *> &loads) -> void {
    for (auto store : stores) {
      for (auto other : stores) {
        taskFunctionDG->addMemoryDataDependenceEdge(
            store,
            other,
            DataDependenceType::DG_DATA_WAW,
            true);
        taskFunctionDG->addMemoryDataDependenceEdge(
            other,
            store,
            DataDependenceType::DG_DATA_WAW,
            true);
      }
    }

    for (auto store : stores) {
      for (auto load : loads) {
        taskFunctionDG->addMemoryDataDependenceEdge(
            store,
            load,
            DataDependenceType::DG_DATA_RAW,
            true);
        taskFunctionDG->addMemoryDataDependenceEdge(
            load,
            store,
            DataDependenceType::DG_DATA_WAR,
            true);
      }
    }
  };

  /*
   * Derive inter-iteration memory dependencies from the loop carried
   * environment
   */
  for (auto spill : this->spills) {
    aliasStoresAndLoadsOfMemoryLocation(spill->environmentStores,
                                        spill->environmentLoads);
  }

  /*
   * Free the memory
   */
  delete doms;

  return taskFunctionDG;
}

/*
 * HACK: Copied straight from PDGAnalysis. Refactor and provide from pdg WITHOUT
 * requiring alias analysis dependencies
 */
static void constructEdgesFromUseDefs(PDG *pdg) {
  for (auto node : make_range(pdg->begin_nodes(), pdg->end_nodes())) {
    auto pdgValue = node->getT();
    if (pdgValue->getNumUses() == 0)
      continue;

    for (auto &U : pdgValue->uses()) {
      auto user = U.getUser();

      if (isa<Instruction>(user) || isa<Argument>(user)) {
        auto edge =
            pdg->addVariableDataDependenceEdge(pdgValue, user, DG_DATA_RAW);
      }
    }
  }
}

/*
 * HACK: Copied straight from PDGAnalysis. Refactor and provide from pdg WITHOUT
 * requiring alias analysis dependencies
 */
static void constructEdgesFromControlForFunction(
    PDG *pdg,
    Function &F,
    const DominatorForest &postDominatorForest) {

  for (auto &B : F) {

    /*
     * Fetch the basic blocks that are dominated by @B.
     */
    auto descendants = postDominatorForest.getDescendants(&B);

    /*
     * For each basic block that B post dominates, check if B doesn't stricly
     * post dominate its predecessor If it does not, then there is a control
     * dependency from the predecessor to B
     */
    for (auto dominatedBB : descendants) {
      for (auto predBB :
           make_range(pred_begin(dominatedBB), pred_end(dominatedBB))) {
        if (postDominatorForest.strictlyDominates(&B, predBB)) {
          continue;
        }
        auto controlTerminator = predBB->getTerminator();
        for (auto &I : B) {
          pdg->addControlDependenceEdge(controlTerminator, &I);
        }
      }
    }
  }
}

} // namespace arcana::noelle
