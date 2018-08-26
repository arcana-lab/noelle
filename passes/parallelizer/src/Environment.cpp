#include "Parallelizer.hpp"

using namespace llvm;

bool Parallelizer::allPostLoopEnvValuesAreReducable (DSWPLoopDependenceInfo *LDI) {
  for (auto envIndex : LDI->environment->getPostEnvIndices()) {
    auto producer = LDI->environment->producerAt(envIndex);
    auto scc = LDI->loopSCCDAG->sccOfValue(producer);

    if (scc->getType() != SCC::SCCType::COMMUTATIVE) {
      return false;
    }
  }
  return true;
}
