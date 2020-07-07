/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "Architecture.hpp"
#include "LoopDependenceInfo.hpp"

using namespace llvm;

LoopDependenceInfo::LoopDependenceInfo(
  PDG *fG,
  Loop *l,
  DominatorSummary &DS,
  ScalarEvolution &SE,
  uint32_t maxCores
) : DOALLChunkSize{8},
    maximumNumberOfCoresForTheParallelization{maxCores},
    liSummary{l},
    loopGoverningIVAttribution{nullptr}
  {

  /*
   * Enable all transformations.
   */
  this->enableAllTransformations();

  /*
   * Fetch the PDG of the loop and its SCCDAG.
   */
  this->fetchLoopAndBBInfo(l, SE);
  auto ls = getLoopStructure();
  auto loopExitBlocks = ls->getLoopExitBasicBlocks();
  auto DGs = this->createDGsForLoop(l, fG);
  this->loopDG = DGs.first;
  this->loopSCCDAG = DGs.second;

  /*
   * Create the environment for the loop.
   */
  this->environment = new LoopEnvironment(loopDG, loopExitBlocks);

  /*
   * Merge SCCs where separation is unnecessary
   * Calculate various attributes on remaining SCCs
   */
  this->loopCarriedDependencies = new LoopCarriedDependencies(this->liSummary, DS, *this->loopSCCDAG);
  SCCDAGNormalizer normalizer{*this->normalizedSCCDAG, this->liSummary, *this->loopCarriedDependencies};
  normalizer.normalizeInPlace();
  this->inductionVariables = new InductionVariableManager(liSummary, SE, *this->normalizedSCCDAG, *environment);
  this->sccdagAttrs = SCCDAGAttrs(
    loopDG,
    this->normalizedSCCDAG,
    this->liSummary,
    SE,
    *this->loopCarriedDependencies,
    *inductionVariables
  );

  /*
   * Collect induction variable information
   */
  auto loopStructure = *liSummary.getLoop(*l->getHeader());
  auto iv = this->inductionVariables->getLoopGoverningInductionVariable(loopStructure);
  if (iv != nullptr) {
    auto entryPHI = iv->getLoopEntryPHI();
    auto sccOfIV = this->normalizedSCCDAG->sccOfValue(entryPHI);
    loopGoverningIVAttribution = new LoopGoverningIVAttribution(*iv, *sccOfIV, loopExitBlocks);
  }

  /*
   * Cache the post-dominator tree.
   */
  for (auto bb : l->blocks()) {
    loopBBtoPD[&*bb] = DS.PDT.getNode(&*bb)->getIDom()->getBlock();
  }

  /*
   * Create the invariant manager.
   */
  auto topLoop = this->liSummary.getLoopNestingTreeRoot();
  this->invariantManager = new InvariantManager(topLoop, this->loopDG);

  /*
   * Fetch the metadata.
   */
  this->addMetadata("noelle.loop_ID");
  this->addMetadata("noelle.loop_optimize");
  
  return ;
}

void LoopDependenceInfo::addMetadata (const std::string &metadataName){

  /*
   * Fetch the loop summary.
   */
  auto ls = this->getLoopStructure();

  /*
   * Fetch the header terminator.
   */
  auto headerTerm = ls->getHeader()->getTerminator();

  /*
   * Fetch the metadata node.
   */
  auto metaNode = headerTerm->getMetadata(metadataName);
  if (!metaNode){
    return ;
  }

  /*
   * Fetch the string.
   */
  auto metaString = cast<MDString>(metaNode->getOperand(0))->getString();

  /*
   * Add the metadata.
   */
  this->metadata[metadataName] = metaString;

  return ;
}

void LoopDependenceInfo::copyParallelizationOptionsFrom (LoopDependenceInfo *otherLDI) {
  this->DOALLChunkSize = otherLDI->DOALLChunkSize;
  this->maximumNumberOfCoresForTheParallelization = otherLDI->maximumNumberOfCoresForTheParallelization;
  this->enabledTransformations = otherLDI->enabledTransformations;

  return ;
}

/*
 * Fetch the number of exit blocks.
 */
