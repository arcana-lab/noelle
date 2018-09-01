#include "Parallelizer.hpp"

using namespace llvm;

void Parallelizer::estimateCostAndExtentOfParallelismOfSCCs (DSWPLoopDependenceInfo *LDI, Heuristics *h) {

  /*
   * Estimate the latency of an invocation of an SCC.
   */
  for (auto sccNode : LDI->loopSCCDAG->getNodes()) {
    auto scc = sccNode->getT();

    auto &sccInfo = LDI->sccdagAttrs.getSCCAttrs(scc);

    sccInfo->internalCost = h->latencyPerInvocation(scc);
  }

  /*
   * Add information about the queues that the current SCC is connected to.
   */
  for (auto sccNode : LDI->loopSCCDAG->getNodes()) {
    auto scc = sccNode->getT();

    /*
     * Fetch the information about the current SCC.
     */
    auto &sccInfo = LDI->sccdagAttrs.getSCCAttrs(scc);

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
      auto &otherSCCAttrs = LDI->sccdagAttrs.getSCCAttrs(otherSCC);

      /*
       * Establish edge information between two SCC
       */
      sccInfo->sccToEdgeInfo[otherSCC] = std::move(std::make_unique<SCCEdgeInfo>());
      otherSCCAttrs->sccToEdgeInfo[scc] = std::move(std::make_unique<SCCEdgeInfo>());

      /*
       * Collect edges representing possible queues
       */
      for (auto subEdge : edge->getSubEdges()) {
        auto queueVal = subEdge->getOutgoingT();

        sccInfo->sccToEdgeInfo[otherSCC]->edges.insert(queueVal);
        otherSCCAttrs->sccToEdgeInfo[scc]->edges.insert(queueVal);
      }
    }
  }

  return ;
}

