#include "Parallelizer.hpp"

using namespace llvm;

bool Parallelizer::hasNonReducablePostLoopEnvVars (DSWPLoopDependenceInfo *LDI) {
  for (auto envIndex : LDI->environment->getPostEnvIndices()) {
    auto producer = LDI->environment->producerAt(envIndex);

    if (!LDI->loopSCCDAG->sccOfValue(producer)->executesAssociatively()) {
      return true;
    }
  }
  return false;
}
