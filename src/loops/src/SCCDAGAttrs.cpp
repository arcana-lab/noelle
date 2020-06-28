/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SCCDAGAttrs.hpp"
#include "PDGPrinter.hpp"

using namespace llvm;

SCCDAGAttrs::SCCDAGAttrs (
  PDG *loopDG,
  SCCDAG *loopSCCDAG,
  LoopsSummary &LIS,
  ScalarEvolution &SE,
  LoopCarriedDependencies &LCD,
  InductionVariableManager &IV
) : loopDG{loopDG}, sccdag{loopSCCDAG} {

  /*
   * Partition dependences between intra-iteration and iter-iteration ones.
   */
  collectLoopCarriedDependencies(LIS, LCD);

  /*
   * Collect flattened list of all IVs at all loop levels
   */
  std::set<InductionVariable *> ivs;
  std::set<InductionVariable *> loopGoverningIVs;
  for (auto &LS : LIS.loops) {
    auto loop = LS.get();
    auto loopIVs = IV.getInductionVariables(*loop);
    ivs.insert(loopIVs.begin(), loopIVs.end());
    auto loopGoverningIV = IV.getLoopGoverningInductionVariable(*loop);
    if (loopGoverningIV) loopGoverningIVs.insert(loopGoverningIV);
  }

  // errs() << "IVs: " << ivs.size() << "\n";
  // for (auto iv : ivs) {
  //   iv->getHeaderPHI()->print(errs() << "IV: "); errs() << "\n";
  // }
  // errs() << "-------------\n";
  // errs() << "Loop governing IVs: " << loopGoverningIVs.size() << "\n";
  // for (auto iv : loopGoverningIVs) {
  //   iv->getHeaderPHI()->print(errs() << "IV: "); errs() << "\n";
  // }
  // errs() << "-------------\n";

  /*
   * Tag SCCs depending on their characteristics.
   */
  loopSCCDAG->iterateOverSCCs([this, &SE, &LIS, &ivs, &loopGoverningIVs, &LCD](SCC *scc) -> bool {

    /*
     * Allocate the metadata about this SCC.
     */
    auto sccInfo = new SCCAttrs(scc, this->accumOpInfo, LIS);
    this->sccToInfo[scc] = sccInfo;

    /*
     * Collect information about the current SCC.
     */
    bool doesSCCOnlyContainIV = this->checkIfSCCOnlyContainsInductionVariables(scc, LIS, ivs, loopGoverningIVs);
    sccInfo->setSCCToBeInductionVariable(doesSCCOnlyContainIV);

    this->checkIfClonable(scc, SE, LIS);

    /*
     * Tag the current SCC.
     */
    if (this->checkIfIndependent(scc)) {
      sccInfo->setType(SCCAttrs::SCCType::INDEPENDENT);
    } else if (this->checkIfReducible(scc, LIS, LCD)) {
      sccInfo->setType(SCCAttrs::SCCType::REDUCIBLE);
    } else {
      sccInfo->setType(SCCAttrs::SCCType::SEQUENTIAL);
    }

    return false;
  });

  collectSCCGraphAssumingDistributedClones();

  return ;
}

std::set<SCC *> SCCDAGAttrs::getSCCsWithLoopCarriedDependencies (void) const {
  std::set<SCC *> sccs;
  for (auto &sccDependencies : this->sccToInternalLoopCarriedDependencies) {
    sccs.insert(sccDependencies.first);
  }
  return sccs;
}
      
std::set<SCC *> SCCDAGAttrs::getSCCsWithLoopCarriedControlDependencies (void) const {
  std::set<SCC *> sccs;

  /*
   * Iterate over SCCs with loop-carried data dependences.
   */
  for (auto &sccDependencies : this->sccToInternalLoopCarriedDependencies) {

    /*
     * Fetch the SCC.
     */
    auto SCC = sccDependencies.first;

    /*
     * Fetch the set of loop-carried data dependences of the current SCC.
     */
    auto &deps = sccDependencies.second;

    /*
     * Check if this SCC has a control loop-carried data dependence.
     */
    auto isControl = false;
    for (auto dep : deps){
      if (dep->isControlDependence()){
        isControl = true;
        break ;
      }
    }
    if (isControl){
      sccs.insert(sccDependencies.first);
    }
  }

  return sccs;
}

