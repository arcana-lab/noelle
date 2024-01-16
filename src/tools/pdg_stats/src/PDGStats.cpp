/*
 * Copyright 2016 - 2023  Yian Su, Simone Campanoni
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
#include <string>

#include "noelle/core/PDGPrinter.hpp"
#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/SCCDAGAttrs.hpp"
#include "PDGStats.hpp"

namespace arcana::noelle {

bool PDGStats::runOnModule(Module &M) {

  /*
   * Fetch the NOELLE framework.
   */
  auto &noelle = getAnalysis<Noelle>();

  /*
   * Compute the loops for all functions.
   */
  std::unordered_map<Function *, LoopForest *> programLoopForests;
  std::unordered_map<Function *, std::vector<LoopContent *> *> programLoops;
  std::unordered_map<LoopStructure *, LoopContent *> lsToLC;
  for (auto &F : M) {

    /*
     * Fetch all loops within the current function.
     */
    programLoops[&F] = noelle.getLoopContents(&F);
    if (programLoops[&F] == nullptr) {
      continue;
    }

    /*
     * Create the map from loop structure to LoopContent.
     */
    std::unordered_map<Function *, std::vector<LoopStructure *>>
        programLoopStructures;
    auto &loopStructures = programLoopStructures[&F];
    for (auto loopContent : *programLoops[&F]) {
      auto ls = loopContent->getLoopStructure();
      lsToLC[ls] = loopContent;
      loopStructures.push_back(ls);
    }

    /*
     * Organize the loops in a forest.
     */
    programLoopForests[&F] =
        noelle.organizeLoopsInTheirNestingForest(loopStructures);
  }

  /*
   * Compute the memory edges in the PDG.
   */
  auto PDG = noelle.getProgramDependenceGraph();
  for (auto edge : PDG->getEdges()) {

    /*
     * Handle dependence.
     */
    this->analyzeDependence(edge);
  }

  /*
   * Collect the statistics for all functions.
   */
  for (auto &F : M) {
    this->collectStatsForNodes(F);
    this->collectStatsForPotentialEdges(programLoopForests, F);
    this->collectStatsForLoopEdges(noelle, programLoopForests, lsToLC, F);

    if (this->dumpLoopDG) {
      this->printRefinedLoopGraphsForFunction(noelle,
                                              programLoopForests,
                                              lsToLC,
                                              F);
    }
  }

  this->collectSCCStats(noelle);

  /*
   * Print the statistics.
   */
  printStats();

  return false;
}

void PDGStats::collectStatsForNodes(Function &F) {
  for (auto &arg : F.args()) {
    this->numberOfNodes++;
  }
  for (auto &B : F) {
    this->numberOfNodes += B.size();
  }

  return;
}

void PDGStats::collectStatsForPotentialEdges(
    std::unordered_map<Function *, LoopForest *> &programLoops,
    Function &F) {

  /*
   * Compute the total number of instructions that could access memory.
   */
  uint64_t totLoads = 0;
  uint64_t totStores = 0;
  uint64_t totCalls = 0;
  for (auto &inst : instructions(F)) {
    if (isa<LoadInst>(&inst)) {
      totLoads++;
      continue;
    }
    if (isa<StoreInst>(&inst)) {
      totStores++;
      continue;
    }
    if (false || isa<CallInst>(&inst) || isa<InvokeInst>(&inst)) {
      totCalls++;
      continue;
    }
  }
  this->numberOfPotentialMemoryDependences +=
      this->computePotentialEdges(totLoads, totStores, totCalls);

  /*
   * Compute the total number of memory dependences between instructions within
   * the context of loops.
   */
  totLoads = 0;
  totStores = 0;
  totCalls = 0;
  if (programLoops.find(&F) != programLoops.end()) {
    auto loopForest = programLoops[&F];
    for (auto loopTree : loopForest->getTrees()) {
      auto visitor = [&totLoads,
                      &totStores,
                      &totCalls](LoopTree *n, uint32_t level) -> bool {
        auto currentLoop = n->getLoop();
        for (auto inst : currentLoop->getInstructions()) {
          if (isa<LoadInst>(inst)) {
            totLoads++;
            continue;
          }
          if (isa<StoreInst>(inst)) {
            totStores++;
            continue;
          }
          if (false || isa<CallInst>(inst) || isa<InvokeInst>(inst)) {
            totCalls++;
            continue;
          }
        }
        return false;
      };
      loopTree->visitPreOrder(visitor);
    }
  }
  this->numberOfPotentialMemoryDependences +=
      this->computePotentialEdges(totLoads, totStores, totCalls);

  return;
}

