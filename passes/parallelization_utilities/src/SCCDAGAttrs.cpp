/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SCCDAGAttrs.hpp"

using namespace llvm;

AccumulatorOpInfo::AccumulatorOpInfo () {
  this->sideEffectFreeOps = {
    Instruction::Add,
    Instruction::FAdd,
    Instruction::Mul,
    Instruction::FMul,
    Instruction::Sub,
    Instruction::FSub
  };
  this->accumOps = std::set<unsigned>(sideEffectFreeOps.begin(), sideEffectFreeOps.end());
  this->opIdentities = {
    { Instruction::Add, 0 },
    { Instruction::FAdd, 0 },
    { Instruction::Mul, 1 },
    { Instruction::FMul, 1 },
    { Instruction::Sub, 0 },
    { Instruction::FSub, 0 }
  };
}

bool AccumulatorOpInfo::isSubOp (unsigned op) {
  return Instruction::Sub == op || Instruction::FSub == op;
}

bool AccumulatorOpInfo::isMulOp (unsigned op) {
  return Instruction::Mul == op || Instruction::FMul == op;
}

bool AccumulatorOpInfo::isAddOp (unsigned op) {
  return Instruction::Add == op || Instruction::FAdd == op;
}

unsigned AccumulatorOpInfo::accumOpForType (unsigned op, Type *type) {
  if (type->isIntegerTy()) {
    return isMulOp(op) ? Instruction::Mul : Instruction::Add;
  } else {
    return isMulOp(op) ? Instruction::FMul : Instruction::FAdd;
  }
}

Value *AccumulatorOpInfo::generateIdentityFor (Instruction *accumulator, Type *castType) {
  Value *initVal = nullptr;
  auto opIdentity = this->opIdentities[accumulator->getOpcode()];
  if (castType->isIntegerTy()) initVal = ConstantInt::get(castType, opIdentity);
  if (castType->isFloatTy()) initVal = ConstantFP::get(castType, (float)opIdentity);
  if (castType->isDoubleTy()) initVal = ConstantFP::get(castType, (double)opIdentity);
  assert(initVal != nullptr);
  return initVal;
}

// FIXME: Do not use
void SCCAttrs::collectSCCValues () {
  struct PathValue {
    PathValue *prev;
    Value *value;
    PathValue (Value *V, PathValue *PV = nullptr) : value{V}, prev{PV} {};
  };

  /*
   * Bookkeeping for later deletion and to avoid duplicate work
   */
  std::set<PathValue *> pathValues;
  std::set<Value *> valuesSeen;

  std::deque<PathValue *> toTraverse;
  auto topLevelNodes = scc->getTopLevelNodes(true);
  for (auto node : topLevelNodes) {
    node->getT()->print(errs() << "TOP LEVEL V: "); errs() << "\n";
    auto pathV = new PathValue(node->getT());
    pathValues.insert(pathV);
    toTraverse.push_front(pathV);
  }

  scc->print(errs(), "COLLECT: ", 0) << "\n";
  while (!toTraverse.empty()) {
    auto pathV = toTraverse.front();
    toTraverse.pop_front();
    // pathV->value->print(errs() << "Traversing V: "); errs() << "\n";

    bool isCycle = false;
    auto prevV = pathV->prev;
    while (prevV) {
      // prevV->value->print(errs() << "\t Prev V: "); errs() << "\n";
      if (pathV->value == prevV->value) {
        isCycle = true;
        break;
      }
      prevV = prevV->prev;
    }

    if (isCycle) {
      auto cycleV = pathV;
      while (cycleV != prevV) {
        stronglyConnectedDataValues.insert(cycleV->value);
        cycleV = cycleV->prev;
      }
      continue;
    }

    auto node = scc->fetchNode(pathV->value);
    for (auto edge : node->getOutgoingEdges()) {

      // Only trace paths across data dependencies, starting
      //  anew on newly encountered data values across control dependencies
      auto nextV = edge->getIncomingT();
      PathValue *nextPathV = nullptr;
      if (edge->isControlDependence()) {
        // nextV->print(errs() << "Control dependence traveling to: "); errs() << "\n";
        nextPathV = new PathValue(nextV);
      } else {
        // nextV->print(errs() << "Data dependence traveling to: "); errs() << "\n";
        nextPathV = new PathValue(nextV, pathV);
      }

      if (nextPathV) {
        pathValues.insert(nextPathV);
        toTraverse.push_front(nextPathV);
      }
    }
  }

  for (auto pathV : pathValues) delete pathV;

  for (auto dataV : stronglyConnectedDataValues) {
    dataV->print(errs() << "COLLECT: V: "); errs() << "\n";
  }
}