std::set<SCC *> SCCDAGAttrs::getSCCsWithLoopCarriedDataDependencies (void) const {
  std::set<SCC *> sccs;

  /*
   * Iterate over SCCs with loop-carried data dependences.
   */
  for (auto &sccDependencies : this->sccToInternalLoopCarriedDependencies) {

    /*
     * Fetch the SCC.
     */
    auto SCC = sccDependencies.first;

    /*
     * Fetch the set of loop-carried data dependences of the current SCC.
     */
    auto &deps = sccDependencies.second;

    /*
     * Check if this SCC has data loop-carried data dependence.
     */
    auto isData = false;
    for (auto dep : deps){
      if (dep->isDataDependence()){
        isData = true;
        break ;
      }
    }
    if (isData){
      sccs.insert(SCC);
    }
  }
  return sccs;
}

bool SCCDAGAttrs::isLoopGovernedBySCC (SCC *governingSCC) const {
  auto topLevelNodes = this->sccdag->getTopLevelNodes();

  /*
   * Step 1: Isolate top level SCCs (excluding independent instructions in SCCDAG)
   */
  std::queue<DGNode<SCC> *> toTraverse;
  for (auto node : topLevelNodes) {
    toTraverse.push(node);
  }
  std::set<SCC *> topLevelSCCs;
  while (!toTraverse.empty()) {

    /*
     * Fetch the current SCC and its metadata.
     */
    auto node = toTraverse.front();
    auto scc = node->getT();
    toTraverse.pop();
    auto sccInfo = this->getSCCAttrs(scc);

    if (sccInfo->canExecuteIndependently()) {
      auto nextDepth = this->sccdag->getNextDepthNodes(node);
      for (auto next : nextDepth) toTraverse.push(next);
      continue;
    }
    topLevelSCCs.insert(scc);
  }

  /*
   * Step 2: Ensure there is only 1, and that it is the target SCC
   */
  if (topLevelSCCs.size() != 1) return false;
  auto topLevelSCC = *topLevelSCCs.begin();
  return topLevelSCC == governingSCC;
}

bool SCCDAGAttrs::areAllLiveOutValuesReducable (LoopEnvironment *env) const {

  /*
   * Iterate over live-out variables.
   */
  for (auto envIndex : env->getEnvIndicesOfLiveOutVars()) {

    /*
     * Fetch the SCC that contains the producer of the environment variable.
     */
    auto producer = env->producerAt(envIndex);
    auto scc = this->sccdag->sccOfValue(producer);

    /*
     * Check the SCC type.
     */
    auto sccInfo = this->getSCCAttrs(scc);
    if (sccInfo->getType() == SCCAttrs::SCCType::INDEPENDENT) {
      continue ;
    }
    if (sccInfo->getType() == SCCAttrs::SCCType::REDUCIBLE) {
      continue ;
    }

    return false;
  }

  return true;
}

bool SCCDAGAttrs::isSCCContainedInSubloop (const LoopsSummary &LIS, SCC *scc) const {
  auto instInSubloops = true;
  auto topLoop = LIS.getLoopNestingTreeRoot();
  for (auto iNodePair : scc->internalNodePairs()) {
    if (auto inst = dyn_cast<Instruction>(iNodePair.first)) {
      instInSubloops &= (LIS.getLoop(*inst) != topLoop);
    } else {
      instInSubloops = false;
    }
  }

  return instInSubloops;
}

SCCAttrs * SCCDAGAttrs::getSCCAttrs (SCC *scc) const {
  auto sccInfo = this->sccToInfo.find(scc);
  if (sccInfo == this->sccToInfo.end()){
    return nullptr;
  }
  return sccInfo->second;
}

void SCCDAGAttrs::collectSCCGraphAssumingDistributedClones () {
  auto addIncomingNodes = [&](std::queue<DGNode<SCC> *> &queue, DGNode<SCC> *node) -> void {
    std::set<DGNode<SCC> *> nodes;
    auto scc = node->getT();
    for (auto edge : node->getIncomingEdges()) {
      nodes.insert(edge->getOutgoingNode());
      this->edgesViaClones[scc].insert(edge);
    }
    for (auto node : nodes) {
      queue.push(node);
    }
  };

  for (auto childSCCNode : this->sccdag->getNodes()) {
    auto childSCC = childSCCNode->getT();
    std::queue<DGNode<SCC> *> nodesToCheck;
    std::unordered_map<DGNode<SCC> *, bool> analyzed;

    analyzed[childSCCNode] = true;
    addIncomingNodes(nodesToCheck, childSCCNode);
    
    while (!nodesToCheck.empty()) {
      auto node = nodesToCheck.front();
      nodesToCheck.pop();
      auto scc = node->getT();
      auto sccInfo = this->getSCCAttrs(scc);
      this->parentsViaClones[childSCC].insert(scc);
      if (!sccInfo->canBeCloned()) {
        continue ;
      }
      if (analyzed[node]){
        continue ;
      }
      addIncomingNodes(nodesToCheck, node);
      analyzed[node] = true;
    }
  }

  return ;
}

