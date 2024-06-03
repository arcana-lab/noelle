/*
 * Copyright 2016 - 2024  Angelo Matni, Simone Campanoni, Brian Homerding
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
#include "noelle/core/DataFlow.hpp"
#include "noelle/core/LoopCarriedDependencies.hpp"
#include "noelle/core/LoopAliasAnalysisEngine.hpp"
#include "noelle/core/LoopContent.hpp"
#include "LoopAwareMemDepAnalysis.hpp"

/*
 * SCAF headers
 */
#ifdef NOELLE_ENABLE_SCAF
#  include "scaf/MemoryAnalysisModules/LoopAA.h"
#  include "scaf/Utilities/PDGQueries.h"
#  include "scaf/Utilities/ModuleLoops.h"
#endif

namespace arcana::noelle {

/*
 * SCAF
 */
#ifdef NOELLE_ENABLE_SCAF
static liberty::LoopAA *NoelleSCAFAA = nullptr;
static liberty::ModuleLoops *ModuleLoops = nullptr;
#endif

class NoelleSCAFIntegration : public ModulePass {
public:
  static char ID;

  NoelleSCAFIntegration();
  bool doInitialization(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnModule(Module &M) override;
};

// Next there is code to register your pass to "opt"
char NoelleSCAFIntegration::ID = 0;
static RegisterPass<NoelleSCAFIntegration> X("noelle-scaf",
                                             "Integration with SCAF");

void refinePDGWithLoopAwareMemDepAnalysis(LDGGenerator &ldgAnalysis,
                                          PDG *loopDG,
                                          LoopTree &loops,
                                          InductionVariableManager &ivManager,
                                          ScalarEvolution &SE) {
  if (ldgAnalysis.areLoopDependenceAnalysesEnabled()) {
    refinePDGWithSCAF(loopDG, loops);
  }

  /*
   * Run the loop-centric data dependence analyses.
   */
  ldgAnalysis.generateLoopDependenceGraph(loopDG, SE, ivManager, loops);

  return;
}

void refinePDGWithSCAF(PDG *loopDG, LoopTree &loopNode) {
#ifdef NOELLE_ENABLE_SCAF
  assert(NoelleSCAFAA != nullptr);

  /*
   * Get the LLVM loop for SCAF.
   */
  auto loopStructure = loopNode.getLoop();
  auto li = &ModuleLoops->getAnalysis_LoopInfo(loopStructure->getFunction());
  auto l = li->getLoopFor(loopStructure->getHeader());

  /*
   * Iterate over all the edges of the loop PDG and collect memory deps to be
   * queried. For each pair of instructions with a memory dependence map it to
   * a small vector of found edges (0th element is for RAW, 1st for WAW, 2nd for
   * WAR)
   */
  std::map<std::pair<Instruction *, Instruction *>,
           SmallVector<DGEdge<Value, Value> *, 3>>
      memDeps;
  for (auto edge : make_range(loopDG->begin_edges(), loopDG->end_edges())) {

    /*
     * Skip dependences that are not between instructions of the target loop
     */
    if (!loopDG->isInternal(edge->getDst())
        || !loopDG->isInternal(edge->getSrc())) {
      continue;
    }

    /*
     * If the dependence is not via memory, then SCAF cannot help.
     */
    if (!isa<MemoryDependence<Value, Value>>(edge)) {
      continue;
    }
    auto memDep = cast<MemoryDependence<Value, Value>>(edge);

    /*
     * Fetch the instructions involved in the dependence.
     */
    auto pdgValueI = memDep->getSrc();
    auto i = dyn_cast<Instruction>(pdgValueI);
    assert(i && "Expecting an instruction as the value of a PDG node");

    auto pdgValueJ = memDep->getDst();
    auto j = dyn_cast<Instruction>(pdgValueJ);
    assert(j && "Expecting an instruction as the value of a PDG node");

    if (!memDeps.count({ i, j })) {
      memDeps[{ i, j }] = { nullptr, nullptr, nullptr };
    }

    if (memDep->isRAWDependence()) {
      memDeps[{ i, j }][0] = memDep;
    } else if (memDep->isWAWDependence()) {
      memDeps[{ i, j }][1] = memDep;
    } else if (memDep->isWARDependence()) {
      memDeps[{ i, j }][2] = memDep;
    }
  }

  /*
   * For each memory depedence perform loop-aware dependence analysis to
   * disprove it. Queries for loop-carried and intra-iteration deps.
   */
  for (auto memDep : memDeps) {

    /*
     * Fetch the current pair of instructions
     */
    auto instPair = memDep.first;
    auto i = instPair.first;
    auto j = instPair.second;

    /*
     * Fetch the dependences.
     */
    auto edges = memDep.second;

    // encode the found dependences in a bit vector.
    // set least significant bit for RAW, 2nd bit for WAW, 3rd bit for WAR
    uint8_t depTypes = 0;
    for (uint8_t i = 0; i <= 2; ++i) {
      if (edges[i]) {
        depTypes |= 1 << i;
      }
    }
    // Try to disprove all the reported loop-carried deps
    uint8_t disprovedLCDepTypes =
        disproveLoopCarriedMemoryDep(i, j, depTypes, l, NoelleSCAFAA);

    // for every disproved loop-carried dependence
    // check if there is a intra-iteration dependence
    uint8_t disprovedIIDepTypes = 0;
    if (disprovedLCDepTypes) {
      disprovedIIDepTypes = disproveIntraIterationMemoryDep(i,
                                                            j,
                                                            disprovedLCDepTypes,
                                                            l,
                                                            NoelleSCAFAA);

      // remove any edge that SCAF disproved both its loop-carried and
      // intra-iteration version
      for (uint8_t i = 0; i <= 2; ++i) {
        if (disprovedIIDepTypes & (1 << i)) {
          auto e = edges[i];
          loopDG->removeEdge(e);
        }
      }

      // set LoopCarried bit false for all the non-disproved intra-iteration
      // edges (but were not loop-carried)
      uint8_t iiDepTypes = disprovedLCDepTypes - disprovedIIDepTypes;
      for (uint8_t i = 0; i <= 2; ++i) {
        if (iiDepTypes & (1 << i)) {
          auto e = edges[i];
          e->setLoopCarried(false);
        }
      }
    }
  }
#endif

  return;
}

NoelleSCAFIntegration::NoelleSCAFIntegration() : ModulePass{ ID } {
  return;
}

bool NoelleSCAFIntegration::doInitialization(Module &M) {
  return false;
}

void NoelleSCAFIntegration::getAnalysisUsage(AnalysisUsage &AU) const {
#ifdef NOELLE_ENABLE_SCAF
  AU.addRequired<liberty::LoopAA>();
  AU.addRequired<liberty::ModuleLoops>();
  AU.setPreservesAll();
#endif
  return;
}

bool NoelleSCAFIntegration::runOnModule(Module &M) {
#ifdef NOELLE_ENABLE_SCAF
  NoelleSCAFAA = getAnalysis<liberty::LoopAA>().getTopAA();
  ModuleLoops = &getAnalysis<liberty::ModuleLoops>();
  ModuleLoops->reset();
#endif

  return false;
}

std::set<AliasAnalysisEngine *> LoopContent::getLoopAliasAnalysisEngines(void) {
  std::set<AliasAnalysisEngine *> s;

#ifdef NOELLE_ENABLE_SCAF
  assert(NoelleSCAFAA != nullptr);
  auto aa = new LoopAliasAnalysisEngine("SCAF", NoelleSCAFAA);
  s.insert(aa);
#endif

  return s;
}

} // namespace arcana::noelle