SCCAttrs::SCCAttrs (SCC *s)
  : scc{s}, isClonable{0}, hasIV{0},
    PHINodes{}, accumulators{}, controlFlowInsts{}, controlPairs{},
    singlePHI{nullptr}, singleAccumulator{nullptr}, singleControlPair{nullptr,nullptr} {

  // Collect basic blocks contained within SCC
  for (auto nodePair : this->scc->internalNodePairs()) {
    this->bbs.insert(cast<Instruction>(nodePair.first)->getParent());
  }

  // Collect values actually contained in the strongly connected components,
  // ignoring ancillary values merged into the SCC object
  // collectSCCValues();
}

void SCCDAGAttrs::populate (SCCDAG *loopSCCDAG, LoopInfoSummary &LIS, ScalarEvolution &SE) {
  this->sccdag = loopSCCDAG;
  collectDependencies(LIS);

  for (auto node : loopSCCDAG->getNodes()) {
    auto scc = node->getT();
    this->sccToInfo[scc] = std::move(std::make_unique<SCCAttrs>(scc));

    this->collectPHIsAndAccumulators(scc);
    this->collectControlFlowInstructions(scc);

    this->checkIfInductionVariableSCC(scc, SE, LIS);
    if (isInductionVariableSCC(scc)) this->checkIfIVHasFixedBounds(scc, LIS);
    this->checkIfClonable(scc, SE);

    if (this->checkIfIndependent(scc)) {
      scc->setType(SCC::SCCType::INDEPENDENT);
    } else if (this->checkIfReducible(scc, LIS)) {
      scc->setType(SCC::SCCType::REDUCIBLE);
    } else {
      scc->setType(SCC::SCCType::SEQUENTIAL);
    }
  }

  collectSCCGraphAssumingDistributedClones();
}

std::set<SCC *> SCCDAGAttrs::getSCCsWithLoopCarriedDataDependencies (void) const {
  std::set<SCC *> sccs;
  for (auto &sccDependencies : this->interIterDeps) {
    sccs.insert(sccDependencies.first);
  }
  return sccs;
}

/*
 * Assumption(angelo): An induction variable will be the root SCC of the loop
 */
bool SCCDAGAttrs::isLoopGovernedByIV () const {
  auto topLevelNodes = sccdag->getTopLevelNodes();

  /*
   * Step 1: Isolate top level SCCs (excluding independent instructions in SCCDAG)
   */
  std::queue<DGNode<SCC> *> toTraverse;
  for (auto node : topLevelNodes) toTraverse.push(node);
  std::set<SCC *> topLevelSCCs;
  while (!toTraverse.empty()) {
    auto node = toTraverse.front();
    auto scc = node->getT();
    toTraverse.pop();
    if (canExecuteIndependently(scc)) {
      auto nextDepth = sccdag->getNextDepthNodes(node);
      for (auto next : nextDepth) toTraverse.push(next);
      continue;
    }
    topLevelSCCs.insert(scc);
  }

  /*
   * Step 2: Ensure there is only 1, and that it is an induction variable
   */
  // errs() << "SCCDAGAttrs: NUM TOP LEVEL SCCS: " << topLevelSCCs.size() << "\n";
  if (topLevelSCCs.size() != 1) return false;
  // (*topLevelSCCs.begin())->print(errs() << "That single SCC:\n") << "\n";
  return isInductionVariableSCC(*topLevelSCCs.begin());
}

bool SCCDAGAttrs::areAllLiveOutValuesReducable (LoopEnvironment *env) const {
  for (auto envIndex : env->getEnvIndicesOfLiveOutVars()) {
    auto producer = env->producerAt(envIndex);
    auto scc = sccdag->sccOfValue(producer);

    // TODO(angelo): Implement this if it is legal. Unsure at the moment
    // if (scc->getType() == SCC::SCCType::INDEPENDENT) continue ;
    if (scc->getType() == SCC::SCCType::REDUCIBLE) continue ;

    return false;
  }
  return true;
}

bool SCCDAGAttrs::canExecuteReducibly (SCC *scc) const {
  return scc->getType() == SCC::SCCType::REDUCIBLE;
}

bool SCCDAGAttrs::canExecuteIndependently (SCC *scc) const {
  return scc->getType() == SCC::SCCType::INDEPENDENT;
}

bool SCCDAGAttrs::canBeCloned (SCC *scc) const {
  return sccToInfo.find(scc)->second->isClonable;
}

