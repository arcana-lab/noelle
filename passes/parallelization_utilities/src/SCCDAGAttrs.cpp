#include "SCCDAGAttrs.hpp"

using namespace llvm;

std::set<SCC *> SCCDAGAttrs::getSCCsWithLoopCarriedDataDependencies (void) const {
  std::set<SCC *> sccs;
  for (auto &sccInfoPair : this->sccToInfo) {
    if (!sccInfoPair.second->isIndependent) continue ;
    sccs.insert(sccInfoPair.first);
  }
  return sccs;
}

bool SCCDAGAttrs::loopHasInductionVariable (ScalarEvolution &SE) const {
  
  /*
   * Assumption(angelo): An induction variable will be the root SCC of the loop
   */
  auto topLevelNodes = sccdag->getTopLevelNodes();
  if (topLevelNodes.size() != 1) return false;

  auto scc = (*topLevelNodes.begin())->getT();
  return this->isInductionVariableSCC(SE, scc);
}

bool SCCDAGAttrs::isSCCContainedInSubloop (LoopInfoSummary &LIS, SCC *scc) const {
  bool instInSubloops = true;
  for (auto iNodePair : scc->internalNodePairs()) {
    auto bb = cast<Instruction>(iNodePair.first)->getParent();
    instInSubloops &= LIS.bbToLoop[bb] != LIS.topLoop;
  }
  return instInSubloops;
}

bool SCCDAGAttrs::isInductionVariableSCC (ScalarEvolution &SE, SCC *scc) const {
  auto isIvSCC = true;
  for (auto iNodePair : scc->internalNodePairs()) {
    auto V = iNodePair.first;
    auto canBePartOfSCC = isa<CmpInst>(V) || isa<TerminatorInst>(V);

    auto scev = SE.getSCEV(V);
    switch (scev->getSCEVType()) {
    case scConstant:
    case scTruncate:
    case scZeroExtend:
    case scSignExtend:
    case scAddExpr:
    case scMulExpr:
    case scUDivExpr:
    case scAddRecExpr:
    case scSMaxExpr:
    case scUMaxExpr:
      continue;
    case scUnknown:
    case scCouldNotCompute:
      isIvSCC &= canBePartOfSCC;
      continue;
    default:
     llvm_unreachable("DSWP: Unknown SCEV type!");
    }
  }
  return isIvSCC;
}

std::set<BasicBlock *> & SCCDAGAttrs::getBasicBlocks (SCC *scc){
  auto &sccInfo = this->sccToInfo[scc];

  return sccInfo->bbs;
}

bool SCCDAGAttrs::allPostLoopEnvValuesAreReducable (LoopEnvironment *env) const {
  for (auto envIndex : env->getPostEnvIndices()) {
    auto producer = env->producerAt(envIndex);
    auto scc = sccdag->sccOfValue(producer);

    scc->print(errs() << "SCC that has post env\n") << "\n";
    if (scc->getType() != SCC::SCCType::COMMUTATIVE) {
      return false;
    }
  }
  return true;
}

// REFACTOR(angelo): find better workaround than just a getter for SCCAttrs
std::unique_ptr<SCCAttrs> & SCCDAGAttrs::getSCCAttrs (SCC *scc){
  return this->sccToInfo[scc];
}

void SCCDAGAttrs::populate (SCCDAG *loopSCCDAG, ScalarEvolution &SE) {
  this->sccdag = loopSCCDAG;
  for (auto node : loopSCCDAG->getNodes()) {
    auto scc = node->getT();

    this->sccToInfo[scc] = std::move(std::make_unique<SCCAttrs>(scc));

    if (this->checkIfIndependent(scc)) {
      scc->setType(SCC::SCCType::INDEPENDENT);
    } else if (this->checkIfCommutative(scc)) {
      scc->setType(SCC::SCCType::COMMUTATIVE);
    } else {
      scc->setType(SCC::SCCType::SEQUENTIAL);
    }
  }
}