void PDGStats::printRefinedLoopGraphsForFunction(
    Noelle &noelle,
    std::unordered_map<Function *, LoopForest *> &programLoops,
    std::unordered_map<LoopStructure *, LoopContent *> &lsToLC,
    Function &F) {
  auto loopCount = 0;
  /*
   * Check every loop of the program.
   */
  if (programLoops.find(&F) != programLoops.end()) {
    auto loopForest = programLoops[&F];
    for (auto loopTree : loopForest->getTrees()) {
      auto visitor = [this, &lsToLC, &loopCount, &F](LoopTree *n,
                                                     uint32_t level) -> bool {
        /*
         * Fetch the loop.
         */
        auto currentLoop = n->getLoop();
        auto currentLoopContent = lsToLC[currentLoop];
        assert(currentLoopContent != nullptr);

        /*
         * Fetch the loop dependence graph.
         */
        auto loopDG = currentLoopContent->getLoopDG();

        std::string filename;
        raw_string_ostream ros(filename);
        ros << "pdg-function-" << F.getName() << "-loop" << loopCount
            << "-refined.dot";
        DGPrinter::writeClusteredGraph<PDG, Value>(ros.str(), loopDG);

        loopCount++;

        return false;
      };
      loopTree->visitPreOrder(visitor);
    }
  }

  return;
}

void PDGStats::collectStatsForLoopEdges(
    Noelle &noelle,
    std::unordered_map<Function *, LoopForest *> &programLoops,
    std::unordered_map<LoopStructure *, LoopContent *> &lsToLC,
    Function &F) {

  /*
   * Check every loop of the program.
   */
  if (programLoops.find(&F) != programLoops.end()) {
    auto loopForest = programLoops[&F];
    for (auto loopTree : loopForest->getTrees()) {
      auto visitor = [this, &lsToLC](LoopTree *n, uint32_t level) -> bool {
        /*
         * Fetch the loop.
         */
        auto currentLoop = n->getLoop();
        auto currentLoopContent = lsToLC[currentLoop];
        assert(currentLoopContent != nullptr);

        /*
         * Fetch the loop dependence graph.
         */
        auto loopDG = currentLoopContent->getLoopDG();

        /*
         * Iterate over the dependences.
         */
        for (auto edge : loopDG->getEdges()) {
          this->analyzeDependence(edge);
        }

        return false;
      };
      loopTree->visitPreOrder(visitor);
    }
  }

  return;
}

void PDGStats::collectSCCStats(Noelle &noelle) {
  auto LSs = noelle.getLoopStructures();

  for (auto LS : *LSs) {
    auto LC = noelle.getLoopContent(LS);
    auto manager = LC->getSCCManager();
    auto SCCNodes = manager->getSCCDAG()->getSCCs();
    for (auto sccNode : SCCNodes) {
      auto scc = manager->getSCCAttrs(sccNode);
      this->sccHistogram[scc->getKind()]++;
      this->numberOfSCCs++;
    }
  }
}

bool PDGStats::edgeIsDependenceOf(MDNode *edgeM,
                                  const EDGE_ATTRIBUTE edgeAttribute) {
  if (MDNode *m = dyn_cast<MDNode>(edgeM->getOperand(edgeAttribute))) {
    if (MDString *s = dyn_cast<MDString>(m->getOperand(0))) {
      return s->getString() == "true" ? true : false;
    }
  }

  assert(false && "Error fetching edge attribute from Metadata");
}

void PDGStats::printStats() {
  errs() << "Number of Nodes: " << this->numberOfNodes << "\n";
  errs() << "Number of Edges (a.k.a. dependences): " << this->numberOfEdges
         << "\n";
  errs()
      << "  Number of control dependences: " << this->numberOfControlDependence
      << "\n";
  errs() << "  Number of data dependences: "
         << this->numberOfEdges - this->numberOfControlDependence << "\n";
  errs() << "    Number of variable dependences: "
         << this->numberOfVariableDependence << "\n";
  errs()
      << "    Number of memory dependences: " << this->numberOfMemoryDependence
      << "\n";
  errs() << "      Number of memory must dependences: "
         << this->numberOfMemoryMustDependence << "\n";
  errs() << "      Number of memory may dependences: "
         << this->numberOfMemoryDependence - this->numberOfMemoryMustDependence
         << "\n";
  errs() << "      Number of potential memory dependences: "
         << this->numberOfPotentialMemoryDependences << "\n";

  printSCCStats();

  return;
}