bool SCCDAGAttrs::isInductionVariableSCC (SCC *scc) const {
  return sccToInfo.find(scc)->second->hasIV;
}

bool SCCDAGAttrs::isSCCContainedInSubloop (LoopInfoSummary &LIS, SCC *scc) const {
  bool instInSubloops = true;
  for (auto iNodePair : scc->internalNodePairs()) {
    auto bb = cast<Instruction>(iNodePair.first)->getParent();
    instInSubloops &= LIS.bbToLoop[bb] != LIS.topLoop;
  }
  return instInSubloops;
}

std::set<BasicBlock *> & SCCDAGAttrs::getBasicBlocks (SCC *scc) {
  auto &sccInfo = this->sccToInfo[scc];
  return sccInfo->bbs;
}

// REFACTOR(angelo): find better workaround than just a getter for SCCAttrs
std::unique_ptr<SCCAttrs> & SCCDAGAttrs::getSCCAttrs (SCC *scc) {
  return this->sccToInfo[scc];
}

void SCCDAGAttrs::collectSCCGraphAssumingDistributedClones () {
  auto addIncomingNodes = [&](std::queue<DGNode<SCC> *> &queue, DGNode<SCC> *node) -> void {
    std::set<DGNode<SCC> *> nodes;
    auto scc = node->getT();
    for (auto edge : node->getIncomingEdges()) {
      nodes.insert(edge->getOutgoingNode());
      this->edgesViaClones[scc].insert(edge);
    }
    for (auto node : nodes) queue.push(node);
  };

  for (auto sccPair : sccdag->internalNodePairs()) {
    auto childSCC = sccPair.first;
    std::queue<DGNode<SCC> *> nodesToCheck;
    addIncomingNodes(nodesToCheck, sccPair.second);

    while (!nodesToCheck.empty()) {
      auto node = nodesToCheck.front();
      nodesToCheck.pop();
      auto scc = node->getT();
      this->parentsViaClones[childSCC].insert(scc);
      if (this->canBeCloned(scc)) addIncomingNodes(nodesToCheck, node);
    }
  }
}

void SCCDAGAttrs::collectDependencies (LoopInfoSummary &LIS) {

  /*
   * Collect values producing intra iteration data dependencies
   */
  for (auto edge : sccdag->getEdges()) {
    auto sccTo = edge->getIncomingT();
    for (auto subEdge : edge->getSubEdges()) {
      intraIterDeps[subEdge->getOutgoingT()].insert(sccTo);
    }
  }

  /*
   * Collect back edges within each SCC
   * Data dependency back edges are from non-control
   * instructions to PHIs in loop headers
   * Control dependency back edges are from conditional
   * branches to instructions in loop headers
   */
  for (auto sccNode : sccdag->getNodes()) {
    auto scc = sccNode->getT();
    for (auto valuePair : scc->internalNodePairs()) {
      if (auto phi = dyn_cast<PHINode>(valuePair.first)) {
        auto loop = LIS.bbToLoop[phi->getParent()];
        if (loop->header != phi->getParent()) continue;

        for (auto edge : valuePair.second->getIncomingEdges()) {
          if (edge->isControlDependence()) continue;
          auto depI = (Instruction*)edge->getOutgoingT();
          if (!scc->isInternal(depI)) continue;
          if (canPrecedeInCurrentIteration(LIS, depI, phi)) continue;
          interIterDeps[scc].insert(edge);
        }
      }

      if (auto term = dyn_cast<TerminatorInst>(valuePair.first)) {
        for (auto edge : valuePair.second->getOutgoingEdges()) {
          auto depV = edge->getIncomingT();
          assert(isa<Instruction>(depV));
          auto depBB = ((Instruction*)depV)->getParent();
          if (term->getParent() != depBB) continue;
          interIterDeps[scc].insert(edge);
        }
      }

      if (isa<StoreInst>(valuePair.first)
          || isa<LoadInst>(valuePair.first)
          || isa<CallInst>(valuePair.first)) {
        auto memI = (Instruction *)valuePair.first;
        for (auto edge : valuePair.second->getOutgoingEdges()) {
          if (!edge->isMemoryDependence()) continue;
          auto depI = (Instruction *)edge->getIncomingT();
          if (canPrecedeInCurrentIteration(LIS, memI, depI)) continue;
          interIterDeps[scc].insert(edge);
        }
      }
    }

    // bool noCycle = !scc->hasCycle(/*ignoreControlDep=*/false);
    /*
    bool noInterIterDeps = interIterDeps.find(scc) == interIterDeps.end();
    if (noCycle != noInterIterDeps) {
      errs() << "ERROR: Improper collection of inter iteration dependencies in SCC! "
        << (noCycle ? "no cycle" : "cycle") << " but "
        << (noInterIterDeps ? "no inter iter dependencies" : "inter iter dependencies") << "\n";
      scc->print(errs()) << "\n";
      assert(false && "SCCDAGAttrs::collectDependencies");
    }
    */
  }
}

