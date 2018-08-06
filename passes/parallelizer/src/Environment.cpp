#include "Parallelizer.hpp"

using namespace llvm;

bool Parallelizer::hasPostLoopEnvVars (DSWPLoopDependenceInfo *LDI) {
  for (auto nodeI : LDI->loopDG->externalNodePairs()) {

    /*
     * Determine whether the external value is a consumer of loop-internal values
     */
    for (auto incomingEdge : nodeI.second->getIncomingEdges()) {
      if (!incomingEdge->isMemoryDependence() && !incomingEdge->isControlDependence()) return true;
    }
  }
  return false;
}
