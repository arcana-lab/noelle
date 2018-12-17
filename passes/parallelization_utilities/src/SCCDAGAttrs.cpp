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

void SCCDAGAttrs::populate (SCCDAG *loopSCCDAG, LoopInfoSummary &LIS, ScalarEvolution &SE) {
  this->sccdag = loopSCCDAG;
  for (auto node : loopSCCDAG->getNodes()) {
    auto scc = node->getT();
    this->sccToInfo[scc] = std::move(std::make_unique<SCCAttrs>(scc));

    this->collectPHIsAndAccumulators(scc);
    this->checkIfInductionVariableSCC(scc, SE);
    if (isInductionVariableSCC(scc)) this->checkIfSimpleIV(scc, LIS);
    this->checkIfClonable(scc, SE);

    if (this->checkIfIndependent(scc)) {
      scc->setType(SCC::SCCType::INDEPENDENT);
    } else if (this->checkIfCommutative(scc)) {
      scc->setType(SCC::SCCType::COMMUTATIVE);
    } else {
      scc->setType(SCC::SCCType::SEQUENTIAL);
    }
  }

  collectSCCGraphAssumingDistributedClones();
}

std::set<SCC *> SCCDAGAttrs::getSCCsWithLoopCarriedDataDependencies (void) const {
  std::set<SCC *> sccs;
  for (auto &sccInfoPair : this->sccToInfo) {
    if (sccInfoPair.second->isIndependent) continue ;
    sccs.insert(sccInfoPair.first);
  }
  return sccs;
}

/*
 * Assumption(angelo): An induction variable will be the root SCC of the loop
 */
bool SCCDAGAttrs::doesLoopHaveIV () const {
  auto topLevelNodes = sccdag->getTopLevelNodes();
  if (topLevelNodes.size() != 1) return false;
  auto scc = (*topLevelNodes.begin())->getT();
  return isInductionVariableSCC(scc);
}

bool SCCDAGAttrs::areAllLiveOutValuesReducable (LoopEnvironment *env) const {
  for (auto envIndex : env->getEnvIndicesOfLiveOutVars()) {
    auto producer = env->producerAt(envIndex);
    auto scc = sccdag->sccOfValue(producer);

    // TODO(angelo): Implement this if it is legal. Unsure at the moment
    // if (scc->getType() == SCC::SCCType::INDEPENDENT) continue ;
    if (scc->getType() == SCC::SCCType::COMMUTATIVE) continue ;
    scc->print(errs() << "NON COMM SCC THAT CAUSES NONREDUCABLE LIVE OUT:\n") << "\n";

    return false;
  }
  return true;
}

bool SCCDAGAttrs::canExecuteCommutatively (SCC *scc) const {
  return sccToInfo.find(scc)->second->isReducable;
}

bool SCCDAGAttrs::canExecuteIndependently (SCC *scc) const {
  return sccToInfo.find(scc)->second->isIndependent;
}

bool SCCDAGAttrs::canBeCloned (SCC *scc) const {
  return sccToInfo.find(scc)->second->isClonable;
}