// TODO: Consolidate this logic and its equivalent in PDGAnalysis
bool SCCDAGAttrs::canPrecedeInCurrentIteration (LoopInfoSummary &LIS, Instruction *from, Instruction *to) const {
  BasicBlock *fromBB = from->getParent();
  BasicBlock *toBB = to->getParent();
  auto loopIter = LIS.bbToLoop.find(fromBB);
  BasicBlock *headerBB = nullptr;
  if (loopIter != LIS.bbToLoop.end()) headerBB = loopIter->second->header;

  if (fromBB == toBB) {
    for (auto &I : *fromBB) {
      if (&I == from) return true;
      if (&I == to) return false;
    }
  }

  std::queue<BasicBlock *> bbToTraverse;
  std::set<BasicBlock *> bbReached;
  auto traverseOn = [&](BasicBlock *bb) -> void {
    bbToTraverse.push(bb); bbReached.insert(bb);
  };
  traverseOn(toBB);

  while (!bbToTraverse.empty()) {
    auto bb = bbToTraverse.front();
    bbToTraverse.pop();
    if (bb == fromBB) return true;
    if (bb == headerBB) continue;

    for (auto predBB : make_range(pred_begin(bb), pred_end(bb))) {
      if (bbReached.find(predBB) == bbReached.end()) {
        traverseOn(predBB);
      }
    }
  }

  return false;
}

void SCCDAGAttrs::collectPHIsAndAccumulators (SCC *scc) {
  auto &sccInfo = this->getSCCAttrs(scc);
  for (auto iNodePair : scc->internalNodePairs()) {
    auto V = iNodePair.first;

    if (auto phi = dyn_cast<PHINode>(V)) {
      sccInfo->PHINodes.insert(phi);
      continue;
    }
    if (auto I = dyn_cast<Instruction>(V)) {
      auto binOp = I->getOpcode();
      if (accumOpInfo.accumOps.find(binOp) != accumOpInfo.accumOps.end()) {
        sccInfo->accumulators.insert(I);
        continue;
      }
    }
  }

  if (sccInfo->PHINodes.size() == 1) {
    sccInfo->singlePHI = *sccInfo->PHINodes.begin();
  }
  if (sccInfo->accumulators.size() == 1) {
    sccInfo->singleAccumulator = *sccInfo->accumulators.begin();
  }
}

void SCCDAGAttrs::collectControlFlowInstructions (SCC *scc) {
  auto &sccInfo = this->getSCCAttrs(scc);
  for (auto iNodePair : scc->internalNodePairs()) {
    if (iNodePair.second->numOutgoingEdges() == 0) continue;
    if (auto term = dyn_cast<TerminatorInst>(iNodePair.first)) {
      sccInfo->controlFlowInsts.insert(term);
    }
  }

  for (auto term : sccInfo->controlFlowInsts) {
    if (auto br = dyn_cast<BranchInst>(term)) {
      assert(br->isConditional()
        && "BranchInst with outgoing edges in an SCC must be conditional!");
      sccInfo->controlPairs.insert(std::make_pair(br->getCondition(), br));
    }
    if (auto switchI = dyn_cast<SwitchInst>(term)) {
      sccInfo->controlPairs.insert(std::make_pair(switchI->getCondition(), switchI));
    }
  }

  if (sccInfo->controlPairs.size() == 1) {
    sccInfo->singleControlPair = *sccInfo->controlPairs.begin();
  }
}

