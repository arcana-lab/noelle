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
#include "arcana/noelle/core/Architecture.hpp"
#include "arcana/noelle/core/PDG.hpp"
#include "arcana/noelle/core/PDGGenerator.hpp"
#include "arcana/noelle/core/SCCDAG.hpp"
#include "arcana/noelle/core/LoopContent.hpp"
#include "arcana/noelle/core/LoopCarriedDependencies.hpp"

namespace arcana::noelle {

LoopContent::LoopContent(LDGGenerator &ldgGenerator,
                         CompilationOptionsManager *compilationOptionsManager,
                         PDG *fG,
                         LoopTree *loopNode,
                         Loop *l,
                         DominatorSummary &DS,
                         ScalarEvolution &SE)
  : LoopContent{ ldgGenerator,
                 compilationOptionsManager,
                 fG,
                 loopNode,
                 l,
                 DS,
                 SE,
                 Architecture::getNumberOfLogicalCores(),
                 {},
                 true } {
  return;
}

LoopContent::LoopContent(LDGGenerator &ldgGenerator,
                         CompilationOptionsManager *compilationOptionsManager,
                         PDG *fG,
                         LoopTree *loopNode,
                         Loop *l,
                         DominatorSummary &DS,
                         ScalarEvolution &SE,
                         uint32_t maxCores)
  : LoopContent{ ldgGenerator,
                 compilationOptionsManager,
                 fG,
                 loopNode,
                 l,
                 DS,
                 SE,
                 maxCores,
                 {},
                 true } {

  return;
}

LoopContent::LoopContent(
    LDGGenerator &ldgGenerator,
    CompilationOptionsManager *compilationOptionsManager,
    PDG *fG,
    LoopTree *loopNode,
    Loop *l,
    DominatorSummary &DS,
    ScalarEvolution &SE,
    uint32_t maxCores,
    std::unordered_set<LoopContentOptimization> optimizations)
  : LoopContent{ ldgGenerator,
                 compilationOptionsManager,
                 fG,
                 loopNode,
                 l,
                 DS,
                 SE,
                 maxCores,
                 optimizations,
                 true } {

  return;
}

LoopContent::LoopContent(LDGGenerator &ldgGenerator,
                         CompilationOptionsManager *compilationOptionsManager,
                         PDG *fG,
                         LoopTree *loopNode,
                         Loop *l,
                         DominatorSummary &DS,
                         ScalarEvolution &SE,
                         uint32_t maxCores,
                         bool enableLoopAwareDependenceAnalyses)
  : LoopContent{ ldgGenerator,
                 compilationOptionsManager,
                 fG,
                 loopNode,
                 l,
                 DS,
                 SE,
                 maxCores,
                 {},
                 enableLoopAwareDependenceAnalyses } {

  return;
}

LoopContent::LoopContent(
    LDGGenerator &ldgGenerator,
    CompilationOptionsManager *compilationOptionsManager,
    PDG *fG,
    LoopTree *loopNode,
    Loop *l,
    DominatorSummary &DS,
    ScalarEvolution &SE,
    uint32_t maxCores,
    std::unordered_set<LoopContentOptimization> optimizations,
    bool enableLoopAwareDependenceAnalyses)
  : LoopContent(ldgGenerator,
                compilationOptionsManager,
                fG,
                loopNode,
                l,
                DS,
                SE,
                maxCores,
                optimizations,
                enableLoopAwareDependenceAnalyses,
                8) {
  return;
}

LoopContent::LoopContent(
    LDGGenerator &ldgGenerator,
    CompilationOptionsManager *compilationOptionsManager,
    PDG *fG,
    LoopTree *loopNode,
    Loop *l,
    DominatorSummary &DS,
    ScalarEvolution &SE,
    uint32_t maxCores,
    std::unordered_set<LoopContentOptimization> optimizations,
    bool enableLoopAwareDependenceAnalyses,
    uint32_t chunkSize)
  : loop{ loopNode },
    memoryCloningAnalysis{ nullptr },
    com{ compilationOptionsManager } {
  assert(this->loop != nullptr);

  /*
   * Assertions.
   */
  for (auto edge : fG->getEdges()) {
    assert(!edge->isLoopCarriedDependence() && "Flag was already set");
  }

  /*
   * Create the loop transformations manager
   */
  this->loopTransformationsManager =
      new LoopTransformationsManager(maxCores,
                                     chunkSize,
                                     optimizations,
                                     enableLoopAwareDependenceAnalyses);

  /*
   * Enable all transformations.
   */
  this->loopTransformationsManager->enableAllTransformations();

  /*
   * Fetch the loop dependence graph (i.e., the subset of the PDG that relates
   * to the loop @l) and its SCCDAG.
   */
  this->fetchLoopAndBBInfo(l, SE);
  auto ls = this->getLoopStructure();
  auto loopExitBlocks = ls->getLoopExitBasicBlocks();
  auto DGs = this->createDGsForLoop(ldgGenerator,
                                    compilationOptionsManager,
                                    l,
                                    loopNode,
                                    fG,
                                    DS,
                                    SE);
  this->loopDG = DGs.first;
  auto loopSCCDAG = DGs.second;

  /*
   * Create the environment for the loop.
   *
   * Exclude stack objects that will be cloned. To do so, we need to collect
   * this set of objects.
   */
  std::set<Value *> stackObjectsThatWillBeCloned;
  if (this->memoryCloningAnalysis != nullptr) {
    for (auto memObject :
         this->memoryCloningAnalysis->getClonableMemoryObjects()) {

      /*
       * Check if the stack object needs to be initialized.
       * If it does, then we need to have the original stack object as live-in.
       */
      if (memObject->doPrivateCopiesNeedToBeInitialized()) {
        continue;
      }

      /*
       * The stack object does not need to be initialized.
       * So, we can avoid having the pointer to the original stack object as
       * live-in.
       */
      auto stackObject = memObject->getAllocation();
      stackObjectsThatWillBeCloned.insert(stackObject);
    }
  }
  this->environment =
      new LoopEnvironment(loopDG, loopExitBlocks, stackObjectsThatWillBeCloned);

  /*
   * Create the invariant manager.
   *
   * This step identifies instructions that are loop invariants.
   */
  auto topLoop = this->loop->getLoop();
  this->invariantManager = new InvariantManager(topLoop, this->loopDG);

  /*
   * Create the induction variable manager.
   *
   * This step identifies IVs.
   *
   * First, we need to compute the LDG that doesn't include memory dependences.
   * Memory dependences don't matter for the IV detection.
   * Then, we compute the SCCDAG of this sub-LDG.
   * And then, we can identify IVs from this new SCCDAG.
   */
  auto loopSCCDAGWithoutMemoryDeps =
      ldgGenerator.computeSCCDAGWithOnlyVariableAndControlDependences(loopDG);
  this->inductionVariables =
      new InductionVariableManager(this->loop,
                                   *invariantManager,
                                   SE,
                                   *loopSCCDAGWithoutMemoryDeps,
                                   *environment,
                                   *l);

  /*
   * Calculate various attributes on SCCs
   */
  this->sccdagAttrs = new SCCDAGAttrs(
      compilationOptionsManager->canFloatsBeConsideredRealNumbers(),
      loopDG,
      loopSCCDAG,
      this->loop,
      *inductionVariables,
      DS);
  this->domainSpaceAnalysis =
      new LoopIterationSpaceAnalysis(this->loop, *this->inductionVariables, SE);

  /*
   * Collect induction variable information
   */
  this->inductionVariables->getLoopGoverningInductionVariable(*topLoop);

  return;
}

void LoopContent::copyParallelizationOptionsFrom(LoopContent *otherLC) {
  auto otherLTM = otherLC->getLoopTransformationsManager();
  assert(otherLTM != nullptr);

  /*
   * Free the memory.
   */
  delete this->loopTransformationsManager;

  /*
   * Clone the loop transformation manager
   */
  this->loopTransformationsManager = new LoopTransformationsManager(*otherLTM);

  return;
}

void LoopContent::fetchLoopAndBBInfo(Loop *l, ScalarEvolution &SE) {

  /*
   * Compute the trip counts of all loops in the loop tree that starts with @l.
   */
  auto loopTripCount = this->computeTripCounts(l, SE);
  if (loopTripCount > 0) {
    this->compileTimeKnownTripCount = true;
    this->tripCount = loopTripCount;

  } else {
    this->compileTimeKnownTripCount = false;
    this->tripCount = 0;
  }

  return;
}

uint64_t LoopContent::computeTripCounts(Loop *l, ScalarEvolution &SE) {

  /*
   * Fetch the trip count of the loop given as input.
   */
  auto tripCount = SE.getSmallConstantTripCount(l);

  return tripCount;
}

std::pair<PDG *, SCCDAG *> LoopContent::createDGsForLoop(
    LDGGenerator &ldgGenerator,
    CompilationOptionsManager *com,
    Loop *l,
    LoopTree *loopNode,
    PDG *functionDG,
    DominatorSummary &DS,
    ScalarEvolution &SE) {

  /*
   * Perform loop-aware memory dependence analysis to refine the loop dependence
   * graph.
   */
  auto loopDG = ldgGenerator.generateLoopDependenceGraph(functionDG,
                                                         SE,
                                                         DS,
                                                         com,
                                                         l,
                                                         *loopNode);

  /*
   * Analyze the loop to identify opportunities of cloning stack objects.
   */
  if (this->loopTransformationsManager->isOptimizationEnabled(
          LoopContentOptimization::MEMORY_CLONING_ID)) {
    this->removeUnnecessaryDependenciesThatCloningMemoryNegates(loopNode,
                                                                loopDG,
                                                                DS);
  }

  /*
   * Remove memory dependences with known thread-safe library functions.
   */
  if (this->loopTransformationsManager->isOptimizationEnabled(
          LoopContentOptimization::THREAD_SAFE_LIBRARY_ID)) {
    this->removeUnnecessaryDependenciesWithThreadSafeLibraryFunctions(loopNode,
                                                                      loopDG,
                                                                      DS);
  }

  /*
   * Build a SCCDAG of loop-internal instructions
   */
  auto loopInternalDG = loopDG->clone(false);
  auto loopSCCDAG = new SCCDAG(loopInternalDG);

  /*
   * Safety check: check that the SCCDAG includes all instructions of the loop
   * given as input.
   */
#ifdef DEBUG

  /*
   * Check that all loop instructions belong to LC-specific containers.
   */
  {
    int64_t numberOfInstructionsInLoop = 0;
    for (auto bbIter : l->blocks()) {
      for (auto &I : *bbIter) {
        assert(std::find(loopInternals.begin(), loopInternals.end(), &I)
               != loopInternals.end());
        assert(loopInternalDG->isInternal(&I));
        assert(loopSCCDAG->doesItContain(&I));
        numberOfInstructionsInLoop++;
      }
    }

    /*
     * Check that all LC-specific containers include only loop instructions.
     */
    assert(loopInternals.size() == numberOfInstructionsInLoop);
    assert(loopInternalDG->numNodes() == loopInternals.size());
  }
#endif

  return std::make_pair(loopDG, loopSCCDAG);
}

void LoopContent::removeUnnecessaryDependenciesWithThreadSafeLibraryFunctions(
    LoopTree *loopNode,
    PDG *loopDG,
    DominatorSummary &DS) {

  /*
   * Fetch the loop sub-tree rooted at @this.
   */
  auto rootLoop = loopNode->getLoop();

  /*
   * Identify the dependences to remove.
   */
  std::unordered_set<DGEdge<Value, Value> *> edgesToRemove;
  for (auto edge :
       LoopCarriedDependencies::getLoopCarriedDependenciesForLoop(*rootLoop,
                                                                  loopNode,
                                                                  *loopDG)) {

    /*
     * Only memory dependences can be removed.
     */
    if (!isa<MemoryDependence<Value, Value>>(edge)) {
      continue;
    }

    /*
     * Only dependences between instructions can be removed.
     */
    auto producer = dyn_cast<Instruction>(edge->getSrc());
    auto consumer = dyn_cast<Instruction>(edge->getDst());
    if (!producer || !consumer) {
      continue;
    }

    /*
     * Only self-dependences can be removed.
     */
    if (producer != consumer) {
      continue;
    }

    /*
     * Only dependences with thread-safe library functions can be removed.
     */
    if (auto producerCall = dyn_cast<CallInst>(producer)) {
      auto callee = producerCall->getCalledFunction();
      if (callee != nullptr) {
        if (PDGGenerator::isTheLibraryFunctionThreadSafe(callee)) {
          edgesToRemove.insert(edge);
          continue;
        }
      }
    }
  }

  /*
   * Removed the identified dependences.
   */
  for (auto edge : edgesToRemove) {
    edge->setLoopCarried(false);
    loopDG->removeEdge(edge);
  }

  return;
}

void LoopContent::removeUnnecessaryDependenciesThatCloningMemoryNegates(
    LoopTree *loopNode,
    PDG *loopInternalDG,
    DominatorSummary &DS) {

  /*
   * Fetch the loop sub-tree rooted at @this.
   */
  auto rootLoop = loopNode->getLoop();

  /*
   * Create the memory cloning analyzer.
   */
  this->memoryCloningAnalysis =
      new MemoryCloningAnalysis(rootLoop, DS, loopInternalDG);

  /*
   * Identify opportunities for cloning stack locations.
   */
  std::unordered_set<DGEdge<Value, Value> *> edgesToRemove;
  for (auto edge : LoopCarriedDependencies::getLoopCarriedDependenciesForLoop(
           *rootLoop,
           loopNode,
           *loopInternalDG)) {

    /*
     * Only memory dependences can be removed by cloning memory objects.
     */
    if (!isa<MemoryDependence<Value, Value>>(edge)) {
      continue;
    }
    auto memoryDep = cast<MemoryDependence<Value, Value>>(edge);

    /*
     * Only dependences between instructions can be removed by cloning memory
     * objects.
     */
    auto producer = dyn_cast<Instruction>(memoryDep->getSrc());
    auto consumer = dyn_cast<Instruction>(memoryDep->getDst());
    if (!producer || !consumer) {
      continue;
    }
    auto locationsProducer =
        this->memoryCloningAnalysis->getClonableMemoryObjectsFor(producer);
    auto locationsConsumer =
        this->memoryCloningAnalysis->getClonableMemoryObjectsFor(consumer);
    if (locationsProducer.empty() || locationsConsumer.empty()) {
      continue;
    }

    auto isRAW = false;
    for (auto locationP : locationsProducer) {
      for (auto locationC : locationsConsumer) {
        if (memoryDep->isRAWDependence()
            && locationP->isInstructionStoringLocation(producer)
            && locationC->isInstructionLoadingLocation(consumer))
          isRAW = true;
      }
    }
    auto isWAR = false;
    for (auto locationP : locationsProducer) {
      for (auto locationC : locationsConsumer) {
        if (memoryDep->isWARDependence()
            && locationP->isInstructionLoadingLocation(producer)
            && locationC->isInstructionStoringLocation(consumer))
          isWAR = true;
      }
    }
    auto isWAW = false;
    for (auto locationP : locationsProducer) {
      for (auto locationC : locationsConsumer) {
        if (memoryDep->isWAWDependence()
            && locationP->isInstructionStoringLocation(producer)
            && locationC->isInstructionStoringLocation(consumer))
          isWAW = true;
      }
    }

    if (!isRAW && !isWAR && !isWAW) {
      continue;
    }
    // producer->print(errs() << "Found alloca location for producer: "); errs()
    // << "\n"; consumer->print(errs() << "Found alloca location for consumer:
    // "); errs() << "\n"; locationProducer->getAllocation()->print(errs() <<
    // "Alloca: "); errs() << "\n";
    // locationConsumer->getAllocation()->print(errs() << "Alloca: "); errs() <<
    // "\n";

    edgesToRemove.insert(memoryDep);
  }

  /*
   * Remove the dependences.
   */
  for (auto edge : edgesToRemove) {
    edge->setLoopCarried(false);
    loopInternalDG->removeEdge(edge);
  }

  return;
}

PDG *LoopContent::getLoopDG(void) const {
  return this->loopDG;
}

bool LoopContent::iterateOverSubLoopsRecursively(
    std::function<bool(const LoopStructure &child)> funcToInvoke) {

  /*
   * Iterate over the children.
   */
  for (auto subloop : this->loop->getLoops()) {
    if (funcToInvoke(*subloop)) {
      return true;
    }
  }

  return false;
}

LoopStructure *LoopContent::getLoopStructure(void) const {
  return this->loop->getLoop();
}

LoopStructure *LoopContent::getNestedMostLoopStructure(Instruction *I) const {
  return this->loop->getInnermostLoopThatContains(I);
}

InductionVariableManager *LoopContent::getInductionVariableManager(void) const {
  return inductionVariables;
}

MemoryCloningAnalysis *LoopContent::getMemoryCloningAnalysis(void) const {
  assert(
      this->memoryCloningAnalysis != nullptr
      && "Requesting memory cloning analysis without having specified LoopContentOptimization::MEMORY_CLONING");
  return this->memoryCloningAnalysis;
}

bool LoopContent::doesHaveCompileTimeKnownTripCount(void) const {
  return this->compileTimeKnownTripCount;
}

uint64_t LoopContent::getCompileTimeTripCount(void) const {
  return this->tripCount;
}

InvariantManager *LoopContent::getInvariantManager(void) const {
  return this->invariantManager;
}

LoopIterationSpaceAnalysis *LoopContent::getLoopIterationSpaceAnalysis(
    void) const {
  return this->domainSpaceAnalysis;
}

LoopTree *LoopContent::getLoopHierarchyStructures(void) const {
  return this->loop;
}

SCCDAGAttrs *LoopContent::getSCCManager(void) const {
  return this->sccdagAttrs;
}

LoopEnvironment *LoopContent::getEnvironment(void) const {
  return this->environment;
}

LoopTransformationsManager *LoopContent::getLoopTransformationsManager(
    void) const {
  return this->loopTransformationsManager;
}

LoopContent::~LoopContent() {
  delete this->loopDG;
  delete this->environment;

  if (this->inductionVariables) {
    delete this->inductionVariables;
  }

  assert(this->invariantManager);
  delete this->invariantManager;

  delete this->domainSpaceAnalysis;

  return;
}

} // namespace arcana::noelle
