#include "DSWP.hpp"

using namespace llvm;

void DSWP::estimateCostAndExtentOfParallelismOfSCCs (DSWPLoopDependenceInfo *LDI) {
  LDI->sccdagInfo.populate(LDI->loopSCCDAG);

  /*
   * Check whether each SCC has a cycle
   */
  for (auto &sccInfoPair : LDI->sccdagInfo.sccToInfo) {
    sccInfoPair.second->hasLoopCarriedDep = sccInfoPair.first->hasCycle();
  }

  /*
   * Estimate each SCC's cost
   */
  auto isSyntacticSugar = [&](Instruction *I) -> bool {
    return isa<PHINode>(I) || isa<GetElementPtrInst>(I) || isa<CastInst>(I);
  };
  std::unordered_map<Function *, int> funcToCost;
  for (auto &sccInfoPair : LDI->sccdagInfo.sccToInfo) {
    
    for (auto nodePair : sccInfoPair.first->internalNodePairs()) {
      auto I = cast<Instruction>(nodePair.first);
      if (isSyntacticSugar(I)) continue;

      if (auto call = dyn_cast<CallInst>(I)) {
        auto F = call->getCalledFunction();
        
        /*
         * Compute function cost as 1-layer deep instruction tally
         */
        if (funcToCost.find(F) != funcToCost.end()) {
          sccInfoPair.second->cost += funcToCost[F];
        } else {
          auto instInF = 0;
          if (!F || F->empty()) {
            instInF = 10;
          } else {
            for (auto &B : *F) {
              for (auto &J : B) {
                if (!isSyntacticSugar(&J)) instInF++;
              }
            }
          }
          sccInfoPair.second->cost += instInF;
          funcToCost[F] = instInF;
        }
      } else {
        sccInfoPair.second->cost++;
      }
    }
  }
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