bool SCCDAGAttrs::isInductionVariableSCC (SCC *scc) const {
  return sccToInfo.find(scc)->second->isIVSCC;
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

void SCCDAGAttrs::collectPHIsAndAccumulators (SCC *scc) {
  auto &sccInfo = this->getSCCAttrs(scc);
  for (auto iNodePair : scc->internalNodePairs()) {
    auto V = iNodePair.first;
    if (isa<CmpInst>(V) || isa<TerminatorInst>(V)) continue ;
    if (isa<GetElementPtrInst>(V) || isa<CastInst>(V)) continue ;

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

    /*
     * Fail to collect if an unexpected instruction is found
     */
    sccInfo->PHINodes.clear();
    sccInfo->accumulators.clear();
    return ;
  }

  if (sccInfo->PHINodes.size() == 1) {
    sccInfo->singlePHI = *sccInfo->PHINodes.begin();
  }
  if (sccInfo->accumulators.size() == 1) {
    sccInfo->singleAccumulator = *sccInfo->accumulators.begin();
  }
}

bool SCCDAGAttrs::checkIfCommutative (SCC *scc) {

  /*
   * Requirement: SCC has no data dependent SCCs
   */
  for (auto iNodePair : scc->externalNodePairs()) {
    if (iNodePair.second->numIncomingEdges() == 0) continue ;
    for (auto edge : iNodePair.second->getIncomingEdges()) {
      if (!edge->isControlDependence()) return false;
    }
  }

  /*
   * Requirement: 1+ accumulators that are all side effect free
   * Requirement: all accumulators act on one PHI/accumulator in the SCC
   *  and one constant or external value
   */
  auto &sccInfo = this->getSCCAttrs(scc);
  if (sccInfo->accumulators.size() == 0) return false;
  auto phis = sccInfo->PHINodes;
  auto accums = sccInfo->accumulators;
  for (auto accum : accums) {
    unsigned opCode = accum->getOpcode();
    if (accumOpInfo.sideEffectFreeOps.find(opCode) == accumOpInfo.sideEffectFreeOps.end()) {
      return false;
    }

    auto isIndependentOfSCC = [&](Value *val) -> bool {
      if (!scc->isInternal(val)) return false;
      auto node = scc->fetchNode(val);
      for (auto edge : node->getIncomingEdges()) {
        if (scc->isInternal(edge->getOutgoingT())) return false;
      }
      return true;
    };
    auto isExternalIndependentOrConstant = [&](Value *val) -> bool {
      if (isa<ConstantData>(val)) return true;
      if (scc->isExternal(val)) return true;
      return isIndependentOfSCC(val);
    };
    auto isInternalPHI = [&](Value *val) -> bool {
      return isa<PHINode>(val) && phis.find(cast<PHINode>(val)) != phis.end()
        && !isIndependentOfSCC(val);
    };
    auto isInternalAccum = [&](Value *val) -> bool {
      return isa<Instruction>(val) && accums.find(cast<Instruction>(val)) != accums.end();
    };
    auto isInternalPHIOrAccum = [&](Value *val) -> bool {
      if (auto cast = dyn_cast<CastInst>(val)) {
        return isInternalPHI(cast->getOperand(0)) || isInternalAccum(cast->getOperand(0));
      }
      return isInternalPHI(val) || isInternalAccum(val);
    };

    auto opL = accum->getOperand(0);
    auto opR = accum->getOperand(1);
    if (!(isExternalIndependentOrConstant(opL) ^ isExternalIndependentOrConstant(opR))) return false;
    if (!(isInternalPHIOrAccum(opL) ^ isInternalPHIOrAccum(opR))) return false;
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

  return this->getSCCAttrs(scc)->isReducable = true;
}

/*
 * The SCC is independent if it doesn't have loop carried data dependencies
 */
bool SCCDAGAttrs::checkIfIndependent (SCC *scc) {
  return this->getSCCAttrs(scc)->isIndependent = !scc->hasCycle(/*ignoreControlDep=*/false);
}

/*
 * TODO(angelo): More strictly verify that Cmp/Br instrs only use constants
 *  or PHINodes in the SCC
 */
bool SCCDAGAttrs::checkIfInductionVariableSCC (SCC *scc, ScalarEvolution &SE) {
  auto isIvSCC = true;
  bool hasPHINode = false;
  for (auto iNodePair : scc->internalNodePairs()) {
    auto V = iNodePair.first;
    auto canBePartOfSCC = isa<CmpInst>(V) || isa<TerminatorInst>(V);
    hasPHINode |= isa<PHINode>(V);

    auto scev = SE.getSCEV(V);
    switch (scev->getSCEVType()) {
    case scConstant:
    case scTruncate:
    case scZeroExtend:
    case scSignExtend:
    case scSMaxExpr:
    case scUMaxExpr:
    case scUDivExpr:
    case scAddExpr:
    case scMulExpr:
    case scAddRecExpr:
      continue;
    case scUnknown:
    case scCouldNotCompute:
      isIvSCC &= canBePartOfSCC;
      continue;
    default:
     llvm_unreachable("DSWP: Unknown SCEV type!");
    }
  }

  isIvSCC &= hasPHINode;
  return this->getSCCAttrs(scc)->isIVSCC = isIvSCC;
}

void SCCDAGAttrs::checkIfSimpleIV (SCC *scc, LoopInfoSummary &LIS) {

  /*
   * IV is described by single PHI with a start and recurrence incoming value
   * The IV has one accumulator only
   */
  auto &sccInfo = this->getSCCAttrs(scc);
  if (!sccInfo->singlePHI || !sccInfo->singleAccumulator) return;
  if (sccInfo->singlePHI->getNumIncomingValues() != 2) return;

  /*
   * The IV has one condition/conditional branch only
   */
  SimpleIVInfo IVInfo;
  for (auto iNodePair : scc->internalNodePairs()) {
    auto V = iNodePair.first;
    if (auto cmp = dyn_cast<CmpInst>(V)) {
      if (IVInfo.cmp) return;
      IVInfo.cmp = cmp;
    } else if (auto br = dyn_cast<BranchInst>(V)) {
      if (br->isUnconditional()) continue;
      if (IVInfo.br) return;
      IVInfo.br = br;
    }
  }
  if (!IVInfo.cmp || !IVInfo.br) return;

  auto accum = sccInfo->singleAccumulator;
  auto incomingStart = sccInfo->singlePHI->getIncomingValue(0);
  if (incomingStart == accum) incomingStart = sccInfo->singlePHI->getIncomingValue(1);
  IVInfo.start = incomingStart;

  /*
   * The IV recurrence is integer, by +-1 
   */
  auto stepValue = accum->getOperand(0);
  if (stepValue == sccInfo->singlePHI) stepValue = accum->getOperand(1);
  if (!isa<ConstantInt>(stepValue)) return;
  IVInfo.step = (ConstantInt*)stepValue;
  auto stepSize = IVInfo.step->getValue();
  if (stepSize != 1 && stepSize != -1) return;

  auto cmpLHS = IVInfo.cmp->getOperand(0);
  unsigned cmpToInd = cmpLHS == sccInfo->singlePHI || cmpLHS == accum;
  IVInfo.cmpIVTo = IVInfo.cmp->getOperand(cmpToInd);
  IVInfo.isCmpOnAccum = IVInfo.cmp->getOperand((cmpToInd + 1) % 2) == accum;
  IVInfo.isCmpIVLHS = cmpToInd;

  /*
   * The last value before the end value reached by the IV can be determined
   */
  if (!doesIVHaveSimpleEndVal(IVInfo, LIS)) return;

  sccInfo->simpleIVInfo = new SimpleIVInfo(IVInfo);
}

bool SCCDAGAttrs::doesIVHaveSimpleEndVal (SimpleIVInfo &ivInfo, LoopInfoSummary &LIS) {

  /*
   * Branch statement has two successors, one in the loop body, one outside the loop
   */
  auto loop = LIS.bbToLoop[ivInfo.br->getParent()];
  auto brLHSInLoop = loop->bbs.find(ivInfo.br->getSuccessor(0)) != loop->bbs.end();
  auto brRHSInLoop = loop->bbs.find(ivInfo.br->getSuccessor(1)) != loop->bbs.end();
  if (!(brLHSInLoop ^ brRHSInLoop)) return false;

  bool exitOnCmp = !brLHSInLoop;
  auto signedPred = ivInfo.cmp->isUnsigned() ? ivInfo.cmp->getSignedPredicate() : ivInfo.cmp->getPredicate();
  signedPred = ivInfo.isCmpIVLHS ? signedPred : ICmpInst::getSwappedPredicate(signedPred);
  int stepSize = ivInfo.step->getValue().getSExtValue();

  auto cmpPredAbort = [&]() -> void {
    errs() << "SCCDAGAttrs:   Error: comparison and branch of top level IV misunderstood\n";
    abort();
  };

  if (!exitOnCmp) { 
    if (stepSize == 1) {
      switch (signedPred) {
        case CmpInst::Predicate::ICMP_SLE:
          ivInfo.endOffset = 1;
        case CmpInst::Predicate::ICMP_NE:
        case CmpInst::Predicate::ICMP_SLT:
          break;
        default:
          cmpPredAbort();
      }
    } else {
      switch (signedPred) {
        case CmpInst::Predicate::ICMP_SGE:
          ivInfo.endOffset = -1;
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
          ivInfo.endOffset = 1;
        case CmpInst::Predicate::ICMP_SGE:
        case CmpInst::Predicate::ICMP_EQ:
          break;
        default:
          cmpPredAbort();
      }
    } else {
      switch (signedPred) {
        case CmpInst::Predicate::ICMP_SLT:
          ivInfo.endOffset = -1;
        case CmpInst::Predicate::ICMP_SLE:
        case CmpInst::Predicate::ICMP_EQ:
          break;
        default:
          cmpPredAbort();
      }
    }
  }

  ivInfo.endOffset -= stepSize * ivInfo.isCmpOnAccum;
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
   * Check if the current node of the SCCDAG is an SCC used by other nodes.
   */
  if (scc->numInternalNodes() == 1 || sccdag->fetchNode(scc)->numOutgoingEdges() == 0) {
    return false;
  }

  /*
   * The current node of the SCCDAG is an SCC.
   *
   * Check if this SCC can be removed exploiting induction variables.
   * In more detail, this SCC can be removed if the loop-carried data dependence, which has created this SCC in the PDG, is due to updates to induction variables.
   */
  if (this->isInductionVariableSCC(scc)) {
    return true;
  }

  return false;
}

bool SCCDAGAttrs::isClonableBySyntacticSugarInstrs (SCC *scc) const {

  /*
   * Check if the current node of the SCCDAG is an SCC used by other nodes.
   */
  if (scc->numInternalNodes() > 1 || sccdag->fetchNode(scc)->numOutgoingEdges() == 0) {
    return false;
  }

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
