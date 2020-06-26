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
    liSummary{}
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
  auto loopSCCDAG = DGs.second;

  /*
   * Create the environment for the loop.
   */
  this->environment = new LoopEnvironment(loopDG, loopExitBlocks);

  /*
   * Merge SCCs where separation is unnecessary
   * Calculate various attributes on remaining SCCs
   */
  LoopCarriedDependencies lcd(this->liSummary, DS, *loopSCCDAG);
  SCCDAGNormalizer normalizer(*loopSCCDAG, this->liSummary, lcd);
  normalizer.normalizeInPlace();
  inductionVariables = new InductionVariableManager(liSummary, SE, *loopSCCDAG, *environment);
  this->sccdagAttrs = SCCDAGAttrs(loopDG, loopSCCDAG, this->liSummary, SE, lcd, *inductionVariables);

  /*
   * Collect induction variable information
   */
  auto iv = inductionVariables->getLoopGoverningInductionVariable(*liSummary.getLoop(*l->getHeader()));
  loopGoverningIVAttribution = iv == nullptr ? nullptr
    : new LoopGoverningIVAttribution(*iv, *loopSCCDAG->sccOfValue(iv->getLoopEntryPHI()), loopExitBlocks);

  /*
   * Cache the post-dominator tree.
   */
  for (auto bb : l->blocks()) {
    loopBBtoPD[&*bb] = DS.PDT.getNode(&*bb)->getIDom()->getBlock();
  }

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

LoopDependenceInfo::~LoopDependenceInfo() {
  delete this->loopDG;
  delete this->environment;

  if (this->inductionVariables) delete this->inductionVariables;
  if (this->loopGoverningIVAttribution) delete this->loopGoverningIVAttribution;

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

  /*
   * Create a LoopInfo summary
   */
  this->liSummary.populate(l);

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

  return make_pair(loopDG, loopSCCDAG);
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

PDG * LoopDependenceInfo::getLoopDG (void){
  return this->loopDG;
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

bool LoopDependenceInfo::doesHaveCompileTimeKnownTripCount (void) const {
  return this->compileTimeKnownTripCount;
}

uint64_t LoopDependenceInfo::getCompileTimeTripCount (void) const {
  return this->tripCount;
}

uint32_t LoopDependenceInfo::getMaximumNumberOfCores (void) const {
  return this->maximumNumberOfCoresForTheParallelization;
}
