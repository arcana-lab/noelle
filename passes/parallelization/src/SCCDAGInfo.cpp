#include "SCCDAGInfo.hpp"

using namespace llvm;

bool SCCDAGInfo::doesHaveLoopCarriedDataDependences (void) const {
  return true; //TODO
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
    
void SCCDAGInfo::populate (SCCDAG *loopSCCDAG) {
  this->sccdag = loopSCCDAG;
  for (auto node : loopSCCDAG->getNodes()) {
    this->sccToInfo[node->getT()] = std::move(std::make_unique<SCCInfo>(node->getT()));
  }

  return ;
}
