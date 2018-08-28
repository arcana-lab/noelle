#include "SCCDAGAttrs.hpp"

using namespace llvm;

std::set<SCC *> SCCDAGAttrs::getSCCsWithLoopCarriedDataDependencies (void) const {
  std::set<SCC *> sccs;
  for (auto &sccInfoPair : this->sccToInfo) {
    if (!sccInfoPair.second->hasLoopCarriedDataDep) continue ;
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

void SCCDAGAttrs::populate (SCCDAG *loopSCCDAG) {
  this->sccdag = loopSCCDAG;
  for (auto node : loopSCCDAG->getNodes()) {
    auto scc = node->getT();

    this->sccToInfo[scc] = std::move(std::make_unique<SCCAttrs>(scc));

    auto &sccInfo = this->sccToInfo[scc];
    sccInfo->hasLoopCarriedDataDep = scc->hasCycle(/*ignoreControlDep=*/false);

    if (!sccInfo->hasLoopCarriedDataDep) {
      scc->setType(SCC::SCCType::INDEPENDENT);
    } else if (this->executesCommutatively(scc)) {
      scc->setType(SCC::SCCType::COMMUTATIVE);
    } else {
      scc->setType(SCC::SCCType::SEQUENTIAL);
    }
  }
}

bool SCCDAGAttrs::executesCommutatively (SCC *scc) const {
  std::set<unsigned> sideEffectFreeBinOps = {
    Instruction::Add,
    Instruction::Mul,
    Instruction::Sub
  };

  /*
   * Requirement: SCC has no dependent SCCs
   */
  errs() << "Checking for dependent SCCs\n";
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
  unsigned opCode = cast<Instruction>(firstCommVal)->getOpcode();
  bool isFirstMul = opCode == Instruction::Mul;
  for (auto commValNode : commValNodes) {
    auto commVal = commValNode->getT();

    /*
     * Requirement: instructions are all Add/Sub or all Mul
     */
    bool isMul = cast<Instruction>(commVal)->getOpcode() == Instruction::Mul;
    if (isMul ^ isFirstMul) return false;
    errs() << "Share group of opcode\n";
  }

  errs() << "SUCCESS THIS IS A COMMUTATIVE SCC!\n";
  return true;
}
