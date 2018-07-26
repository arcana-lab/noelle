#include "DSWP.hpp"

using namespace llvm;

void DSWP::estimateCostAndExtentOfParallelismOfSCCs (DSWPLoopDependenceInfo *LDI, Heuristics *h) {
  LDI->sccdagInfo.populate(LDI->loopSCCDAG);

  /*
   * Tag the SCCDAG nodes to have or not a loop-carried data dependence.
   */
  for (auto &sccInfoPair : LDI->sccdagInfo.sccToInfo) {

    /*
     * Fetch the SCC.
     */
    auto scc = sccInfoPair.first ;

    /*
     * Tag the SCC to be sequential or not.
     */
    LDI->sccdagInfo.setSCCToHaveLoopCarriedDataDependence(scc, scc->hasCycle());
  }

  /*
   * Estimate the latency of an invocation of an SCC.
   */
  for (auto &sccInfoPair : LDI->sccdagInfo.sccToInfo) {
    for (auto nodePair : sccInfoPair.first->internalNodePairs()) {
      auto I = cast<Instruction>(nodePair.first);
      sccInfoPair.second->internalCost += h->latencyPerInvocation(I);
    }
  }

  /*
   * Add information about the queues that the current SCC is connected to.
   */
  for (auto sccNode : LDI->loopSCCDAG->getNodes()) {

    /*
     * Fetch the SCC.
     */
    auto scc = sccNode->getT();

    /*
     * Fetch the information about the current SCC.
     */
    auto &sccInfo = LDI->sccdagInfo.sccToInfo[scc];

    /*
     * Check all outgoing edges of the current SCC.
     */
    for (auto edge : sccNode->getOutgoingEdges()) {

      /*
       * Skip self-dependences.
       */
      auto otherSCC = edge->getIncomingT();
      if (otherSCC == scc) continue;

      /*
       * Fetch the information about the SCC that is the destination of the current dependence.
       */
      auto &otherSCCInfo = LDI->sccdagInfo.sccToInfo[otherSCC];

      /*
       * Establish edge information between two SCC
       */
      sccInfo->sccToEdgeInfo[otherSCC] = std::move(std::make_unique<SCCEdgeInfo>());
      otherSCCInfo->sccToEdgeInfo[scc] = std::move(std::make_unique<SCCEdgeInfo>());

      /*
       * Collect edges representing possible queues
       */
      for (auto subEdge : edge->getSubEdges()) {
        auto queueVal = subEdge->getOutgoingT();
        sccInfo->sccToEdgeInfo[otherSCC]->edges.insert(queueVal);
        otherSCCInfo->sccToEdgeInfo[scc]->edges.insert(queueVal);
      }
    }
  }

  return ;
}

void DSWP::collectRemovableSCCsByInductionVars (DSWPLoopDependenceInfo *LDI) {
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*LDI->function).getSE();
  for (auto sccNode : LDI->loopSCCDAG->getNodes()) {

    /*
     * Check if the current node of the SCCDAG is an SCC used by other nodes.
     */
    if (sccNode->getT()->numInternalNodes() == 1 || sccNode->numOutgoingEdges() == 0) {
      continue;
    }

    /*
     * The current node of the SCCDAG is an SCC.
     *
     * Check if this SCC can be removed exploiting induction variables.
     * In more detail, this SCC can be removed if the loop-carried data dependence, which has created this SCC in the PDG, is due to updates to induction variables.
     */
    auto scc = sccNode->getT();
    auto isRemovableSCC = true;
    for (auto iNodePair : scc->internalNodePairs()) {
      auto V = iNodePair.first;
      auto canBePartOfRemovableSCC = isa<CmpInst>(V) || isa<TerminatorInst>(V);

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
        isRemovableSCC &= canBePartOfRemovableSCC;
        continue;
      default:
       llvm_unreachable("DSWP: Unknown SCEV type!");
      }
    }

    if (isRemovableSCC) {
      LDI->partitions.removableNodes.insert(scc);
    }
  }

  return ;
}

void DSWP::collectRemovableSCCsBySyntacticSugarInstrs (DSWPLoopDependenceInfo *LDI) {
  for (auto sccNode : LDI->loopSCCDAG->getNodes()) {
    auto scc = sccNode->getT();
    if (scc->numInternalNodes() > 1 || sccNode->numOutgoingEdges() == 0) continue;
    auto I = scc->begin_internal_node_map()->first;
    if (isa<PHINode>(I) || isa<GetElementPtrInst>(I) || isa<CastInst>(I)) {
      LDI->partitions.removableNodes.insert(scc);
    }
  }
}
