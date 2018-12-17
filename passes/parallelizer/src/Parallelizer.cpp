/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Parallelizer.hpp"

using namespace llvm;
  
bool Parallelizer::parallelizeLoop (DSWPLoopDependenceInfo *LDI, Parallelization &par, DSWP &dswp, DOALL &doall, HELIX &helix, Heuristics *h){

  /*
   * Assertions.
   */
  assert(LDI != nullptr);
  assert(h != nullptr);
  if (this->verbose != Verbosity::Disabled) {
    errs() << "Parallelizer: Start\n";
    errs() << "Parallelizer:  Function \"" << LDI->function->getName() << "\"\n";
    errs() << "Parallelizer:  Try to parallelize the loop \"" << *LDI->header->getFirstNonPHI() << "\"\n";
  }

  /*
   * Merge SCCs where separation is unnecessary.
   */
  mergeTrivialNodesInSCCDAG(LDI);

  /*
   * Collect information about the non-trivial SCCs
   */
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*LDI->function).getSE();
  collectSCCDAGAttrs(LDI, h, SE);

  /*
   * Parallelize the loop.
   */
  auto codeModified = false;
  Value *envArray;
  if (doall.canBeAppliedToLoop(LDI, par, h, SE)){

    /*
     * Apply DOALL.
     */
    codeModified = doall.apply(LDI, par, h, SE);
    envArray = doall.getEnvArray();
    doall.reset();

  } else if (helix.canBeAppliedToLoop(LDI, par, h, SE)) {

    /*
     * Apply HELIX
     */
    codeModified = helix.apply(LDI, par, h, SE);
    envArray = helix.getEnvArray();
    helix.reset();

  } else {
    dswp.initialize(LDI, h);
    if (dswp.canBeAppliedToLoop(LDI, par, h, SE)) {

      /*
       * Apply DSWP.
       */
      codeModified = dswp.apply(LDI, par, h, SE);
      envArray = dswp.getEnvArray();
    }
    dswp.reset();
  }

  /*
   * Check if the loop has been parallelized.
   */
  if (!codeModified){
    return false;
  }
  assert(LDI->entryPointOfParallelizedLoop != nullptr);
  assert(LDI->exitPointOfParallelizedLoop != nullptr);
  assert(envArray != nullptr);

  /*
   * The loop has been parallelized.
   *
   * Link the parallelized loop within the original function that includes the sequential loop.
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "Parallelizer:  Link the parallelize loop\n";
  }
  auto exitIndex = cast<Value>(ConstantInt::get(par.int64, LDI->environment->indexOfExitBlock()));
  par.linkParallelizedLoopToOriginalFunction(
    LDI->function->getParent(),
    LDI->preHeader,
    LDI->entryPointOfParallelizedLoop,
    LDI->exitPointOfParallelizedLoop,
    envArray,
    exitIndex,
    LDI->loopExitBlocks
  );
  if (this->verbose >= Verbosity::Maximal) {
    LDI->function->print(errs() << "Final printout:\n"); errs() << "\n";
  }

  /*
   * Return
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "Parallelizer: Exit\n";
  }
  return true;
}

void Parallelizer::collectSCCDAGAttrs (DSWPLoopDependenceInfo *LDI, Heuristics *h, ScalarEvolution &SE) {

  /*
   * Evaluate the SCCs (e.g., which ones are commutative) of the SCCDAG of the loop.
   */
  LDI->sccdagAttrs.populate(LDI->loopSCCDAG, LDI->liSummary, SE);
}