bool SCCDAGAttrs::checkIfReducible (SCC *scc, LoopInfoSummary &LIS) {
  auto &sccInfo = this->getSCCAttrs(scc);

  /*
   * Requirement: There are no data/memory dependent SCCs of this SCC
   */
  for (auto iNodePair : scc->externalNodePairs()) {
    if (iNodePair.second->numConnectedEdges() == 0) continue ;
    for (auto edge : iNodePair.second->getAllConnectedEdges()) {
      if (edge->isMemoryDependence()) return false;
    }

    if (iNodePair.second->numIncomingEdges() == 0) continue ;
    for (auto edge : iNodePair.second->getIncomingEdges()) {
      if (!edge->isControlDependence()) return false;
    }
  }

  /*
   * Requirement: Data flow: a single data backedge per loop level
   * contained in this SCC
   */
  std::set<LoopSummary *> backedgeLoops;
  for (auto edge : interIterDeps[scc]) {
    if (edge->isControlDependence()) return false;
    if (edge->isMemoryDependence()) return false;

    auto outI = isa<Instruction>(edge->getOutgoingT())
      ? cast<Instruction>(edge->getOutgoingT()) : nullptr;
    auto inI = isa<Instruction>(edge->getIncomingT())
      ? cast<Instruction>(edge->getIncomingT()) : nullptr;
    if (!outI || !inI) return false;

    auto outgoingBBLoop = LIS.bbToLoop.find(outI->getParent());
    auto incomingBBLoop = LIS.bbToLoop.find(inI->getParent());
    if (outgoingBBLoop == LIS.bbToLoop.end() ||
        outgoingBBLoop == incomingBBLoop) return false;

    auto loop = outgoingBBLoop->second;
    if (backedgeLoops.find(loop) != backedgeLoops.end()) return false;
    backedgeLoops.insert(loop);
  }

  /*
   * Requirement: Control flow is intra-iteration; conditions are
   * determined externally to the SCC
   */
  for (auto pair : sccInfo->controlPairs) {
    if (scc->isInternal(pair.first)) return false;
  }

  /*
   * Requirement: all PHI incoming values from within a loop iteration
   * are from other internal PHIs (no PHI = constant, etc... business)
   * so that accumulation is truly expressed solely by accumulators
   */
  for (auto phi : sccInfo->PHINodes) {
    auto loopOfPHI = LIS.bbToLoop[phi->getParent()];
    for (auto i = 0; i < phi->getNumIncomingValues(); ++i) {
      auto incomingBB = phi->getIncomingBlock(i);
      auto loopOfIncoming = LIS.bbToLoop.find(incomingBB);
      if (loopOfIncoming == LIS.bbToLoop.end() || loopOfIncoming->second != loopOfPHI) continue;
      if (!isDerivedPHIOrAccumulator(phi->getIncomingValue(i), scc)) return false;
    }
  }

  /*
   * Requirement: 1+ accumulators that are all side effect free
   * Requirement: all accumulators act on one PHI/accumulator in the SCC
   *  and one constant or external value
   */
  if (sccInfo->accumulators.size() == 0) return false;
  auto phis = sccInfo->PHINodes;
  auto accums = sccInfo->accumulators;
  for (auto accum : accums) {
    unsigned opCode = accum->getOpcode();
    if (accumOpInfo.sideEffectFreeOps.find(opCode) == accumOpInfo.sideEffectFreeOps.end()) {
      return false;
    }

    auto opL = accum->getOperand(0);
    auto opR = accum->getOperand(1);
    if (!(isDerivedWithinSCC(opL, scc) ^ isDerivedWithinSCC(opR, scc))) return false;
    if (!(isDerivedPHIOrAccumulator(opL, scc) ^ isDerivedPHIOrAccumulator(opR, scc))) return false;
  }

  /*
   * Requirement: instructions are all Add/Sub or all Mul
   * Requirement: second operand of subtraction must be external
   */
  bool isFirstMul = accumOpInfo.isMulOp((*accums.begin())->getOpcode());
  for (auto accum : accums) {
    bool isMul = accumOpInfo.isMulOp(accum->getOpcode());
    if (isMul ^ isFirstMul) return false;
    if (accumOpInfo.isSubOp(accum->getOpcode())) {
      if (scc->isInternal(accum->getOperand(1))) return false;
    }
  }

  return true;
}

/*
 * The SCC is independent if it doesn't have loop carried data dependencies
 */
bool SCCDAGAttrs::checkIfIndependent (SCC *scc) {
  return interIterDeps.find(scc) == interIterDeps.end();
}

