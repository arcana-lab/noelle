/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni, Brian Homerding
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
#include "noelle/core/SCCDAGAttrs.hpp"
#include "noelle/core/PDGPrinter.hpp"
#include "noelle/core/BinaryReductionSCC.hpp"
#include "noelle/core/LoopIterationSCC.hpp"
#include "noelle/core/LinearInductionVariableSCC.hpp"
#include "noelle/core/StackObjectClonableSCC.hpp"
#include "noelle/core/LoopCarriedUnknownSCC.hpp"
#include "noelle/core/LoopCarriedDependencies.hpp"

namespace llvm::noelle {

SCCDAGAttrs::SCCDAGAttrs(bool enableFloatAsReal,
                         PDG *loopDG,
                         SCCDAG *loopSCCDAG,
                         LoopForestNode *loopNode,
                         InductionVariableManager &IV,
                         DominatorSummary &DS)
  : enableFloatAsReal{ enableFloatAsReal },
    loopDG{ loopDG },
    sccdag{ loopSCCDAG },
    memoryCloningAnalysis{ nullptr } {

  /*
   * Partition dependences between intra-iteration and iter-iteration ones.
   */
  this->collectLoopCarriedDependencies(loopNode);

  /*
   * Collect flattened list of all IVs at all loop levels
   */
  std::set<InductionVariable *> ivs;
  std::set<InductionVariable *> loopGoverningIVs;
  for (auto loop : loopNode->getLoops()) {
    auto loopIVs = IV.getInductionVariables(*loop);
    ivs.insert(loopIVs.begin(), loopIVs.end());
    auto loopGoverningIV = IV.getLoopGoverningInductionVariable(*loop);
    if (loopGoverningIV)
      loopGoverningIVs.insert(loopGoverningIV);
  }

  // DGPrinter::writeGraph<SCCDAG, SCC>("sccdag.dot", sccdag);
  // errs() << "IVs: " << ivs.size() << "\n";
  // for (auto iv : ivs) {
  //   iv->getLoopEntryPHI()->print(errs() << "IV: "); errs() << "\n";
  // }
  // errs() << "-------------\n";
  // errs() << "Loop governing IVs: " << loopGoverningIVs.size() << "\n";
  // for (auto iv : loopGoverningIVs) {
  //   iv->getLoopEntryPHI()->print(errs() << "IV: "); errs() << "\n";
  // }
  // errs() << "-------------\n";

  /*
   * Compute memory cloning location analysis
   */
  auto rootLoop = loopNode->getLoop();
  this->memoryCloningAnalysis = new MemoryCloningAnalysis(rootLoop, DS, loopDG);

  /*
   * Tag SCCs depending on their characteristics.
   */
  loopSCCDAG->iterateOverSCCs(
      [this, loopNode, rootLoop, &ivs, &loopGoverningIVs, &DS](
          SCC *scc) -> bool {
        /*
         * Collect information about the current SCC.
         */
        auto doesSCCOnlyContainIV =
            this->checkIfSCCOnlyContainsInductionVariables(scc,
                                                           loopNode,
                                                           ivs,
                                                           loopGoverningIVs);
        auto lcVar = this->checkIfReducible(scc, loopNode);
        auto isReducable = lcVar != nullptr;
        auto stackObjectsThatAreClonable =
            this->checkIfClonableByUsingLocalMemory(scc, loopNode);

        /*
         * Allocate the metadata about this SCC.
         */
        SCCAttrs *sccInfo = nullptr;
        if (this->checkIfIndependent(scc)) {

          /*
           * The SCC does not cross multiple loop iterations.
           */
          sccInfo = new LoopIterationSCC(scc, rootLoop);

        } else if (doesSCCOnlyContainIV.size() > 0) {

          /*
           * The SCC is an IV.
           */
          auto loopCarriedDependences =
              this->sccToLoopCarriedDependencies.at(scc);
          sccInfo = new LinearInductionVariableSCC(scc,
                                                   rootLoop,
                                                   loopCarriedDependences,
                                                   DS,
                                                   doesSCCOnlyContainIV);

        } else if (isReducable) {

          /*
           * The SCC is a reduction variable.
           */
          auto loopCarriedDependences =
              this->sccToLoopCarriedDependencies.at(scc);
          sccInfo = new BinaryReductionSCC(scc,
                                           rootLoop,
                                           loopCarriedDependences,
                                           lcVar,
                                           DS);

        } else if (stackObjectsThatAreClonable.size() > 0) {

          /*
           * The SCC can be removed by cloning stack objects.
           */
          auto loopCarriedDependences =
              this->sccToLoopCarriedDependencies.at(scc);
          sccInfo =
              new StackObjectClonableSCC(scc, rootLoop, loopCarriedDependences);
          sccInfo->addClonableMemoryLocationsContainedInSCC(
              stackObjectsThatAreClonable);

        } else {

          /*
           * The SCC crosses multiple loop iterations and we don't know how to
           * parallelize it.
           */
          auto loopCarriedDependences =
              this->sccToLoopCarriedDependencies.at(scc);
          sccInfo =
              new LoopCarriedUnknownSCC(scc, rootLoop, loopCarriedDependences);
        }
        assert(sccInfo != nullptr);
        this->sccToInfo[scc] = sccInfo;

        this->checkIfClonable(scc, loopNode);

        return false;
      });

  collectSCCGraphAssumingDistributedClones();

  return;
}

std::set<LoopCarriedSCC *> SCCDAGAttrs::getSCCsWithLoopCarriedDependencies(
    void) const {
  std::set<LoopCarriedSCC *> sccs;
  for (auto &sccDependencies : this->sccToLoopCarriedDependencies) {
    auto scc = sccDependencies.first;
    auto sccAttrs = static_cast<LoopCarriedSCC *>(this->getSCCAttrs(scc));
    sccs.insert(sccAttrs);
  }
  return sccs;
}

std::set<LoopCarriedSCC *> SCCDAGAttrs::
    getSCCsWithLoopCarriedControlDependencies(void) const {
  std::set<LoopCarriedSCC *> sccs;

  /*
   * Iterate over SCCs with loop-carried data dependences.
   */
  for (auto &sccDependencies : this->sccToLoopCarriedDependencies) {

    /*
     * Fetch the set of loop-carried data dependences of the current SCC.
     */
    auto &deps = sccDependencies.second;

    /*
     * Check if this SCC has a control loop-carried data dependence.
     */
    auto isControl = false;
    for (auto dep : deps) {
      if (dep->isControlDependence()) {
        isControl = true;
        break;
      }
    }
    if (isControl) {
      auto scc = sccDependencies.first;
      auto sccAttrs = static_cast<LoopCarriedSCC *>(this->getSCCAttrs(scc));
      sccs.insert(sccAttrs);
    }
  }

  return sccs;
}

std::set<LoopCarriedSCC *> SCCDAGAttrs::getSCCsWithLoopCarriedDataDependencies(
    void) const {
  std::set<LoopCarriedSCC *> sccs;

  /*
   * Iterate over SCCs with loop-carried data dependences.
   */
  for (auto &sccDependencies : this->sccToLoopCarriedDependencies) {

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
    for (auto dep : deps) {
      if (dep->isDataDependence()) {
        isData = true;
        break;
      }
    }
    if (isData) {
      auto sccAttrs = static_cast<LoopCarriedSCC *>(this->getSCCAttrs(SCC));
      sccs.insert(sccAttrs);
    }
  }

  return sccs;
}

bool SCCDAGAttrs::isLoopGovernedBySCC(SCC *governingSCC) const {
  auto topLevelNodes = this->sccdag->getTopLevelNodes();

  /*
   * Step 1: Isolate top level SCCs (excluding independent instructions in
   * SCCDAG)
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

    if (isa<LoopIterationSCC>(sccInfo)) {
      auto nextDepth = this->sccdag->getNextDepthNodes(node);
      for (auto next : nextDepth)
        toTraverse.push(next);
      continue;
    }
    topLevelSCCs.insert(scc);
  }

  /*
   * Step 2: Ensure there is only 1, and that it is the target SCC
   */
  if (topLevelSCCs.size() != 1)
    return false;
  auto topLevelSCC = *topLevelSCCs.begin();
  return topLevelSCC == governingSCC;
}

std::set<uint32_t> SCCDAGAttrs::getLiveOutVariablesThatAreNotReducable(
    LoopEnvironment *env) const {
  std::set<uint32_t> s;

  /*
   * Iterate over live-out variables.
   */
  for (auto envID : env->getEnvIDsOfLiveOutVars()) {

    /*
     * Fetch the SCC that contains the producer of the environment variable.
     */
    auto producer = env->getProducer(envID);
    auto scc = this->sccdag->sccOfValue(producer);

    /*
     * Check the SCC type.
     */
    auto sccInfo = this->getSCCAttrs(scc);
    if (isa<ReductionSCC>(sccInfo)) {
      continue;
    }
    if (isa<LoopIterationSCC>(sccInfo)) {
      continue;
    }

    /*
     * We found a live-out variable that cannot be reduced.
     */
    s.insert(envID);
  }

  return s;
}

bool SCCDAGAttrs::isSCCContainedInSubloop(LoopForestNode *loop,
                                          SCC *scc) const {
  auto instInSubloops = true;
  auto topLoop = loop->getLoop();
  for (auto iNodePair : scc->internalNodePairs()) {
    if (auto inst = dyn_cast<Instruction>(iNodePair.first)) {
      instInSubloops &= (loop->getInnermostLoopThatContains(inst) != topLoop);
    } else {
      instInSubloops = false;
    }
  }

  return instInSubloops;
}

SCCAttrs *SCCDAGAttrs::getSCCAttrs(SCC *scc) const {
  auto sccInfo = this->sccToInfo.find(scc);
  if (sccInfo == this->sccToInfo.end()) {
    return nullptr;
  }
  return sccInfo->second;
}

void SCCDAGAttrs::collectSCCGraphAssumingDistributedClones() {
  auto addIncomingNodes = [&](std::queue<DGNode<SCC> *> &queue,
                              DGNode<SCC> *node) -> void {
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
        continue;
      }
      if (analyzed[node]) {
        continue;
      }
      addIncomingNodes(nodesToCheck, node);
      analyzed[node] = true;
    }
  }

  return;
}

