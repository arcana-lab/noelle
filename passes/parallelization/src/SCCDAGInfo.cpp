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

std::set<BasicBlock *> & SCCDAGInfo::getBasicBlocks (SCC *scc){
  auto &sccInfo = this->sccToInfo[scc];

  return sccInfo->bbs;
}

// TODO(angelo): find better workaround than just a getter for SCCInfo
std::unique_ptr<SCCInfo> & SCCDAGInfo::getSCCInfo (SCC *scc){
  return this->sccToInfo[scc];
}

void SCCDAGInfo::populate (SCCDAG *loopSCCDAG) {
  this->sccdag = loopSCCDAG;
  for (auto node : loopSCCDAG->getNodes()) {
    this->sccToInfo[node->getT()] = std::move(std::make_unique<SCCInfo>(node->getT()));
  }

  return ;
}