bool SCCDAGAttrs::checkIfInductionVariableSCC (SCC *scc, ScalarEvolution &SE, LoopInfoSummary &LIS) {
  auto &sccInfo = this->getSCCAttrs(scc);
  auto setHasIV = [&](bool hasIV) -> bool {
    // scc->printMinimal(errs() << "Not IV:\n") << "\n";
    return sccInfo->hasIV = hasIV;
  };

  /*
   * Identify single conditional branch that dictates control flow in the SCC
   */
  TerminatorInst *term = sccInfo->singleControlPair.second;
  if (!term || !isa<BranchInst>(term)) return setHasIV(false);
  Value *condition = sccInfo->singleControlPair.first;
  if (!condition || !isa<CmpInst>(condition)) return setHasIV(false);
  CmpInst *cmp = (CmpInst*)condition;

  /*
   * Identify, on the CmpInst, a PHINode or accumulator, and some value not
   * derived within the SCC
   */
  auto opL = cmp->getOperand(0), opR = cmp->getOperand(1);
  if (!(isDerivedWithinSCC(opL, scc) ^ isDerivedWithinSCC(opR, scc))) return setHasIV(false);
  if (!(isDerivedPHIOrAccumulator(opL, scc) ^ isDerivedPHIOrAccumulator(opR, scc))) return setHasIV(false);

  /*
   * Ensure a single PHI with induction accumulation only
   */
  if (!sccInfo->singlePHI) return setHasIV(false);
  auto loopOfPHI = LIS.bbToLoop[sccInfo->singlePHI->getParent()];
  for (auto i = 0; i < sccInfo->singlePHI->getNumIncomingValues(); ++i) {
    auto incomingBB = sccInfo->singlePHI->getIncomingBlock(i);
    auto loopOfIncoming = LIS.bbToLoop.find(incomingBB);
    if (loopOfIncoming == LIS.bbToLoop.end() || loopOfIncoming->second != loopOfPHI) continue;
    if (!isDerivedPHIOrAccumulator(sccInfo->singlePHI->getIncomingValue(i), scc)) return setHasIV(false);
  }

  for (auto I : sccInfo->accumulators) {
    auto scev = SE.getSCEV(I);
    if (scev->getSCEVType() != scAddRecExpr) {
      return setHasIV(false);
    }
  }

  // scc->printMinimal(errs() << "IS an IV:\n") << "\n";
  return setHasIV(true);
}

void SCCDAGAttrs::checkIfIVHasFixedBounds (SCC *scc, LoopInfoSummary &LIS) {
  FixedIVBounds *fixedIVBounds = new FixedIVBounds();
  FixedIVBounds &IVBounds = *fixedIVBounds;
  auto notSimple = [&]() -> void {
    delete fixedIVBounds;
    return;
  };

  /*
   * IV is described by single PHI with a start and recurrence incoming value
   * The IV has one accumulator only
   */
  auto &sccInfo = this->getSCCAttrs(scc);
  if (!sccInfo->singlePHI || !sccInfo->singleAccumulator) return notSimple();
  if (sccInfo->singlePHI->getNumIncomingValues() != 2) return notSimple();
  if (sccInfo->singleControlPair.first == nullptr) return notSimple();

  auto accum = sccInfo->singleAccumulator;
  auto incomingStart = sccInfo->singlePHI->getIncomingValue(0);
  if (incomingStart == accum) incomingStart = sccInfo->singlePHI->getIncomingValue(1);
  IVBounds.start = incomingStart;

  /*
   * The IV recurrence is integer, by +-1 
   */
  auto stepValue = accum->getOperand(0);
  if (stepValue == sccInfo->singlePHI) stepValue = accum->getOperand(1);
  if (!isa<ConstantInt>(stepValue)) return notSimple();
  IVBounds.step = (ConstantInt*)stepValue;
  auto stepSize = IVBounds.step->getValue();
  if (stepSize != 1 && stepSize != -1) return notSimple();

  auto cmp = (CmpInst*)sccInfo->singleControlPair.first;
  auto cmpLHS = cmp->getOperand(0);
  unsigned cmpToInd = cmpLHS == sccInfo->singlePHI || cmpLHS == accum;
  IVBounds.cmpIVTo = cmp->getOperand(cmpToInd);
  IVBounds.isCmpOnAccum = cmp->getOperand((cmpToInd + 1) % 2) == accum;
  IVBounds.isCmpIVLHS = cmpToInd;

  /*
   * The CmpInst compare value is constant, or a chain (length 0 or more)
   * of independent nodes in the SCC that ends in a loop external value
   */
  if (!isa<ConstantData>(IVBounds.cmpIVTo)) {
    if (!isa<Instruction>(IVBounds.cmpIVTo)) return notSimple();
    IVBounds.cmpToDerivation.push_back((Instruction*)IVBounds.cmpIVTo);
    if (!collectDerivationChain(IVBounds.cmpToDerivation, scc)) return notSimple();

    auto chainEnd = IVBounds.cmpToDerivation.back();
    if (isDerivedWithinSCC(chainEnd, scc)) return notSimple();
  }

  /*
   * The last value before the end value reached by the IV can be determined
   */
  if (!isIVUpperBoundSimple(scc, IVBounds, LIS)) return notSimple();

  sccIVBounds[scc] = fixedIVBounds;
}

