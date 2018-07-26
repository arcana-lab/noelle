#include "DSWP.hpp"

using namespace llvm;
  
bool DSWP::parallelizeLoop (DSWPLoopDependenceInfo *LDI, Parallelization &par, Heuristics *h){
  if (this->verbose > Verbosity::Disabled) {
    errs() << "DSWP: Start\n";
    errs() << "DSWP:  Try to parallelize the loop " << *LDI->header->getFirstNonPHI() << " of function " << LDI->function->getName() << "\n";
  }

  /*
   * Merge SCCs where separation is unnecessary.
   */
  mergeTrivialNodesInSCCDAG(LDI);

  /*
   * Collect information about the SCCs.
   */
  collectSCCDAGInfo(LDI, h);

  /*
   * Check the type of loop.
   */
  auto isDOALL = !LDI->sccdagInfo.doesHaveLoopCarriedDataDependences();
  auto codeModified = false;
  if (isDOALL){

    /*
     * Apply DOALL.
     */
    codeModified = this->applyDOALL(LDI, par, h);

  } else {

    /*
     * Apply DSWP.
     */
    codeModified = this->applyDSWP(LDI, par, h);
  }

  /*
   * Check if the loop has been parallelized.
   */
  if (!codeModified){
    return false;
  }

  /*
   * The loop has been parallelized.
   *
   * Link the parallelized loop within the original function that includes the sequential loop.
   */
  if (this->verbose > Verbosity::Disabled) {
    errs() << "DSWP:  Link the parallelize loop\n";
  }
  auto exitIndex = cast<Value>(ConstantInt::get(par.int64, LDI->environment->indexOfExitBlock()));
  par.linkParallelizedLoopToOriginalFunction(LDI->function->getParent(), LDI->preHeader, LDI->pipelineBB, LDI->envArray, exitIndex, LDI->loopExitBlocks);
  if (this->verbose >= Verbosity::Pipeline) {
    LDI->function->print(errs() << "Final printout:\n"); errs() << "\n";
  }

  /*
   * Return
   */
  if (this->verbose > Verbosity::Disabled) {
    errs() << "DSWP: Exit\n";
  }
  return true;
}

void DSWP::collectSCCDAGInfo (DSWPLoopDependenceInfo *LDI, Heuristics *h) {
  estimateCostAndExtentOfParallelismOfSCCs(LDI, h);

  /*
   * Keep track of which nodes of the SCCDAG are single instructions.
   */
  collectParallelizableSingleInstrNodes(LDI);

  /*
   * Keep track of the SCCs that can be removed.
   */
  collectRemovableSCCsBySyntacticSugarInstrs(LDI);
  collectRemovableSCCsByInductionVars(LDI);

  return ;
}
