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
  Function *f,
  PDG *fG,
  Loop *l,
  LoopInfo &li,
  ScalarEvolution &SE
) : function{f}, DOALLChunkSize{8},
    maximumNumberOfCoresForTheParallelization{Architecture::getNumberOfPhysicalCores()}
  {

  /*
   * Enable all techniques.
   */
  this->enableAllTechniques();

  /*
   * Fetch the PDG of the loop and its SCCDAG.
   */
  this->fetchLoopAndBBInfo(li, l);
  auto DGs = this->createDGsForLoop(l, fG);
  auto loopDG = DGs.first;
  auto loopSCCDAG = DGs.second;

  /*
   * Create the environment for the loop.
   */
  this->environment = new LoopEnvironment(loopDG, this->loopExitBlocks);

  /*
   * Merge SCCs where separation is unnecessary
   * Calculate various attributes on remaining SCCs
   */
  mergeTrivialNodesInSCCDAG(loopSCCDAG);
  this->sccdagAttrs.populate(loopSCCDAG, this->liSummary, SE);

  /*
   * Free the memory.
   */
  delete loopDG;

  return ;
}

LoopDependenceInfo::LoopDependenceInfo(
  Function *f,
  PDG *fG,
  Loop *l,
  LoopInfo &li,
  ScalarEvolution &SE,
  PostDominatorTree &pdt
) : LoopDependenceInfo{f, fG, l, li, SE}
  {
  for (auto bb : l->blocks()) {
    loopBBtoPD[&*bb] = pdt.getNode(&*bb)->getIDom()->getBlock();
  }

  return ;
}

LoopDependenceInfo::~LoopDependenceInfo() {
  delete this->loopDG;
  delete this->environment;

  return ;
}

void LoopDependenceInfo::copyParallelizationOptionsFrom (LoopDependenceInfo *otherLDI) {
  this->DOALLChunkSize = otherLDI->DOALLChunkSize;
  this->maximumNumberOfCoresForTheParallelization = otherLDI->maximumNumberOfCoresForTheParallelization;
  this->enabledTechniques = otherLDI->enabledTechniques;

  return ;
}

/*
 * Fetch the number of exit blocks.
 */
uint32_t LoopDependenceInfo::numberOfExits (void) const{
  return this->loopExitBlocks.size();
}

void LoopDependenceInfo::fetchLoopAndBBInfo (LoopInfo &li, Loop *l) {

  /*
   * Create a LoopInfo summary
   */
  this->liSummary.populate(li, l);

  /*
   * Set the headers.
   */
  this->header = l->getHeader();
  this->preHeader = l->getLoopPreheader();

  /*
   * Set the loop body.
   */
  for (auto bb : l->blocks()){
    this->loopBBs.push_back(&*bb);
  }

  l->getExitBlocks(loopExitBlocks);

  return ;
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
  this->loopDG = loopDG->createSubgraphFromValues(loopInternals, false);
  auto loopSCCDAG = SCCDAG::createSCCDAGFrom(this->loopDG);

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
      assert(this->loopDG->isInternal(&I));
      assert(loopSCCDAG->doesItContain(&I));
      numberOfInstructionsInLoop++;
    }
  }

  /*
   * Check that all LDI-specific containers include only loop instructions.
   */
  assert(loopInternals.size() == numberOfInstructionsInLoop);
  assert(loopDG->numNodes() == loopInternals.size());
  }
  #endif

  return make_pair(loopDG, loopSCCDAG);
}

void LoopDependenceInfo::mergeTrivialNodesInSCCDAG (SCCDAG *loopSCCDAG) {

  /*
   * Merge SCCs.
   */
  // NOTE(angelo): For the sake of brevity in logging, instead of logging
  // the before and after, which can be VERY verbose, we should just log
  // each change made by these merge helpers. This would still capture everything
  // necessary for debugging purposes.
  mergeSingleSyntacticSugarInstrs(loopSCCDAG);
  mergeBranchesWithoutOutgoingEdges(loopSCCDAG);

  return ;
}