void PDGStats::printSCCStats() {
  errs() << "Number of SCCs: " << this->numberOfSCCs << "\n";
  for (const auto &[kind, counts] : this->sccHistogram) {
    if (counts != 0) {
      errs() << "  Number of SCC of type " << toString(kind) << ": " << counts
             << "\n";
    }
  }
  errs() << "  Number of SCC of any other type: 0\n";
}

std::string PDGStats::toString(GenericSCC::SCCKind sccKind) {
  switch (sccKind) {
    case GenericSCC::LOOP_CARRIED:
      return "LOOP_CARRIED";
    case GenericSCC::REDUCTION:
      return "REDUCTION";
    case GenericSCC::BINARY_REDUCTION:
      return "BINARY_REDUCTION";
    case GenericSCC::LAST_REDUCTION:
      return "LAST_REDUCTION";
    case GenericSCC::RECOMPUTABLE:
      return "RECOMPUTABLE";
    case GenericSCC::SINGLE_ACCUMULATOR_RECOMPUTABLE:
      return "SINGLE_ACCUMULATOR_RECOMPUTABLE";
    case GenericSCC::INDUCTION_VARIABLE:
      return "INDUCTION_VARIABLE";
    case GenericSCC::LINEAR_INDUCTION_VARIABLE:
      return "LINEAR_INDUCTION_VARIABLE";
    case GenericSCC::LAST_INDUCTION_VARIABLE:
      return "LAST_INDUCTION_VARIABLE";
    case GenericSCC::PERIODIC_VARIABLE:
      return "PERIODIC_VARIABLE";
    case GenericSCC::LAST_SINGLE_ACCUMULATOR_RECOMPUTABLE:
      return "LAST_SINGLE_ACCUMULATOR_RECOMPUTABLE";
    case GenericSCC::UNKNOWN_CLOSED_FORM:
      return "UNKNOWN_CLOSED_FORM";
    case GenericSCC::LAST_RECOMPUTABLE:
      return "LAST_RECOMPUTABLE";
    case GenericSCC::MEMORY_CLONABLE:
      return "MEMORY_CLONABLE";
    case GenericSCC::STACK_OBJECT_CLONABLE:
      return "STACK_OBJECT_CLONABLE";
    case GenericSCC::LAST_MEMORY_CLONABLE:
      return "LAST_MEMORY_CLONABLE";
    case GenericSCC::LOOP_CARRIED_UNKNOWN:
      return "LOOP_CARRIED_UNKNOWN";
    case GenericSCC::LAST_LOOP_CARRIED:
      return "LAST_LOOP_CARRIED";
    case GenericSCC::LOOP_ITERATION:
      return "LOOP_ITERATION";
    case GenericSCC::LAST_LOOP_ITERATION:
      return "LAST_LOOP_ITERATION";
    default:
      assert(false);
  }
}

PDGStats::PDGStats() : ModulePass{ ID } {
  return;
}

uint64_t PDGStats::computePotentialEdges(uint64_t totLoads,
                                         uint64_t totStores,
                                         uint64_t totCalls) {
  uint64_t tot = 0;

  /*
   * Add the total number of dependences that could exist between memory
   * instructions.
   */
  tot += (totStores * totStores);
  tot += (totLoads * totStores * 2);

  /*
   * Add the total number of dependences that could exist between the call
   * instructions. Notice that two call instructions could have RAW, WAW, and
   * WAR. This is why each pair could have 3 dependences.
   */
  tot += (totCalls * totCalls * 3);

  /*
   * Add the total number of dependences between call and memory instructions.
   */
  tot += (totCalls * totStores * 3);
  tot += (totCalls * totLoads * 2);

  return tot;
}

void PDGStats::analyzeDependence(DGEdge<Value, Value> *edge) {
  this->numberOfEdges++;

  /*
   * Handle memory dependences.
   */
  if (isa<MemoryDependence<Value, Value>>(edge)) {
    this->numberOfMemoryDependence++;
    if (isa<MustMemoryDependence<Value, Value>>(edge)) {
      this->numberOfMemoryMustDependence++;
    }
    return;
  }

  /*
   * Handle variable dependences.
   */
  if (isa<DataDependence<Value, Value>>(edge)) {
    this->numberOfVariableDependence++;
    return;
  }

  /*
   * Handle control dependences.
   */
  if (isa<ControlDependence<Value, Value>>(edge)) {
    this->numberOfControlDependence++;
    return;
  }

  return;
}

PDGStats::~PDGStats() {
  return;
}

} // namespace arcana::noelle