void SCCDAGAttrs::collectLoopCarriedDependencies(LoopForestNode *loopNode) {

  /*
   * Iterate over all the loops contained within the one handled by @this
   */
  for (auto loop : loopNode->getLoops()) {

    /*
     * Fetch the set of loop-carried data dependences of the current loop.
     */
    auto loopCarriedEdges =
        LoopCarriedDependencies::getLoopCarriedDependenciesForLoop(*loop,
                                                                   loopNode,
                                                                   *sccdag);

    /*
     * Make the map from SCCs to loop-carried data dependences.
     */
    for (auto edge : loopCarriedEdges) {

      /*
       * Fetch the SCCs that contain the source and destination of the current
       * loop-carried data dependence.
       */
      auto producer = edge->getOutgoingT();
      auto consumer = edge->getIncomingT();
      auto producerSCC = this->sccdag->sccOfValue(producer);
      auto consumerSCC = this->sccdag->sccOfValue(consumer);

      /*
       * Make the mapping from SCCs to dependences explicit.
       */
      sccToLoopCarriedDependencies[producerSCC].insert(edge);
      sccToLoopCarriedDependencies[consumerSCC].insert(edge);
    }
  }

  return;
}

std::set<InductionVariable *> SCCDAGAttrs::
    checkIfSCCOnlyContainsInductionVariables(
        SCC *scc,
        LoopForestNode *loopNode,
        std::set<InductionVariable *> &IVs,
        std::set<InductionVariable *> &loopGoverningIVs) const {

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
  if (containedIVs.size() == 0)
    return {};

  /*
   * If a contained IV is loop governing, ensure loop governance is well formed
   * TODO: Remove this, as this loop governing attribution isn't necessary for
   * all users of SCCDAGAttrs
   */
  for (auto containedIV : containedIVs) {
    if (loopGoverningIVs.find(containedIV) == loopGoverningIVs.end())
      continue;
    auto exitBlocks =
        loopNode->getInnermostLoopThatContains(containedIV->getLoopEntryPHI())
            ->getLoopExitBasicBlocks();
    LoopGoverningIVAttribution attribution(loopNode->getLoop(),
                                           *containedIV,
                                           *scc,
                                           exitBlocks);
    if (!attribution.isSCCContainingIVWellFormed()) {
      // containedIV->getLoopEntryPHI()->print(errs() << "Not well formed SCC
      // for loop governing IV!\n"); errs() << "\n";
      return {};
    }
    containedInsts.insert(
        attribution.getHeaderCompareInstructionToComputeExitCondition());
    containedInsts.insert(attribution.getHeaderBrInst());
    auto conditionValue = attribution.getExitConditionValue();
    if (isa<Instruction>(conditionValue))
      containedInsts.insert(cast<Instruction>(conditionValue));
    auto conditionDerivation = attribution.getConditionValueDerivation();
    containedInsts.insert(conditionDerivation.begin(),
                          conditionDerivation.end());
  }

  /*
   * NOTE: No side effects can be contained in the SCC; only instructions of the
   * IVs
   */
  for (auto nodePair : scc->internalNodePairs()) {
    auto value = nodePair.first;
    if (auto inst = dyn_cast<Instruction>(value)) {
      if (containedInsts.find(inst) != containedInsts.end())
        continue;
    }

    // value->print(errs() << "Suspect value: "); errs() << "\n";
    // for (auto containedI : containedInsts) {
    //   containedI->print(errs() << "Contained: "); errs() << "\n";
    // }
    return {};
  }

  return containedIVs;
}

