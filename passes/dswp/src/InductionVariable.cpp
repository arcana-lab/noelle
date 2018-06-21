#include "DSWP.hpp"

using namespace llvm;

void DSWP::collectRemovableSCCsByInductionVars (DSWPLoopDependenceInfo *LDI)
{
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*LDI->function).getSE();
  auto &sccSubgraph = LDI->loopSCCDAG;
  for (auto sccNode : sccSubgraph->getNodes())
  {
    auto scc = sccNode->getT();
    if (sccNode->numOutgoingEdges() == 0) continue;

    bool isRemovableSCC = true;
    for (auto iNodePair : scc->internalNodePairs())
    {
      auto V = iNodePair.first;
      bool canBePartOfRemovableSCC = isa<CmpInst>(V) || isa<TerminatorInst>(V);

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
        // V->print(errs() << "Is not a removable instruction:\t"); errs() << "\n";
        continue;
      default:
       llvm_unreachable("Unknown SCEV type!");
      }
    }

    if (isRemovableSCC) LDI->removableSCCs.insert(scc);
    // if (isRemovableSCC) scc->print(errs() << "REMOVABLE SCC:\n") << "\n";
  }
}

void DSWP::addRemovableSCCsToStages (DSWPLoopDependenceInfo *LDI) {
  for (auto &stage : LDI->stages) {
    std::set<DGNode<SCC> *> visitedNodes;
    std::queue<DGNode<SCC> *> dependentSCCNodes;

    for (auto scc : stage->stageSCCs) {
      dependentSCCNodes.push(LDI->loopSCCDAG->fetchNode(scc));
    }

    while (!dependentSCCNodes.empty()) {
      auto depSCCNode = dependentSCCNodes.front();
      dependentSCCNodes.pop();

      for (auto sccEdge : depSCCNode->getIncomingEdges()) {
        auto fromSCCNode = sccEdge->getOutgoingNode();
        auto fromSCC = fromSCCNode->getT();
        if (visitedNodes.find(fromSCCNode) != visitedNodes.end()) continue;
        if (LDI->removableSCCs.find(fromSCC) == LDI->removableSCCs.end()) continue;

        stage->removableSCCs.insert(fromSCC);
        dependentSCCNodes.push(fromSCCNode);
        visitedNodes.insert(fromSCCNode);
      }
    }
  }
}