void SCCDAGAttrs::collectLoopCarriedDependencies (LoopsSummary &LIS, LoopCarriedDependencies &LCD) {

  for (auto &loop : LIS.loops) {

    auto &loopRef = *loop.get();
    auto loopCarriedEdges = LCD.getLoopCarriedDependenciesForLoop(*loop.get());

    for (auto edge : loopCarriedEdges) {

      auto producer = edge->getOutgoingT();
      auto consumer = edge->getIncomingT();
      auto producerSCC = this->sccdag->sccOfValue(producer);
      auto consumerSCC = this->sccdag->sccOfValue(consumer);

      sccToLoopCarriedDependencies[producerSCC].insert(edge);

      if (producerSCC != consumerSCC) continue;
      sccToInternalLoopCarriedDependencies[producerSCC].insert(edge);

    }
  }

  return ;
}

bool SCCDAGAttrs::checkIfSCCOnlyContainsInductionVariables (
  SCC *scc,
  LoopsSummary &LIS,
  std::set<InductionVariable *> &IVs,
  std::set<InductionVariable *> &loopGoverningIVs) {

  /*
   * Identify contained induction variables
   */
  std::set<InductionVariable *> containedIVs;
  std::set<Instruction *> containedInsts;
  for (auto iv : IVs) {
    if (scc->isInternal(iv->getLoopEntryPHI())) {
      containedIVs.insert(iv);
      auto allInsts = iv->getAllInstructions();
      containedInsts.insert(allInsts.begin(), allInsts.end());
    }
  }
  if (containedIVs.size() == 0) return false;

  /*
   * If a contained IV is loop governing, ensure loop governance is well formed
   */
  for (auto containedIV : containedIVs) {
    if (loopGoverningIVs.find(containedIV) == loopGoverningIVs.end()) continue;
    auto exitBlocks = LIS.getLoop(*containedIV->getLoopEntryPHI()->getParent())->getLoopExitBasicBlocks();
    LoopGoverningIVAttribution attribution(*containedIV, *scc, exitBlocks);
    if (!attribution.isSCCContainingIVWellFormed()) {
      // errs() << "Not well formed SCC for loop governing IV!\n";
      return false;
    }
    containedInsts.insert(attribution.getHeaderCmpInst());
    containedInsts.insert(attribution.getHeaderBrInst());
    auto conditionValue = attribution.getHeaderCmpInstConditionValue();
    if (isa<Instruction>(conditionValue)) containedInsts.insert(cast<Instruction>(conditionValue));
    auto conditionDerivation = attribution.getConditionValueDerivation();
    containedInsts.insert(conditionDerivation.begin(), conditionDerivation.end());
  }

  /*
   * NOTE: No side effects can be contained in the SCC; only instructions of the IVs
   */
  for (auto nodePair : scc->internalNodePairs()) {
    auto value = nodePair.first;
    if (auto inst = dyn_cast<Instruction>(value)) {
      if (containedInsts.find(inst) != containedInsts.end()) continue;

      if (auto br = dyn_cast<BranchInst>(value)) {
        if (br->isUnconditional()) continue;
      } else if (isa<GetElementPtrInst>(inst) || isa<PHINode>(inst) || isa<CastInst>(inst)) {
        continue;
      }
    }

    // value->print(errs() << "Suspect value: "); errs() << "\n";
    // for (auto containedI : containedInsts) {
    //   containedI->print(errs() << "Contained: "); errs() << "\n";
    // }
    return false;
  }

  return true;
}