LoopCarriedVariable *SCCDAGAttrs::checkIfReducible(SCC *scc,
                                                   LoopForestNode *loopNode) {

  /*
   * Check if the SCC has loop-carried dependences.
   * If not, then this SCC is not reducable because there is nothing to reduce.
   */
  if (this->sccToLoopCarriedDependencies.find(scc)
      == this->sccToLoopCarriedDependencies.end()) {
    return nullptr;
  }

  /*
   * A reducible variable consists of one loop carried value
   * that tracks the evolution of the reducible value
   */
  auto rootLoop = loopNode->getLoop();
  auto rootLoopHeader = rootLoop->getHeader();
  std::unordered_set<PHINode *> loopCarriedPHIs{};
  for (auto dependency : this->sccToLoopCarriedDependencies.at(scc)) {

    /*
     * We do not handle reducibility of memory locations
     */
    if (dependency->isMemoryDependence()) {
      return nullptr;
    }

    /*
     * Ignore external control dependencies, do not allow internal ones
     */
    auto producer = dependency->getOutgoingT();
    if (dependency->isControlDependence()) {
      if (scc->isInternal(producer)) {
        return nullptr;
      }
      continue;
    }

    /*
     * Fetch the destination of the dependence.
     */
    auto consumer = dependency->getIncomingT();
    if (!isa<PHINode>(consumer)) {

      /*
       * We do not handle SCCs with loop-carried data dependences with
       * instructions that are not PHI.
       */
      return nullptr;
    }
    assert(isa<PHINode>(consumer)
           && "All consumers of loop carried data dependencies must be PHIs");
    auto consumerPHI = cast<PHINode>(consumer);

    /*
     * Look for an internal consumer of a loop carried dependence
     *
     * NOTE: External consumers may be last-live out propagations of a reducible
     * variable or could disqualify this from reducibility: let the
     * LoopCarriedVariable analysis determine this
     */
    if (!scc->isInternal(consumerPHI)) {
      continue;
    }

    /*
     * Ignore sub-loops as they do not need to be reduced
     */
    if (rootLoopHeader != consumerPHI->getParent()) {
      continue;
    }

    loopCarriedPHIs.insert(consumerPHI);
  }

  /*
   * Check if there are loop carried dependences related to PHI nodes.
   */
  if (loopCarriedPHIs.size() != 1) {
    return nullptr;
  }
  auto singleLoopCarriedPHI = *loopCarriedPHIs.begin();

  /*
   * Analyze the loop-carried variable related to the SCC.
   */
  auto variable = new LoopCarriedVariable(*rootLoop,
                                          loopNode,
                                          *loopDG,
                                          *sccdag,
                                          *scc,
                                          singleLoopCarriedPHI);
  if (!variable->isEvolutionReducibleAcrossLoopIterations()) {
    delete variable;
    return nullptr;
  }

  /*
   * The SCC can be reduced.
   *
   * Check if the reducable variable is a floating point and check if floating
   * point variables can be considered as real numbers.
   */
  auto variableType = singleLoopCarriedPHI->getType();
  if (true && (variableType->isFloatTy() || variableType->isDoubleTy())
      && (!this->enableFloatAsReal)) {

    /*
     * Floating point values cannot be considered real numbers and therefore
     * floating point variables cannot be reduced.
     */
    return nullptr;
  }

  /*
   * This SCC can be reduced.
   */
  return variable;
}

