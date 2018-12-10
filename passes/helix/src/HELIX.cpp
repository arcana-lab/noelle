#include "HELIX.hpp"
#include "HELIXTask.hpp"

HELIX::HELIX (Module &module, Verbosity v)
  :
  ParallelizationTechnique{module, v}
  {

  /*
   * Fetch the dispatcher to use to jump to a parallelized HELIX loop.
   */
  this->taskDispatcher = this->module.getFunction("helixDispatcher");

  return ;
}

bool HELIX::canBeAppliedToLoop (LoopDependenceInfo *LDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) const {
  return false;
}

bool HELIX::apply (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  Heuristics *h,
  ScalarEvolution &SE
) {

  return false;
}