bool SCCDAGAttrs::checkIfReducible (SCC *scc, LoopsSummary &LIS, LoopCarriedDependencies &LCD) {

  scc->printMinimal(errs() << "Is this reducible?\n");

  /*
   * Only one loop carried data value per SCC can be reduced
   * If there is only one, check if that Variable is reducible
   * 
   * NOTE: We don't handle memory variables yet
   */
  auto rootLoop = LIS.getLoopNestingTreeRoot();
  PHINode *singleLoopCarriedPHI = nullptr;
  for (auto dependency : sccToInternalLoopCarriedDependencies.at(scc)) {
    if (dependency->isMemoryDependence()) return false;

    if (dependency->isControlDependence()) continue;
    auto consumer = dependency->getIncomingT();
    assert(isa<PHINode>(consumer)
      && "All consumers of loop carried data dependencies must be PHIs");
    auto consumerPHI = cast<PHINode>(consumer);
    dependency->print(errs() << "Consumer of loop carried dep\n"); errs() << "\n";

    /*
     * Ignore sub loops as they do not need to be reduced
     */
    if (!rootLoop->isIncluded(consumerPHI)) continue;

    if (singleLoopCarriedPHI == consumerPHI) continue;
    if (singleLoopCarriedPHI) return false;

    singleLoopCarriedPHI = consumerPHI;
  }

  if (!singleLoopCarriedPHI) return false;

  Variable variable(*rootLoop, LCD, *loopDG, *scc, singleLoopCarriedPHI);
  return variable.isEvolutionReducibleAcrossLoopIterations();
}

/*
 * The SCC is independent if it doesn't have loop carried data dependencies
 */
bool SCCDAGAttrs::checkIfIndependent (SCC *scc) {
  return sccToInternalLoopCarriedDependencies.find(scc) == sccToInternalLoopCarriedDependencies.end();
}

void SCCDAGAttrs::checkIfClonable (SCC *scc, ScalarEvolution &SE, LoopsSummary &LIS) {

  /*
   * Check the simple cases.
   */
  if ( false
       || isClonableByInductionVars(scc)
       || isClonableBySyntacticSugarInstrs(scc)
       || isClonableByCmpBrInstrs(scc)
       || isClonableByHavingNoMemoryOrLoopCarriedDataDependencies(scc, LIS)
      ) {
    this->getSCCAttrs(scc)->setSCCToBeClonable();
    clonableSCCs.insert(scc);
    return ;
  }

  return ;
}

bool SCCDAGAttrs::isClonableByHavingNoMemoryOrLoopCarriedDataDependencies (SCC *scc, LoopsSummary &LIS) const {

  /*
   * FIXME: This check should not exist; instead, SCC where cloning
   * is trivial should be separated out by the parallelization scheme
   */
  if (this->sccdag->fetchNode(scc)->numOutgoingEdges() == 0) return false;

  for (auto edge : scc->getEdges()) {
    if (edge->isMemoryDependence()) return false;
  }

  if (sccToInternalLoopCarriedDependencies.find(scc) == sccToInternalLoopCarriedDependencies.end()) return true;

  auto topLoop = LIS.getLoopNestingTreeRoot();
  for (auto loopCarriedDependency : sccToInternalLoopCarriedDependencies.at(scc)) {
    auto valueFrom = loopCarriedDependency->getOutgoingT();
    auto valueTo = loopCarriedDependency->getIncomingT();
    assert(isa<Instruction>(valueFrom) && isa<Instruction>(valueTo));
    if (LIS.getLoop(*cast<Instruction>(valueFrom)) == topLoop ||
      LIS.getLoop(*cast<Instruction>(valueTo)) == topLoop) {
        return false;
    }
  }

  return true;
}

bool SCCDAGAttrs::isClonableByInductionVars (SCC *scc) const {

  /*
   * FIXME: This check should not exist; instead, SCC where cloning
   * is trivial should be separated out by the parallelization scheme
   */
  if (this->sccdag->fetchNode(scc)->numOutgoingEdges() == 0) return false;

  /*
   * Fetch the SCC metadata.
   */
  auto sccInfo = this->getSCCAttrs(scc);

  return sccInfo->isInductionVariableSCC();
}

bool SCCDAGAttrs::isClonableBySyntacticSugarInstrs (SCC *scc) const {

  /*
   * FIXME: This check should not exist; instead, SCC where cloning
   * is trivial should be separated out by the parallelization scheme
   */
  if (this->sccdag->fetchNode(scc)->numOutgoingEdges() == 0) return false;

  if (scc->numInternalNodes() > 1) return false;
  auto I = scc->begin_internal_node_map()->first;
  if (isa<PHINode>(I) || isa<GetElementPtrInst>(I) || isa<CastInst>(I)) {
    return true;
  }
  return false;
}