/*
 * The SCC is independent if it doesn't have loop carried data dependencies
 */
bool SCCDAGAttrs::checkIfIndependent(SCC *scc) {
  return this->sccToLoopCarriedDependencies.find(scc)
         == this->sccToLoopCarriedDependencies.end();
}

void SCCDAGAttrs::checkIfClonable(SCC *scc, LoopForestNode *loopNode) {

  /*
   * Check the simple cases.
   * TODO: Separate out cases and catalog SCCs by those cases
   */
  if (isClonableByInductionVars(scc) || isClonableBySyntacticSugarInstrs(scc)
      || isClonableByCmpBrInstrs(scc)
      || isClonableByHavingNoMemoryOrLoopCarriedDataDependencies(scc,
                                                                 loopNode)) {
    this->getSCCAttrs(scc)->setSCCToBeClonable();
    return;
  }

  return;
}

std::set<ClonableMemoryLocation *> SCCDAGAttrs::
    checkIfClonableByUsingLocalMemory(SCC *scc,
                                      LoopForestNode *loopNode) const {

  /*
   * Ignore SCC without loop carried dependencies
   */
  if (this->sccToLoopCarriedDependencies.find(scc)
      == this->sccToLoopCarriedDependencies.end()) {
    return {};
  }

  /*
   * Ensure that loop carried dependencies belong to clonable memory locations.
   *
   * NOTE: Ignore PHIs and unconditional branch instructions
   */
  std::set<ClonableMemoryLocation *> locations;
  for (auto dependency : this->sccToLoopCarriedDependencies.at(scc)) {

    /*
     * Fetch the next loop-carried dependence.
     */
    auto depValue = dependency->getOutgoingT();
    auto inst = dyn_cast<Instruction>(depValue);
    if (!inst) {
      return {};
    }

    /*
     * Attempt to locate the instruction's clonable memory location they
     * store/load from
     */
    auto locs =
        this->memoryCloningAnalysis->getClonableMemoryLocationsFor(inst);
    // inst->print(errs() << "Instruction: "); errs() << "\n";
    // if (!location) {
    //   errs() << "No location\n";
    //   scc->print(errs() << "Getting close\n", "", 100); errs() << "\n";
    // }
    if (locs.empty()) {

      /*
       * The current loop-carried dependence cannot be removed by cloning.
       */
      return {};
    }

    /*
     * The current loop-carried dependence can be removed by cloning.
     */
    // location->getAllocation()->print(errs() << "Location found: "); errs() <<
    // "\n";
    locations.insert(locs.begin(), locs.end());
  }

  /*
   * Check if all loop-carried dependences can be removed by cloning.
   */
  if (locations.size() == 0) {
    return {};
  }

  return locations;
}

