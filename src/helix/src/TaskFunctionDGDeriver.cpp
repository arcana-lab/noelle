/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "HELIX.hpp"
#include "HELIXTask.hpp"
#include <set>

using namespace llvm ;

static void constructEdgesFromUseDefs (PDG *pdg);
static void constructEdgesFromControlForFunction (PDG *pdg, Function &F, PostDominatorTree &postDomTree);

PDG * HELIX::constructTaskFunctionDGFromOriginalLoopDG (LoopDependenceInfo *LDI, PostDominatorTree &postDomTreeOfTaskFunction) {
  auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);

  this->taskFunctionDG = new PDG(*helixTask->F);
  constructEdgesFromUseDefs(this->taskFunctionDG);

  dumpToFile(*LDI);
  constructEdgesFromControlForFunction(this->taskFunctionDG, *helixTask->F, postDomTreeOfTaskFunction);

  auto copyEdgeUsingTaskClonedValues = [&](DGEdge<Value> *originalEdge) -> void {
      DGEdge<Value> edgeToPointToClones(*originalEdge);
      // originalEdge->getOutgoingT()->print(errs() << "Fetching clone of: "); errs() << "\n";
      // originalEdge->getIncomingT()->print(errs() << "Fetching clone of: "); errs() << "\n";
      edgeToPointToClones.setNodePair(
        this->taskFunctionDG->fetchNode(helixTask->instructionClones.at(cast<Instruction>(originalEdge->getOutgoingT()))),
        this->taskFunctionDG->fetchNode(helixTask->instructionClones.at(cast<Instruction>(originalEdge->getIncomingT())))
      );
      this->taskFunctionDG->copyAddEdge(edgeToPointToClones);
  };

  /*
   * Derive intra-iteration memory dependencies from original dependence graph
   */
  for (auto nodePair : LDI->getLoopDG()->internalNodePairs()) {
    auto value = nodePair.first;
    if (isa<StoreInst>(value) || isa<LoadInst>(value) || isa<CallInst>(value)) {
      for (auto edge : nodePair.second->getOutgoingEdges()) {
        if (LDI->getLoopDG()->isInternal(edge->getIncomingT()) && edge->isMemoryDependence()) {
          copyEdgeUsingTaskClonedValues(edge);
        }
      }
    }
  }

  auto aliasStoresAndLoadsOfMemoryLocation = [&](std::set<StoreInst *> &stores, std::set<LoadInst *> &loads) -> void {
    for (auto store : stores) {
      for (auto other : stores) {
        if (store == other) continue;
        this->taskFunctionDG->addEdge(store, other)->setMemMustType(true , true, DataDependenceType::DG_DATA_WAW);
        this->taskFunctionDG->addEdge(other, store)->setMemMustType(true , true, DataDependenceType::DG_DATA_WAW);
      }
    }

    for (auto store : stores) {
      for (auto load : loads) {
        this->taskFunctionDG->addEdge(store, load)->setMemMustType(true , true, DataDependenceType::DG_DATA_RAW);
        this->taskFunctionDG->addEdge(load, store)->setMemMustType(true , true, DataDependenceType::DG_DATA_WAR);
      }
    }
  };

  /*
   * Derive inter-iteration memory dependencies from the loop carried environment
   */
  for (auto spill : this->spills) {
    std::set<LoadInst *> loads;
    loads.insert(spill->environmentLoad);
    aliasStoresAndLoadsOfMemoryLocation(spill->environmentStores, loads);
  }

  DGPrinter::writeGraph<PDG>("technique-task-fdg-" + std::to_string(LDI->getID()) + ".dot", taskFunctionDG);
  return this->taskFunctionDG;
}

/*
 * HACK: Copied straight from PDGAnalysis. Refactor and provide from pdg WITHOUT requiring alias analysis dependencies
 */
static void constructEdgesFromUseDefs (PDG *pdg){
  for (auto node : make_range(pdg->begin_nodes(), pdg->end_nodes())) {
    auto pdgValue = node->getT();
    if (pdgValue->getNumUses() == 0)
      continue;

    for (auto& U : pdgValue->uses()) {
      auto user = U.getUser();

      if (isa<Instruction>(user) || isa<Argument>(user)) {
        auto edge = pdg->addEdge(pdgValue, user);
        edge->setMemMustType(false, true, DG_DATA_RAW);
      }
    }
  }
}

/*
 * HACK: Copied straight from PDGAnalysis. Refactor and provide from pdg WITHOUT requiring alias analysis dependencies
 */
static void constructEdgesFromControlForFunction (PDG *pdg, Function &F, PostDominatorTree &postDomTree) {
  for (auto &B : F)
  {
    SmallVector<BasicBlock *, 10> dominatedBBs;
    postDomTree.getDescendants(&B, dominatedBBs);

    /*
     * For each basic block that B post dominates, check if B doesn't stricly post dominate its predecessor
     * If it does not, then there is a control dependency from the predecessor to B 
     */
    for (auto dominatedBB : dominatedBBs)
    {
      for (auto predBB : make_range(pred_begin(dominatedBB), pred_end(dominatedBB)))
      {
        if (postDomTree.properlyDominates(&B, predBB)) continue;
        auto controlTerminator = predBB->getTerminator();
        for (auto &I : B)
        {
          auto edge = pdg->addEdge((Value*)controlTerminator, (Value*)&I);
          edge->setControl(true);
        }
      }
    }
  }
}