uint32_t LoopDependenceInfo::numberOfExits (void) const{
  return this->getLoopStructure()->getLoopExitBasicBlocks().size();
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

std::pair<PDG *, SCCDAG *> LoopDependenceInfo::createDGsForLoop (Loop *l, PDG *functionDG){

  /*
   * Set the loop dependence graph.
   */
  auto loopDG = functionDG->createLoopsSubgraph(l);

  /*
   * Build a SCCDAG of loop-internal instructions
   */
  std::vector<Value *> loopInternals;
  for (auto internalNode : loopDG->internalNodePairs()) {
      loopInternals.push_back(internalNode.first);
  }
  auto loopInternalDG = loopDG->createSubgraphFromValues(loopInternals, false);
  auto loopInternalSCCDAG = new SCCDAG(loopInternalDG);
  this->normalizedSCCDAG = new SCCDAG(loopInternalDG);

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
      assert(loopInternalSCCDAG->doesItContain(&I));
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

  return std::make_pair(loopDG, loopInternalSCCDAG);
}
  
bool LoopDependenceInfo::isTransformationEnabled (Transformation transformation){
  auto exist = this->enabledTransformations.find(transformation) != this->enabledTransformations.end();

  return exist;
}

void LoopDependenceInfo::enableAllTransformations (void){
  for (int32_t i = Transformation::First; i <= Transformation::Last; i++){
    auto t = static_cast<Transformation>(i);
    this->enabledTransformations.insert(t);
  }

  return ;
}

void LoopDependenceInfo::disableTransformation (Transformation transformationToDisable){
  this->enabledTransformations.erase(transformationToDisable);

  return ;
}

PDG * LoopDependenceInfo::getLoopDG (void) const {
  return this->loopDG;
}

SCCDAG * LoopDependenceInfo::getLoopSCCDAG (void) const {
  return this->loopSCCDAG;
}

bool LoopDependenceInfo::iterateOverSubLoopsRecursively (
  std::function<bool (const LoopStructure &child)> funcToInvoke
  ){

  /*
   * Iterate over the children.
   */
  for (auto subloop : this->liSummary.loops){
    if (funcToInvoke(*subloop)){
      return true ;
    }
  }

  return false;
}

uint64_t LoopDependenceInfo::getID (void) const {

  /*
   * Check if there is metadata.
   */
  uint64_t ID;
  if (!this->doesHaveMetadata("noelle.loop_ID")){
    abort();
  }

  /*
   * Fetch the ID from the metadata.
   */
  auto IDString = this->getMetadata("noelle.loop_ID");
  ID = std::stoul(IDString);

  return ID;
}

std::string LoopDependenceInfo::getMetadata (const std::string &metadataName) const {

  /*
   * Check if the metadata exists.
   */
  if (!this->doesHaveMetadata(metadataName)){
    return "";
  }

  return this->metadata.at(metadataName);
}

bool LoopDependenceInfo::doesHaveMetadata (const std::string &metadataName) const {
  if (this->metadata.find(metadataName) == this->metadata.end()){
    return false;
  }

  return true;
}

LoopStructure * LoopDependenceInfo::getNestedMostLoopStructure (Instruction *I) const {
  return this->liSummary.getLoop(*I);
}

LoopStructure * LoopDependenceInfo::getLoopStructure (void) const {
  return this->liSummary.getLoopNestingTreeRoot();
}

bool LoopDependenceInfo::isSCCContainedInSubloop (SCC *scc) const {
  return this->sccdagAttrs.isSCCContainedInSubloop(this->liSummary, scc);
}

InductionVariableManager * LoopDependenceInfo::getInductionVariableManager (void) const {
  return inductionVariables;
}

LoopGoverningIVAttribution * LoopDependenceInfo::getLoopGoverningIVAttribution (void) const {
  return loopGoverningIVAttribution;
}

LoopCarriedDependencies * LoopDependenceInfo::getLoopCarriedDependencies (void) const {
  return this->loopCarriedDependencies;
}

bool LoopDependenceInfo::doesHaveCompileTimeKnownTripCount (void) const {
  return this->compileTimeKnownTripCount;
}

uint64_t LoopDependenceInfo::getCompileTimeTripCount (void) const {
  return this->tripCount;
}

uint32_t LoopDependenceInfo::getMaximumNumberOfCores (void) const {
  return this->maximumNumberOfCoresForTheParallelization;
}

InvariantManager * LoopDependenceInfo::getInvariantManager (void) const {
  return this->invariantManager;
}

LoopDependenceInfo::~LoopDependenceInfo() {
  delete this->loopSCCDAG;
  delete this->normalizedSCCDAG;
  delete this->loopDG;
  delete this->environment;
  delete this->loopCarriedDependencies;

  if (this->inductionVariables){
    delete this->inductionVariables;
  }
  if (this->loopGoverningIVAttribution){
    delete this->loopGoverningIVAttribution;
  }

  assert(this->invariantManager);
  delete this->invariantManager;

  return ;
}