bool SCCDAGAttrs::isClonableByHavingNoMemoryOrLoopCarriedDataDependencies(
    SCC *scc,
    LoopForestNode *loopNode) const {

  /*
   * FIXME: This check should not exist; instead, SCC where cloning
   * is trivial should be separated out by the parallelization scheme
   */
  if (this->sccdag->fetchNode(scc)->numOutgoingEdges() == 0)
    return false;

  for (auto edge : scc->getEdges()) {
    if (edge->isMemoryDependence())
      return false;
  }

  if (sccToLoopCarriedDependencies.find(scc)
      == sccToLoopCarriedDependencies.end())
    return true;

  auto topLoop = loopNode->getLoop();
  for (auto loopCarriedDependency : sccToLoopCarriedDependencies.at(scc)) {
    auto valueFrom = loopCarriedDependency->getOutgoingT();
    auto valueTo = loopCarriedDependency->getIncomingT();
    assert(isa<Instruction>(valueFrom) && isa<Instruction>(valueTo));
    if (loopNode->getInnermostLoopThatContains(cast<Instruction>(valueFrom))
            == topLoop
        || loopNode->getInnermostLoopThatContains(cast<Instruction>(valueTo))
               == topLoop) {
      return false;
    }
  }

  return true;
}

bool SCCDAGAttrs::isClonableByInductionVars(SCC *scc) const {

  /*
   * FIXME: This check should not exist; instead, SCC where cloning
   * is trivial should be separated out by the parallelization scheme
   */
  if (this->sccdag->fetchNode(scc)->numOutgoingEdges() == 0) {
    return false;
  }

  /*
   * Fetch the SCC metadata.
   */
  auto sccInfo = this->getSCCAttrs(scc);

  return isa<InductionVariableSCC>(sccInfo);
}

