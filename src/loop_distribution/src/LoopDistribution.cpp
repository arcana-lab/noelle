#include "LoopDistribution.hpp"

using namespace llvm;
 
bool LoopDistribution::splitLoop (
  LoopDependenceInfo const &LDI, 
  SCC *SCCToPullOut
  ){
  std::set<SCC *> SCCs{};
  SCCs.insert(SCCToPullOut);
  auto modified = this->splitLoop(LDI, SCCs);
  return modified;
}


bool LoopDistribution::splitLoop (
  LoopDependenceInfo const &LDI, 
  std::set<SCC *> const &SCCsToPullOut
  ){
  //TODO
  return false;
}
