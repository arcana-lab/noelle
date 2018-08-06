#include "DSWP.hpp"

using namespace llvm;

void DSWP::collectParallelizableSingleInstrNodes (DSWPLoopDependenceInfo *LDI) {
  for (auto sccNode : LDI->loopSCCDAG->getNodes()) {
    if (sccNode->getT()->numInternalNodes() == 1) {
      LDI->singleInstrNodes.insert(sccNode->getT());
    }
  }

  return ;
}