bool SCCDAGAttrs::isClonableBySyntacticSugarInstrs(SCC *scc) const {

  /*
   * FIXME: This check should not exist; instead, SCC where cloning
   * is trivial should be separated out by the parallelization scheme
   */
  if (this->sccdag->fetchNode(scc)->numOutgoingEdges() == 0)
    return false;

  if (scc->numInternalNodes() > 1)
    return false;
  auto I = scc->begin_internal_node_map()->first;
  if (isa<PHINode>(I) || isa<GetElementPtrInst>(I) || isa<CastInst>(I)) {
    return true;
  }
  return false;
}

bool SCCDAGAttrs::isClonableByCmpBrInstrs(SCC *scc) const {
  for (auto iNodePair : scc->internalNodePairs()) {
    auto V = iNodePair.first;
    if (auto inst = dyn_cast<Instruction>(V)) {
      if (isa<CmpInst>(inst) || inst->isTerminator()) {
        continue;
      }
    }
    return false;
  }
  return true;
}

SCCDAG *SCCDAGAttrs::getSCCDAG(void) const {
  return this->sccdag;
}

void SCCDAGAttrs::dumpToFile(int id) {
  std::error_code EC;
  std::string filename = "sccdag-attrs-loop-" + std::to_string(id) + ".dot";

  if (EC) {
    errs() << "ERROR: Could not dump debug logs to file!";
    return;
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
    if (isa<LoopIterationSCC>(sccInfo))
      ros << "Independent ";
    if (sccInfo->canBeCloned())
      ros << "Clonable ";
    if (isa<ReductionSCC>(sccInfo))
      ros << "Reducible ";
    if (isa<InductionVariableSCC>(sccInfo))
      ros << "IV ";
    ros << "\n";

    for (auto iNodePair : sccNode->getT()->internalNodePairs()) {
      iNodePair.first->print(ros);
      ros << "\n";
    }

    ros.flush();
    sccToDescriptionMap.insert(
        std::make_pair(sccNode, addNode(sccDescription, true)));
  }

  for (auto sccEdge : sccdag->getEdges()) {
    auto outgoingDesc =
        sccToDescriptionMap.at(sccEdge->getOutgoingNode())->getT();
    auto incomingDesc =
        sccToDescriptionMap.at(sccEdge->getIncomingNode())->getT();
    stageGraph.addEdge(outgoingDesc, incomingDesc);
  }

  DGPrinter::writeGraph<DG<DGString>, DGString>(filename, &stageGraph);
  for (auto elem : elements) {
    delete elem;
  }

  return;
}

std::unordered_set<SCCAttrs *> SCCDAGAttrs::getSCCsOfKind(SCCAttrs::SCCKind K) {
  std::unordered_set<SCCAttrs *> SCCs{};
  for (auto pair : this->sccToInfo) {
    auto sccAttrs = pair.second;
    auto sccKind = sccAttrs->getKind();
    if (sccKind == K) {
      SCCs.insert(sccAttrs);
    }
  }

  return SCCs;
}

SCCDAGAttrs::~SCCDAGAttrs() {
  return;
}

} // namespace llvm::noelle
