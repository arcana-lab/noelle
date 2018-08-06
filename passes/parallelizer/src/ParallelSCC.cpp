#include "Parallelizer.hpp"

using namespace llvm;

void Parallelizer::collectParallelizableSingleInstrNodes (DSWPLoopDependenceInfo *LDI) {
  for (auto sccNode : LDI->loopSCCDAG->getNodes()) {
    if (sccNode->getT()->numInternalNodes() == 1) {
      LDI->singleInstrNodes.insert(sccNode->getT());
    }
  }

  return ;
}