bool SCCDAGAttrs::isIVUpperBoundSimple (SCC *scc, FixedIVBounds &IVBounds, LoopInfoSummary &LIS) {
  auto &sccInfo = this->getSCCAttrs(scc);
  auto cmp = cast<CmpInst>(sccInfo->singleControlPair.first);
  auto br = cast<BranchInst>(sccInfo->singleControlPair.second);

  /*
   * Branch statement has two successors, one in the loop body, one outside the loop
   */
  auto loop = LIS.bbToLoop[br->getParent()];
  auto brLHSInLoop = loop->bbs.find(br->getSuccessor(0)) != loop->bbs.end();
  auto brRHSInLoop = loop->bbs.find(br->getSuccessor(1)) != loop->bbs.end();
  if (!(brLHSInLoop ^ brRHSInLoop)) return false;

  bool exitOnCmp = !brLHSInLoop;
  auto signedPred = cmp->isUnsigned() ? cmp->getSignedPredicate() : cmp->getPredicate();
  signedPred = IVBounds.isCmpIVLHS ? signedPred : ICmpInst::getSwappedPredicate(signedPred);
  int stepSize = IVBounds.step->getValue().getSExtValue();

  auto cmpPredAbort = [&]() -> void {
    errs() << "SCCDAGAttrs:   Error: comparison and branch of top level IV misunderstood\n";
    abort();
  };

  if (!exitOnCmp) { 
    if (stepSize == 1) {
      switch (signedPred) {
        case CmpInst::Predicate::ICMP_SLE:
          IVBounds.endOffset = 1;
        case CmpInst::Predicate::ICMP_NE:
        case CmpInst::Predicate::ICMP_SLT:
          break;
        default:
          cmpPredAbort();
      }
    } else {
      switch (signedPred) {
        case CmpInst::Predicate::ICMP_SGE:
          IVBounds.endOffset = -1;
        case CmpInst::Predicate::ICMP_NE:
        case CmpInst::Predicate::ICMP_SGT:
          break;
        default:
          cmpPredAbort();
      }
    }
  } else {
    if (stepSize == 1) {
      switch (signedPred) {
        case CmpInst::Predicate::ICMP_SGT:
          IVBounds.endOffset = 1;
        case CmpInst::Predicate::ICMP_SGE:
        case CmpInst::Predicate::ICMP_EQ:
          break;
        default:
          cmpPredAbort();
      }
    } else {
      switch (signedPred) {
        case CmpInst::Predicate::ICMP_SLT:
          IVBounds.endOffset = -1;
        case CmpInst::Predicate::ICMP_SLE:
        case CmpInst::Predicate::ICMP_EQ:
          break;
        default:
          cmpPredAbort();
      }
    }
  }

  IVBounds.endOffset -= stepSize * IVBounds.isCmpOnAccum;
  return true;
}

void SCCDAGAttrs::checkIfClonable (SCC *scc, ScalarEvolution &SE) {
  if (isClonableByInductionVars(scc) ||
      isClonableBySyntacticSugarInstrs(scc) ||
      isClonableByCmpBrInstrs(scc)) {
    this->getSCCAttrs(scc)->isClonable = true;
    clonableSCCs.insert(scc);
  }
}

bool SCCDAGAttrs::isClonableByInductionVars (SCC *scc) const {

  /*
   * FIXME: This check should not exist; instead, SCC where cloning
   * is trivial should be separated out by the parallelization scheme
   */
  if (sccdag->fetchNode(scc)->numOutgoingEdges() == 0) return false;
  return this->isInductionVariableSCC(scc);
}

bool SCCDAGAttrs::isClonableBySyntacticSugarInstrs (SCC *scc) const {

  /*
   * FIXME: This check should not exist; instead, SCC where cloning
   * is trivial should be separated out by the parallelization scheme
   */
  if (sccdag->fetchNode(scc)->numOutgoingEdges() == 0) return false;

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
    if (isa<CmpInst>(V) || isa<TerminatorInst>(V)) continue;
    return false;
  }
  return true;
}

/*
 * NOTE: Derivation within an SCC requires inclusion in the SCC object
 * and dependency to a value in the strongly connected component, in the cycle
 * TODO: Derivation should only consider data dependency cycles, not control
 */
