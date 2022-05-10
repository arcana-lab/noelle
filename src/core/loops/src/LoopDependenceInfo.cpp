/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni, Brian Homerding
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/Architecture.hpp"
#include "noelle/core/PDG.hpp"
#include "noelle/core/PDGAnalysis.hpp"
#include "noelle/core/SCCDAG.hpp"
#include "noelle/core/LoopDependenceInfo.hpp"
#include "LoopAwareMemDepAnalysis.hpp"

namespace llvm::noelle {

LoopDependenceInfo::LoopDependenceInfo (
  PDG *fG,
  StayConnectedNestedLoopForestNode *loopNode,
  Loop *l,
  DominatorSummary &DS,
  ScalarEvolution &SE
) : LoopDependenceInfo{fG, loopNode, l, DS, SE, Architecture::getNumberOfLogicalCores(), true, {}, true} 
  {
  return ;
}

LoopDependenceInfo::LoopDependenceInfo(
  PDG *fG,
  StayConnectedNestedLoopForestNode *loopNode,
  Loop *l,
  DominatorSummary &DS,
  ScalarEvolution &SE,
  uint32_t maxCores,
  bool enableFloatAsReal
) : LoopDependenceInfo{fG, loopNode, l, DS, SE, maxCores, enableFloatAsReal, {}, true} {

  return ;
}

LoopDependenceInfo::LoopDependenceInfo(
  PDG *fG,
  StayConnectedNestedLoopForestNode *loopNode,
  Loop *l,
  DominatorSummary &DS,
  ScalarEvolution &SE,
  uint32_t maxCores,
  bool enableFloatAsReal,
  std::unordered_set<LoopDependenceInfoOptimization> optimizations
) : LoopDependenceInfo{fG, loopNode, l, DS, SE, maxCores, enableFloatAsReal, optimizations, true} {

  return ;
}

LoopDependenceInfo::LoopDependenceInfo (
  PDG *fG,
  StayConnectedNestedLoopForestNode *loopNode,
  Loop *l,
  DominatorSummary &DS,
  ScalarEvolution &SE,
  uint32_t maxCores,
  bool enableFloatAsReal,
  bool enableLoopAwareDependenceAnalyses
) : LoopDependenceInfo{fG, loopNode, l, DS, SE, maxCores, enableFloatAsReal, {}, enableLoopAwareDependenceAnalyses}{

  return ;
}

LoopDependenceInfo::LoopDependenceInfo(
  PDG *fG,
  StayConnectedNestedLoopForestNode *loopNode,
  Loop *l,
  DominatorSummary &DS,
  ScalarEvolution &SE,
  uint32_t maxCores,
  bool enableFloatAsReal,
  std::unordered_set<LoopDependenceInfoOptimization> optimizations,
  bool enableLoopAwareDependenceAnalyses
) : LoopDependenceInfo(fG, loopNode, l, DS, SE, maxCores, enableFloatAsReal, optimizations, enableLoopAwareDependenceAnalyses, 8)
{
  return ;
}

LoopDependenceInfo::LoopDependenceInfo(
  PDG *fG,
  StayConnectedNestedLoopForestNode *loopNode,
  Loop *l,
  DominatorSummary &DS,
  ScalarEvolution &SE,
  uint32_t maxCores,
  bool enableFloatAsReal,
  std::unordered_set<LoopDependenceInfoOptimization> optimizations,
  bool enableLoopAwareDependenceAnalyses,
  uint32_t chunkSize
) :   loop{loopNode}
    , memoryCloningAnalysis{nullptr}
  {
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
  this->loopTransformationsManager = new LoopTransformationsManager(maxCores, chunkSize, optimizations, enableLoopAwareDependenceAnalyses);

  /*
   * Enable all transformations.
   */
  this->loopTransformationsManager->enableAllTransformations();

  /*
   * Fetch the loop dependence graph (i.e., the subset of the PDG that relates to the loop @l) and its SCCDAG.
   */
  this->fetchLoopAndBBInfo(l, SE);
  auto ls = this->getLoopStructure();
  auto loopExitBlocks = ls->getLoopExitBasicBlocks();
  auto DGs = this->createDGsForLoop(l, loopNode, fG, DS, SE);
  this->loopDG = DGs.first;
  auto loopSCCDAG = DGs.second;

  /*
   * Create the environment for the loop.
   *
   * Exclude stack objects that will be cloned. To do so, we need to collect this set of objects.
   */
  std::set<Value *> stackObjectsThatWillBeCloned;
  if (this->memoryCloningAnalysis != nullptr){
    for (auto memObject : this->memoryCloningAnalysis->getClonableMemoryLocations()){
      auto stackObject = memObject->getAllocation();
      stackObjectsThatWillBeCloned.insert(stackObject);
    }
  }
  this->environment = new LoopEnvironment(loopDG, loopExitBlocks, stackObjectsThatWillBeCloned);

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
  auto loopSCCDAGWithoutMemoryDeps = this->computeSCCDAGWithOnlyVariableAndControlDependences(loopDG);
  this->inductionVariables = new InductionVariableManager(this->loop, *invariantManager, SE, *loopSCCDAGWithoutMemoryDeps, *environment, *l);

  /*
   * Calculate various attributes on SCCs
   */
  this->sccdagAttrs = new SCCDAGAttrs(enableFloatAsReal, loopDG, loopSCCDAG, this->loop, SE, *inductionVariables, DS);
  this->domainSpaceAnalysis = new LoopIterationDomainSpaceAnalysis(this->loop, *this->inductionVariables, SE);

  /*
   * Collect induction variable information
   */
  auto iv = this->inductionVariables->getLoopGoverningInductionVariable(*topLoop);
  loopGoverningIVAttribution = iv == nullptr ? nullptr
    : new LoopGoverningIVAttribution(*iv, *loopSCCDAG->sccOfValue(iv->getLoopEntryPHI()), loopExitBlocks);

  return ;
}

void LoopDependenceInfo::copyParallelizationOptionsFrom (LoopDependenceInfo *otherLDI) {
  auto otherLTM = otherLDI->getLoopTransformationsManager();
  assert(otherLTM != nullptr);

  /*
   * Free the memory.
   */
  delete this->loopTransformationsManager ;

  /*
   * Clone the loop transformation manager
   */
  this->loopTransformationsManager = new LoopTransformationsManager(*otherLTM);

  return ;
}

void LoopDependenceInfo::fetchLoopAndBBInfo (
  Loop *l,
  ScalarEvolution &SE
  ){

  /*
   * Compute the trip counts of all loops in the loop tree that starts with @l.
   */
  auto loopTripCount = this->computeTripCounts(l, SE);
  if (loopTripCount > 0){
    this->compileTimeKnownTripCount = true;
    this->tripCount = loopTripCount;

  } else {
    this->compileTimeKnownTripCount = false;
    this->tripCount = 0;
  }

  return ;
}

uint64_t LoopDependenceInfo::computeTripCounts (
  Loop *l,
  ScalarEvolution &SE
  ){

  /*
   * Fetch the trip count of the loop given as input.
   */
  auto tripCount = SE.getSmallConstantTripCount(l);

  return tripCount;
}

std::pair<PDG *, SCCDAG *> LoopDependenceInfo::createDGsForLoop (
  Loop *l,
  StayConnectedNestedLoopForestNode *loopNode,
  PDG *functionDG,
  DominatorSummary &DS,
  ScalarEvolution &SE
) {

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

  std::vector<Value *> loopInternals;
  for (auto internalNode : loopDG->internalNodePairs()) {
      loopInternals.push_back(internalNode.first);
  }

  /*
   * Compute the SCCDAG using only variable-related dependences.
   * This will be used to detect induction variables.
   */
  auto loopInternalDG = loopDG->createSubgraphFromValues(loopInternals, false);
  auto loopSCCDAGWithoutMemoryDeps = this->computeSCCDAGWithOnlyVariableAndControlDependences(loopInternalDG);

  /*
   * Detect the loop-carried data dependences.
   *
   * HACK: The reason LoopCarriedDependencies is constructed SPECIFICALLY with the DG
   * that is used to query it is because it holds references to edges copied to that specific
   * instance of the DG. Edges are NOT referential to a single DG source.
   * When they are, this won't need to be done
   *
   * HACK: The SCCDAG is constructed with a loop internal DG to avoid external nodes in the loop DG
   * which provide context (live-ins/live-outs) but which complicate analyzing the resulting SCCDAG 
   */
  LoopCarriedDependencies::setLoopCarriedDependencies(loopNode, DS, *loopDG);

  /*
   * Detect loop invariants and induction variables.
   */
  auto loopStructure = loopNode->getLoop();
  auto loopExitBlocks = loopStructure->getLoopExitBasicBlocks();
  auto env = LoopEnvironment(loopDG, loopExitBlocks, {});
  auto invManager = InvariantManager(loopStructure, loopDG);
  auto ivManager = InductionVariableManager(loopNode, invManager, SE, *loopSCCDAGWithoutMemoryDeps, env, *l); 

  /*
   * Perform loop-aware memory dependence analysis to refine the loop dependence graph.
   */
  auto domainSpace = LoopIterationDomainSpaceAnalysis(loopNode, ivManager, SE);
  if (this->loopTransformationsManager->areLoopAwareAnalysesEnabled()){
    refinePDGWithLoopAwareMemDepAnalysis(loopDG, l, loopStructure, loopNode, &domainSpace);
  }

  /*
   * Analyze the loop to identify opportunities of cloning stack objects.
   */
  if (this->loopTransformationsManager->isOptimizationEnabled(LoopDependenceInfoOptimization::MEMORY_CLONING_ID)){
    this->removeUnnecessaryDependenciesThatCloningMemoryNegates(loopNode, loopDG, DS);
  }

  /*
   * Remove memory dependences with known thread-safe library functions.
   */
  if (this->loopTransformationsManager->isOptimizationEnabled(LoopDependenceInfoOptimization::THREAD_SAFE_LIBRARY_ID)){
    this->removeUnnecessaryDependenciesWithThreadSafeLibraryFunctions(loopNode, loopDG, DS);
  }

  /*
   * Build a SCCDAG of loop-internal instructions
   */
  loopInternalDG = loopDG->createSubgraphFromValues(loopInternals, false);
  auto loopSCCDAG = new SCCDAG(loopInternalDG);

  /*
   * Safety check: check that the SCCDAG includes all instructions of the loop given as input.
   */
  #ifdef DEBUG

  /*
   * Check that all loop instructions belong to LDI-specific containers.
   */
  {
  int64_t numberOfInstructionsInLoop = 0;
  for (auto bbIter : l->blocks()){
    for (auto &I : *bbIter){
      assert(std::find(loopInternals.begin(), loopInternals.end(), &I) != loopInternals.end());
      assert(loopInternalDG->isInternal(&I));
      assert(loopSCCDAG->doesItContain(&I));
      numberOfInstructionsInLoop++;
    }
  }

  /*
   * Check that all LDI-specific containers include only loop instructions.
   */
  assert(loopInternals.size() == numberOfInstructionsInLoop);
  assert(loopInternalDG->numNodes() == loopInternals.size());
  }
  #endif

  return std::make_pair(loopDG, loopSCCDAG);
}

void LoopDependenceInfo::removeUnnecessaryDependenciesWithThreadSafeLibraryFunctions (
  StayConnectedNestedLoopForestNode *loopNode,
  PDG *loopDG,
  DominatorSummary &DS
){

  /*
   * Fetch the loop sub-tree rooted at @this.
   */
  auto rootLoop = loopNode->getLoop();

  /*
   * Identify the dependences to remove.
   */
  std::unordered_set<DGEdge<Value> *> edgesToRemove;
  for (auto edge : LoopCarriedDependencies::getLoopCarriedDependenciesForLoop(*rootLoop, loopNode, *loopDG)) {

    /*
     * Only memory dependences can be removed.
     */
    if (!edge->isMemoryDependence()) {
      continue;
    }

    /*
     * Only dependences between instructions can be removed.
     */
    auto producer = dyn_cast<Instruction>(edge->getOutgoingT());
    auto consumer = dyn_cast<Instruction>(edge->getIncomingT());
    if (!producer || !consumer) {
      continue;
    }

    /*
     * Only self-dependences can be removed.
     */
    if (producer != consumer){
      continue ;
    }

    /*
     * Only dependences with thread-safe library functions can be removed.
     */
    if (auto producerCall = dyn_cast<CallInst>(producer)){
      auto callee = producerCall->getCalledFunction();
      if (callee != nullptr){
        if (PDGAnalysis::isTheLibraryFunctionThreadSafe(callee)){
          edgesToRemove.insert(edge);
          continue ;
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

  return ;
}

void LoopDependenceInfo::removeUnnecessaryDependenciesThatCloningMemoryNegates (
  StayConnectedNestedLoopForestNode *loopNode,
  PDG *loopInternalDG,
  DominatorSummary &DS
) {

  /*
   * Fetch the loop sub-tree rooted at @this.
   */
  auto rootLoop = loopNode->getLoop();

  /*
   * Create the memory cloning analyzer.
   */
  this->memoryCloningAnalysis = new MemoryCloningAnalysis(rootLoop, DS, loopInternalDG);

  /*
   * Identify opportunities for cloning stack locations.
   */
  std::unordered_set<DGEdge<Value> *> edgesToRemove;
  for (auto edge : LoopCarriedDependencies::getLoopCarriedDependenciesForLoop(*rootLoop, loopNode, *loopInternalDG)) {

    /*
     * Only memory dependences can be removed by cloning memory objects.
     */
    if (!edge->isMemoryDependence()) {
      continue;
    }

    /*
     * Only dependences between instructions can be removed by cloning memory objects.
     */
    auto producer = dyn_cast<Instruction>(edge->getOutgoingT());
    auto consumer = dyn_cast<Instruction>(edge->getIncomingT());
    if (!producer || !consumer) {
      continue;
    }

    auto locationsProducer = this->memoryCloningAnalysis->getClonableMemoryLocationsFor(producer);
    auto locationsConsumer = this->memoryCloningAnalysis->getClonableMemoryLocationsFor(consumer);
    if (locationsProducer.empty() || locationsConsumer.empty()) {
      continue;
    }

    bool isRAW = false;
    for (auto locationP : locationsProducer) {
      for (auto locationC : locationsConsumer) {
        if (edge->isRAWDependence() && 
            locationP->isInstructionStoringLocation(producer) && 
            locationC->isInstructionLoadingLocation(consumer))
          isRAW = true;
      }
    }
    bool isWAR = false;
    for (auto locationP : locationsProducer) {
      for (auto locationC : locationsConsumer) {
        if (edge->isWARDependence() && 
            locationP->isInstructionLoadingLocation(producer) && 
            locationC->isInstructionStoringLocation(consumer))
          isWAR = true;
      }
    }
    bool isWAW = false;
    for (auto locationP : locationsProducer) {
      for (auto locationC : locationsConsumer) {
        if (edge->isWAWDependence() && 
            locationP->isInstructionStoringLocation(producer) && 
            locationC->isInstructionStoringLocation(consumer))
          isWAW = true;
      }
    }

    if (!isRAW && !isWAR && !isWAW) {
      continue;
    }
    // producer->print(errs() << "Found alloca location for producer: "); errs() << "\n";
    // consumer->print(errs() << "Found alloca location for consumer: "); errs() << "\n";
    // locationProducer->getAllocation()->print(errs() << "Alloca: "); errs() << "\n";
    // locationConsumer->getAllocation()->print(errs() << "Alloca: "); errs() << "\n";

    edgesToRemove.insert(edge);
  }

  /*
   * Remove the dependences.
   */
  for (auto edge : edgesToRemove) {
    edge->setLoopCarried(false);
    loopInternalDG->removeEdge(edge);
  }

  return ;
}
 
PDG * LoopDependenceInfo::getLoopDG (void) const {
  return this->loopDG;
}

bool LoopDependenceInfo::iterateOverSubLoopsRecursively (
  std::function<bool (const LoopStructure &child)> funcToInvoke
  ){

  /*
   * Iterate over the children.
   */
  for (auto subloop : this->loop->getLoops()){
    if (funcToInvoke(*subloop)){
      return true ;
    }
  }

  return false;
}

uint64_t LoopDependenceInfo::getID (void) const {

  /*
   * Fetch the loop structure.
   */
  auto ls = this->getLoopStructure();

  /*
   * Fetch the ID.
   */
  auto ID = ls->getID();

  return ID;
}

LoopStructure * LoopDependenceInfo::getLoopStructure (void) const {
  return this->loop->getLoop();
}

LoopStructure * LoopDependenceInfo::getNestedMostLoopStructure (Instruction *I) const {
  return this->loop->getInnermostLoopThatContains(I);
}

bool LoopDependenceInfo::isSCCContainedInSubloop (SCC *scc) const {
  return this->sccdagAttrs->isSCCContainedInSubloop(this->loop, scc);
}

InductionVariableManager * LoopDependenceInfo::getInductionVariableManager (void) const {
  return inductionVariables;
}

LoopGoverningIVAttribution * LoopDependenceInfo::getLoopGoverningIVAttribution (void) const {
  return loopGoverningIVAttribution;
}

MemoryCloningAnalysis * LoopDependenceInfo::getMemoryCloningAnalysis (void) const {
  assert(this->memoryCloningAnalysis != nullptr
    && "Requesting memory cloning analysis without having specified LoopDependenceInfoOptimization::MEMORY_CLONING");
  return this->memoryCloningAnalysis;
}

bool LoopDependenceInfo::doesHaveCompileTimeKnownTripCount (void) const {
  return this->compileTimeKnownTripCount;
}

uint64_t LoopDependenceInfo::getCompileTimeTripCount (void) const {
  return this->tripCount;
}

InvariantManager * LoopDependenceInfo::getInvariantManager (void) const {
  return this->invariantManager;
}

LoopIterationDomainSpaceAnalysis * LoopDependenceInfo::getLoopIterationDomainSpaceAnalysis (void) const {
  return this->domainSpaceAnalysis;
}

StayConnectedNestedLoopForestNode * LoopDependenceInfo::getLoopHierarchyStructures (void) const {
  return this->loop;
}

SCCDAGAttrs * LoopDependenceInfo::getSCCManager (void) const {
  return this->sccdagAttrs;
}
      
LoopEnvironment * LoopDependenceInfo::getEnvironment (void) const {
  return this->environment;
}

SCCDAG * LoopDependenceInfo::computeSCCDAGWithOnlyVariableAndControlDependences (
  PDG *loopDG
  ){

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
  std::unordered_set<DGEdge<Value> *> memDeps{};
  for (auto currentDependence : loopDG->getSortedDependences()){
    if (currentDependence->isMemoryDependence()){
      memDeps.insert(currentDependence);
    }
  }

  /*
   * Compute the new loop dependence graph
   */
  auto loopDGWithoutMemoryDeps = loopDG->createSubgraphFromValues(loopInternals, false, memDeps);

  /*
   * Compute the SCCDAG
   */
  auto loopSCCDAGWithoutMemoryDeps = new SCCDAG(loopDGWithoutMemoryDeps);

  return loopSCCDAGWithoutMemoryDeps;
}
      
LoopTransformationsManager * LoopDependenceInfo::getLoopTransformationsManager (void) const {
  return this->loopTransformationsManager;
}

LoopDependenceInfo::~LoopDependenceInfo() {
  delete this->loopDG;
  delete this->environment;

  if (this->inductionVariables){
    delete this->inductionVariables;
  }
  if (this->loopGoverningIVAttribution){
    delete this->loopGoverningIVAttribution;
  }

  assert(this->invariantManager);
  delete this->invariantManager;

  delete this->domainSpaceAnalysis;

  return ;
}

}
