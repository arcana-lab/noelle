#include "SCCDAGInfo.hpp"

using namespace llvm;

bool SCCDAGInfo::doesHaveLoopCarriedDataDependences (void) const {
  for (auto &sccInfoPair : this->sccToInfo) {
    if (sccInfoPair.second->hasLoopCarriedDep) return true;
  }
  return false;
}

void SCCDAGInfo::setSCCToHaveLoopCarriedDataDependence (SCC *scc, bool doesItHaveLoopCarriedDataDependence){
  auto &sccInfo = this->sccToInfo[scc];
  sccInfo->hasLoopCarriedDep = doesItHaveLoopCarriedDataDependence;

  return ;
}

void SCCDAGInfo::setQueueableValCost (Value *val, int cost){
  this->queueableValToCost[val] = cost;
}

std::set<BasicBlock *> & SCCDAGInfo::getBasicBlocks (SCC *scc){
  auto &sccInfo = this->sccToInfo[scc];

  return sccInfo->bbs;
}

// TODO(angelo): find better workaround than just a getter for SCCInfo
std::unique_ptr<SCCInfo> & SCCDAGInfo::getSCCInfo (SCC *scc){
  return this->sccToInfo[scc];
}

int SCCDAGInfo::getSCCSubsetCost (std::set<SCC *> &sccs){
  int cost = 0;
  for (auto scc : sccs) {
    auto &sccInfo = this->sccToInfo[scc];

    /*
     * Collect scc internal information 
     */
    cost += sccInfo->internalCost;

    /*
     * Collect scc external cost (through edges)
     */
    std::set<Value *> incomingEdges;
    for (auto &sccEdgesPair : this->sccToInfo[scc]->sccToEdgeInfo) {
      if (sccs.find(sccEdgesPair.first) != sccs.end()) continue;
      auto &edges = sccEdgesPair.second->edges;
      incomingEdges.insert(edges.begin(), edges.end());
    }

    for (auto edgeVal : incomingEdges) {
      cost += this->queueableValToCost[edgeVal];
    }
  }
  return cost;
}

void SCCDAGInfo::populate (SCCDAG *loopSCCDAG) {
  this->sccdag = loopSCCDAG;
  for (auto node : loopSCCDAG->getNodes()) {
    this->sccToInfo[node->getT()] = std::move(std::make_unique<SCCInfo>(node->getT()));
  }

  return ;
}