bool SCCDAGAttrs::isClonableByCmpBrInstrs (SCC *scc) const {
  for (auto iNodePair : scc->internalNodePairs()) {
    auto V = iNodePair.first;
    if (auto inst = dyn_cast<Instruction>(V)){
      if (isa<CmpInst>(inst) || inst->isTerminator()) {
        continue;
      }
    }
    return false;
  }
  return true;
}

bool SCCDAGAttrs::isALoopCarriedDependence (SCC *scc, DGEdge<Value> *dependence) {

  /*
   * Fetch the set of loop-carried data dependences of a SCC.
   */
  if (this->sccToLoopCarriedDependencies.find(scc) == this->sccToLoopCarriedDependencies.end()){
    return false;
  }
  auto lcDeps = this->sccToLoopCarriedDependencies[scc];

  /*
   * Check whether the dependence is inside lcDeps.
   */
  return lcDeps.find(dependence) != lcDeps.end();
}

void SCCDAGAttrs::iterateOverLoopCarriedDataDependences (
  SCC *scc, 
  std::function<bool (DGEdge<Value> *dependence)> func
  ){

  /*
   * Iterate over internal edges of the SCC.
   */
  for (auto valuePair : scc->internalNodePairs()) {
    for (auto edge : valuePair.second->getIncomingEdges()) {

      /*
       * Check if the current edge is a loop-carried data dependence.
       */
      if (!this->isALoopCarriedDependence(scc, edge)){
        continue ;
      }

      /*
       * The current edge is a loop-carried data dependence.
       */
      auto result = func(edge);

      /*
       * Check if the caller wants us to stop iterating.
       */
      if (result){
        return ;
      }
    }
  }

  return ;
}

SCCDAG * SCCDAGAttrs::getSCCDAG (void) const {
  return this->sccdag;
}

void SCCDAGAttrs::dumpToFile (int id) {
  std::error_code EC;
  std::string filename = "sccdag-attrs-loop-" + std::to_string(id) + ".dot";

  if (EC) {
    errs() << "ERROR: Could not dump debug logs to file!";
    return ;
  }

  DG<DGString> stageGraph;
  std::set<DGString *> elements;
  std::unordered_map<DGNode<SCC> *, DGNode<DGString> *> sccToDescriptionMap;

  auto addNode = [&](std::string val, bool isInternal) -> DGNode<DGString> * {
    auto element = new DGString(val);
    elements.insert(element);
    return stageGraph.addNode(element, isInternal);
  };

  for (auto sccNode : sccdag->getNodes()) {
    std::string sccDescription;
    raw_string_ostream ros(sccDescription);

    auto sccInfo = getSCCAttrs(sccNode->getT());
    ros << "Type: ";
    if (sccInfo->canExecuteIndependently()) ros << "Independent ";
    if (sccInfo->canBeCloned()) ros << "Clonable ";
    if (sccInfo->canExecuteReducibly()) ros << "Reducible ";
    if (sccInfo->isInductionVariableSCC()) ros << "IV ";
    ros << "\n";

    for (auto iNodePair : sccNode->getT()->internalNodePairs()) {
      iNodePair.first->print(ros);
      ros << "\n";
    }

    ros.flush();
    sccToDescriptionMap.insert(std::make_pair(sccNode, addNode(sccDescription, true)));
  }

  for (auto sccEdge : sccdag->getEdges()) {
    auto outgoingDesc = sccToDescriptionMap.at(sccEdge->getOutgoingNode())->getT();
    auto incomingDesc = sccToDescriptionMap.at(sccEdge->getIncomingNode())->getT();
    stageGraph.addEdge(outgoingDesc, incomingDesc);
  }

  DGPrinter::writeGraph<DG<DGString>>(filename, &stageGraph);
  for (auto elem : elements) delete elem;
}

bool SCCAttrs::mustExecuteSequentially (void) const {
  return this->getType() == SCCAttrs::SCCType::SEQUENTIAL;
}

bool SCCAttrs::canExecuteReducibly (void) const {
  return this->getType() == SCCAttrs::SCCType::REDUCIBLE;
}

bool SCCAttrs::canExecuteIndependently (void) const {
  return this->getType() == SCCAttrs::SCCType::INDEPENDENT;
}

bool SCCAttrs::canBeCloned (void) const {
  return this->isClonable;
}

bool SCCAttrs::isInductionVariableSCC (void) const {
  return this->hasIV;
}