bool SCCDAGAttrs::checkIfCommutative (SCC *scc) {
  std::set<unsigned> sideEffectFreeBinOps = {
    Instruction::Add,
    Instruction::FAdd,
    Instruction::Mul,
    Instruction::FMul,
    Instruction::Sub,
    Instruction::FSub
  };

  /*
   * Requirement: SCC has no dependent SCCs
   */
  errs() << "Checking for dependent SCCs of SCC:\n";
  scc->print(errs()) << "\n";
  for (auto iNodePair : scc->externalNodePairs()) {
    if (iNodePair.second->numIncomingEdges() > 0) {
      return false;
    }
  }
  errs() << "------------------------------- Phew, isn't dependent\n";

  PHINode *singlePHI = nullptr;
  std::set<DGNode<Value> *> commValNodes;
  for (auto iNodePair : scc->internalNodePairs()) {
    Instruction *val = cast<Instruction>(iNodePair.first);
    val->print(errs() << "Checking val: "); errs() << "\n";

    /*
     * Requirement: one unique PHI node
     */
    if (isa<PHINode>(val)) {
      if (singlePHI) return false;
      singlePHI = (PHINode *)val;
      continue;
    }

    /*
     * Requirement: instructions are side effect free
     */
    unsigned opCode = val->getOpcode();
    if (sideEffectFreeBinOps.find(opCode) == sideEffectFreeBinOps.end()) {
      return false;
    }
    commValNodes.insert(iNodePair.second);
  }
  if (commValNodes.size() == 0) return true;

  /*
   * Requirement: commutative instructions alone do not form a cycle
   */
  SCC commValSCC(commValNodes, /*connectToExternalValues=*/false);
  errs() << "HAS CYCLE!?\n";
  if (commValSCC.hasCycle()) return false;
  errs() << "DOES NOT HAVE CYCLE!?\n";

  auto firstCommVal = (*commValNodes.begin())->getT();
  unsigned firstOpCode = cast<Instruction>(firstCommVal)->getOpcode();
  bool isFirstMul = firstOpCode == Instruction::Mul
    || firstOpCode == Instruction::FMul;
  for (auto commValNode : commValNodes) {
    auto commVal = commValNode->getT();

    /*
     * Requirement: instructions are all Add/Sub or all Mul
     */
    auto opCode = cast<Instruction>(commVal)->getOpcode();
    bool isMul = opCode == Instruction::Mul || opCode == Instruction::FMul;
    if (isMul ^ isFirstMul) return false;
    errs() << "Share group of opcode\n";
  }

  errs() << "SUCCESS THIS IS A COMMUTATIVE SCC!\n";
  this->getSCCAttrs(scc)->isReducable = true;
  return true;
}

bool SCCDAGAttrs::checkIfIndependent (SCC *scc) {

  /*
   * The SCC is independent if it doesn't have loop carried data dependencies
   */
  return this->getSCCAttrs(scc)->isIndependent = scc->hasCycle(/*ignoreControlDep=*/false);
}

void SCCDAGAttrs::checkIfClonable (SCC *scc, ScalarEvolution &SE) {
  checkIfClonableByInductionVars(scc, SE);
  checkIfClonableBySyntacticSugarInstrs(scc);
}

void SCCDAGAttrs::checkIfClonableByInductionVars (SCC *scc, ScalarEvolution &SE) {

  /*
   * Check if the current node of the SCCDAG is an SCC used by other nodes.
   */
  if (scc->numInternalNodes() == 1 || sccdag->fetchNode(scc)->numOutgoingEdges() == 0) {
    return ;
  }

  /*
   * The current node of the SCCDAG is an SCC.
   *
   * Check if this SCC can be removed exploiting induction variables.
   * In more detail, this SCC can be removed if the loop-carried data dependence, which has created this SCC in the PDG, is due to updates to induction variables.
   */
  if (this->isInductionVariableSCC(SE, scc)) {
    this->getSCCAttrs(scc)->isClonable = true;
  }
}

void SCCDAGAttrs::checkIfClonableBySyntacticSugarInstrs (SCC *scc) {

  /*
   * Check if the current node of the SCCDAG is an SCC used by other nodes.
   */
  if (scc->numInternalNodes() > 1 || sccdag->fetchNode(scc)->numOutgoingEdges() == 0) {
    return ;
  }

  auto I = scc->begin_internal_node_map()->first;
  if (isa<PHINode>(I) || isa<GetElementPtrInst>(I) || isa<CastInst>(I)) {
    this->getSCCAttrs(scc)->isClonable = true;
  }
}

bool SCCDAGAttrs::executesCommutatively (SCC *scc) {
  return this->getSCCAttrs(scc)->isReducable;
}

bool SCCDAGAttrs::executesIndependently (SCC *scc) {
  return this->getSCCAttrs(scc)->isIndependent;
}

bool SCCDAGAttrs::canBeCloned (SCC *scc) {
  return this->getSCCAttrs(scc)->isClonable;
}

