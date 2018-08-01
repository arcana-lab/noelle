#include "DSWP.hpp"

using namespace llvm;

bool DSWP::applyDOALL (DSWPLoopDependenceInfo *LDI, Parallelization &par, Heuristics *h) {
  //TODO
  errs() << "DSWP:   IS DO ALL LOOP: ----------------------------\n";

  /*
   * Collect environment information
   */

  // Thoughts: start by copy pasta-ing environment and stage builder functionality
  // Use DOALL specific data structures, even if they share fields with DSWP ones
  // Find the overlap later and refactor

  // TODO: Adapt for DOALL usage
  // collectPreLoopEnvInfo(LDI);
  // LDI->envArrayType = ArrayType::get(PointerType::getUnqual(par.int8), LDI->environment->envSize());

  /*
   * Create DOALL chunking function
   */

  return false;
}
