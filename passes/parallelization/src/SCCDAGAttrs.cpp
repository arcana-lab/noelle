#include "SCCDAGAttrs.hpp"

using namespace llvm;

bool SCCDAGAttrs::doesHaveLoopCarriedDataDependences (void) const {
  for (auto &sccInfoPair : this->sccToInfo) {
    if (sccInfoPair.second->hasLoopCarriedDep) return true;
  }
  return false;
}

void SCCDAGAttrs::setSCCToHaveLoopCarriedDataDependence (SCC *scc, bool doesItHaveLoopCarriedDataDependence){
  auto &sccInfo = this->sccToInfo[scc];
  sccInfo->hasLoopCarriedDep = doesItHaveLoopCarriedDataDependence;

  return ;
}

std::set<BasicBlock *> & SCCDAGAttrs::getBasicBlocks (SCC *scc){
  auto &sccInfo = this->sccToInfo[scc];

  return sccInfo->bbs;
}

// TODO(angelo): find better workaround than just a getter for SCCAttrs
std::unique_ptr<SCCAttrs> & SCCDAGAttrs::getSCCAttrs (SCC *scc){
  return this->sccToInfo[scc];
}

void SCCDAGAttrs::populate (SCCDAG *loopSCCDAG) {
  this->sccdag = loopSCCDAG;
  for (auto node : loopSCCDAG->getNodes()) {
    this->sccToInfo[node->getT()] = std::move(std::make_unique<SCCAttrs>(node->getT()));
  }

  return ;
}