void LoopDependenceInfo::mergeSingleSyntacticSugarInstrs (SCCDAG *loopSCCDAG) {
  std::unordered_map<DGNode<SCC> *, std::set<DGNode<SCC> *> *> mergedToGroup;
  std::set<std::set<DGNode<SCC> *> *> singles;
  for (auto sccNode : loopSCCDAG->getNodes()) {
    auto scc = sccNode->getT();

    /*
     * Determine if node is a single syntactic sugar instruction that has either
     * a single parent SCC or a single child SCC
     */
    if (scc->numInternalNodes() > 1) continue;
    auto I = scc->begin_internal_node_map()->first;
    if (!isa<PHINode>(I) && !isa<GetElementPtrInst>(I) && !isa<CastInst>(I)) continue;

    // TODO: Even if more than one edge exists, attempt next/previous depth SCCs.
    DGNode<SCC> *adjacentNode = nullptr;
    if (sccNode->numOutgoingEdges() == 1) {
      adjacentNode = (*sccNode->begin_outgoing_edges())->getIncomingNode();
    }
    if (sccNode->numIncomingEdges() == 1) {
      auto incomingOption = (*sccNode->begin_incoming_edges())->getOutgoingNode();
      if (!adjacentNode) {
        adjacentNode = incomingOption;
      } else {

        /*
         * NOTE: generally, these are lcssa PHIs, or casts of previous PHIs/instructions
         * If a GEP, it's load is in the child SCC, so leave it with the child
         */
        if (isa<PHINode>(I) || isa<CastInst>(I)) adjacentNode = incomingOption;
      }
    }
    if (!adjacentNode) continue;

    /*
     * Determine the merged state of the single instruction node and its adjacent
     * Merge the current merged nodes holding both of the above
     */
    bool mergedSCCNode = mergedToGroup.find(sccNode) != mergedToGroup.end();
    bool mergedAdjNode = mergedToGroup.find(adjacentNode) != mergedToGroup.end();
    if (mergedSCCNode && mergedAdjNode) {

      /*
       * Combine the adjacent node's merged group into that of the single instruction's merged group
       */
      auto adjSet = mergedToGroup[adjacentNode];
      for (auto node : *adjSet) {
        mergedToGroup[sccNode]->insert(node);
        mergedToGroup[node] = mergedToGroup[sccNode];
      }
      singles.erase(adjSet);
      delete adjSet;
    } else if (mergedSCCNode) {
      mergedToGroup[sccNode]->insert(adjacentNode);
      mergedToGroup[adjacentNode] = mergedToGroup[sccNode];
    } else if (mergedAdjNode) {
      mergedToGroup[adjacentNode]->insert(sccNode);
      mergedToGroup[sccNode] = mergedToGroup[adjacentNode];
    } else {
      auto nodes = new std::set<DGNode<SCC> *>({ sccNode, adjacentNode });
      singles.insert(nodes);
      mergedToGroup[sccNode] = nodes;
      mergedToGroup[adjacentNode] = nodes;
    }
  }

  for (auto sccNodes : singles) { 
    loopSCCDAG->mergeSCCs(*sccNodes);
    delete sccNodes;
  }
}
  
bool LoopDependenceInfo::isTechniqueEnabled (Technique technique){
  auto exist = this->enabledTechniques.find(technique) != this->enabledTechniques.end();

  return exist;
}

void LoopDependenceInfo::enableAllTechniques (void){
  this->enabledTechniques.insert(DOALL_ID);
  this->enabledTechniques.insert(DSWP_ID);
  this->enabledTechniques.insert(HELIX_ID);

  return ;
}

void LoopDependenceInfo::disableTechnique (Technique techniqueToDisable){
  this->enabledTechniques.erase(techniqueToDisable);

  return ;
}

void LoopDependenceInfo::mergeBranchesWithoutOutgoingEdges (SCCDAG *loopSCCDAG) {
  std::vector<DGNode<SCC> *> tailCmpBrs;
  for (auto sccNode : loopSCCDAG->getNodes()) {
    auto scc = sccNode->getT();
    if (sccNode->numIncomingEdges() == 0 || sccNode->numOutgoingEdges() > 0) continue ;

    bool allCmpOrBr = true;
    for (auto node : scc->getNodes()) {
      auto nodeValue = node->getT();

      /*
       * Handle the cmp instruction.
       */
      if (isa<CmpInst>(nodeValue)){
        allCmpOrBr &= true;
        continue ;
      }

      /*
       * Handle the branch instruction.
       */
      auto nodeInst = dyn_cast<Instruction>(nodeValue);
      if (nodeInst == nullptr){
        allCmpOrBr &= false;
        continue ;
      }
      allCmpOrBr &= nodeInst->isTerminator();
    }
    if (allCmpOrBr) tailCmpBrs.push_back(sccNode);
  }

  /*
   * Merge trailing compare/branch scc into previous depth scc
   */
  for (auto tailSCC : tailCmpBrs) {
    std::set<DGNode<SCC> *> nodesToMerge = { tailSCC };
    nodesToMerge.insert(*loopSCCDAG->getPreviousDepthNodes(tailSCC).begin());
    loopSCCDAG->mergeSCCs(nodesToMerge);
  }
}
      
PDG * LoopDependenceInfo::getLoopDG (void){
  return this->loopDG;
}

bool LoopDependenceInfo::iterateOverSubLoopsRecursively (
  std::function<bool (const LoopSummary &child)> funcToInvoke
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
  auto root = this->liSummary.getLoopNestingTreeRoot();
  return root->getID();
}
