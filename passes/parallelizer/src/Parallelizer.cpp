#include "Parallelizer.hpp"

using namespace llvm;
  
bool Parallelizer::parallelizeLoop (DSWPLoopDependenceInfo *LDI, Parallelization &par, DSWP &dswp, DOALL &doall, Heuristics *h){

  /*
   * Assertions.
   */
  assert(LDI != nullptr);
  assert(h != nullptr);
  if (this->verbose > Verbosity::Disabled) {
    errs() << "Parallelizer: Start\n";
    errs() << "Parallelizer:  Function \"" << LDI->function->getName() << "\"\n";
    errs() << "Parallelizer:  Try to parallelize the loop \"" << *LDI->header->getFirstNonPHI() << "\"\n";
  }

  /*
   * Merge SCCs where separation is unnecessary.
   */
  mergeTrivialNodesInSCCDAG(LDI);

  /*
   * Collect information about the SCCs.
   */
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*LDI->function).getSE();
  collectSCCDAGAttrs(LDI, h, SE);

  /*
   * Parallelize the loop.
   */
  auto codeModified = false;
  errs() << "CHECKING OCCURS NOW...\n";
  if (doall.canBeAppliedToLoop(LDI, par, h, SE)){

    /*
     * Apply DOALL.
     */
    codeModified = doall.apply(LDI, par, h, SE);
  } else {

    /*
     * Apply DSWP.
     */
    codeModified = dswp.apply(LDI, par, h, SE);
  }

  /*
   * Check if the loop has been parallelized.
   */
  if (!codeModified){
    return false;
  }
  assert(LDI->entryPointOfParallelizedLoop != nullptr);
  assert(LDI->exitPointOfParallelizedLoop != nullptr);

  /*
   * The loop has been parallelized.
   *
   * Link the parallelized loop within the original function that includes the sequential loop.
   */
  if (this->verbose > Verbosity::Disabled) {
    errs() << "Parallelizer:  Link the parallelize loop\n";
  }
  auto exitIndex = cast<Value>(ConstantInt::get(par.int64, LDI->environment->indexOfExitBlock()));
  par.linkParallelizedLoopToOriginalFunction(
    LDI->function->getParent(),
    LDI->preHeader,
    LDI->entryPointOfParallelizedLoop,
    LDI->exitPointOfParallelizedLoop,
    LDI->envArray,
    exitIndex,
    LDI->loopExitBlocks
  );
  if (this->verbose >= Verbosity::Pipeline) {
    LDI->function->print(errs() << "Final printout:\n"); errs() << "\n";
  }

  /*
   * Return
   */
  if (this->verbose > Verbosity::Disabled) {
    errs() << "Parallelizer: Exit\n";
  }
  return true;
}

void Parallelizer::collectSCCDAGAttrs (DSWPLoopDependenceInfo *LDI, Heuristics *h, ScalarEvolution &SE) {

  /*
   * Evaluate the SCCs (e.g., which ones are commutative) of the SCCDAG of the loop.
   */
  LDI->sccdagAttrs.populate(LDI->loopSCCDAG, SE);

  /*
   * Estimate the latency per invocation of the SCCs.
   */
  estimateCostAndExtentOfParallelismOfSCCs(LDI, h);

  /*
   * Keep track of which nodes of the SCCDAG are single instructions.
   */
  collectParallelizableSingleInstrNodes(LDI);
}