bool SCCDAGAttrs::isDerivedWithinSCC (Value *val, SCC *scc) const {
  if (!scc->isInternal(val)) return false;

  auto &sccInfo = sccToInfo.find(scc)->second;
  auto isStrongly = sccInfo->stronglyConnectedDataValues.find(val)
    != sccInfo->stronglyConnectedDataValues.end();
  auto isWeakly = sccInfo->weaklyConnectedDataValues.find(val)
    != sccInfo->weaklyConnectedDataValues.end();
  if (isStrongly) return true;
  if (isWeakly) return false;

  // Traversing both outgoing OR incoming edges leads back to the node
  // if it is in the SCC; otherwise, it is just a merged in node
  auto startNode = scc->fetchNode(val);
  std::queue<DGNode<Value> *> toOutgoing;
  std::set<DGNode<Value> *> seen;
  toOutgoing.push(startNode);
  bool inCycle = false;
  while (!toOutgoing.empty()) {
    auto node = toOutgoing.front();
    toOutgoing.pop();

    for (auto edge : node->getOutgoingEdges()) {
      if (edge->isControlDependence()) continue;
      auto inNode = edge->getIncomingNode();
      if (scc->isExternal(inNode->getT())) continue;
      if (inNode == startNode) inCycle = true;
      if (seen.find(inNode) == seen.end()) {
        // inNode->getT()->print(errs() << "GOING TO: "); errs() << "\n";
        seen.insert(inNode);
        toOutgoing.push(inNode);
      }
    }
    if (inCycle) break;
  }
  // errs() << "IN CYCLE with outgoings: " << inCycle << "\n";

  if (!inCycle) {
    sccInfo->weaklyConnectedDataValues.insert(val);
    // val->print(errs() << "WEAKLY CONNECTED: "); errs() << "\n";
    return false;
  }

  inCycle = false;
  seen.clear();
  std::queue<DGNode<Value> *> toIncoming;
  toIncoming.push(startNode);
  while (!toIncoming.empty()) {
    auto node = toIncoming.front();
    toIncoming.pop();

    for (auto edge : node->getIncomingEdges()) {
      if (edge->isControlDependence()) continue;
      auto outNode = edge->getOutgoingNode();
      if (scc->isExternal(outNode->getT())) continue;
      if (outNode == startNode) inCycle = true;
      if (seen.find(outNode) == seen.end()) {
        // outNode->getT()->print(errs() << "GOING TO: "); errs() << "\n";
        seen.insert(outNode);
        toIncoming.push(outNode);
      }
    }
    if (inCycle) break;
  }
  // errs() << "IN CYCLE with incomings: " << inCycle << "\n";

  if (!inCycle) {
    sccInfo->weaklyConnectedDataValues.insert(val);
    // val->print(errs() << "WEAKLY CONNECTED: "); errs() << "\n";
    return false;
  }
  sccInfo->stronglyConnectedDataValues.insert(val);
  return true;
}

bool SCCDAGAttrs::isDerivedPHIOrAccumulator (Value *val, SCC *scc) const {
  Value *derived = val;
  if (auto cast = dyn_cast<CastInst>(val)) {
    derived = cast->getOperand(0);
  }

  auto &sccInfo = sccToInfo.find(scc)->second;
  bool isInternalPHI = isa<PHINode>(derived)
    && sccInfo->PHINodes.find(cast<PHINode>(derived)) != sccInfo->PHINodes.end();
  bool isInternalAccum = isa<Instruction>(derived)
    && sccInfo->accumulators.find(cast<Instruction>(derived)) != sccInfo->accumulators.end();
  return isDerivedWithinSCC(derived, scc) && (isInternalPHI || isInternalAccum);
}

bool SCCDAGAttrs::collectDerivationChain (std::vector<Instruction *> &chain, SCC *scc) {
  Instruction *deriving = chain[0];
  if (!scc->isInternal(deriving)) return true;

  std::set<Instruction *> valuesSeen;
  chain.pop_back();
  while (scc->isInternal(deriving)) {
    chain.push_back(deriving);
    if (valuesSeen.find(deriving) != valuesSeen.end()) return false;
    valuesSeen.insert(deriving);

    auto node = scc->fetchNode(deriving);
    std::set<Value *> incomingDataDeps;
    for (auto edge : node->getIncomingEdges()) {
      if (edge->isControlDependence()) continue;
      incomingDataDeps.insert(edge->getOutgoingT());
    }
    incomingDataDeps.erase(deriving);

    /*
     * Continue down the dependency graph only if it is a linear chain
     */
    if (incomingDataDeps.size() == 0) break;
    if (incomingDataDeps.size() != 1) return false;
    auto V = *incomingDataDeps.begin();
    if (!isa<Instruction>(V)) return false;
    deriving = (Instruction*)V;
  }

  return true;
}
