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
  
bool Parallelizer::parallelizeLoop (
  LoopDependenceInfo *LDI, 
  Parallelization &par, 
  DSWP &dswp, 
  DOALL &doall, 
  HELIX &helix, 
  Heuristics *h,
  LoopDistribution &loopDist
  ){

  /*
   * Assertions.
   */
  assert(LDI != nullptr);
  assert(h != nullptr);
  if (this->verbose != Verbosity::Disabled) {
    errs() << "Parallelizer: Start\n";
    errs() << "Parallelizer:  Function = \"" << LDI->function->getName() << "\"\n";
    errs() << "Parallelizer:  Loop " << LDI->getID() << " = \"" << *LDI->header->getFirstNonPHI() << "\"\n";
  }

  /*
   * Apply parallelization enablers.
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "Parallelizer:  Run enablers\n";
  }
  auto enablersModifiedCode = this->applyEnablers(LDI, par, loopDist);
  if (enablersModifiedCode){
    if (this->verbose != Verbosity::Disabled) {
      errs() << "Parallelizer:    Enablers have modified the code\n";
      errs() << "Parallelizer: Exit\n";
    }
    return true;
  }

  /*
   * Gauge the limits of each parallelization scheme
   */
  auto numDSWPDependencies = 0, numHELIXDependencies = 0;
  for (auto valueToSCCs : LDI->sccdagAttrs.intraIterDeps) {
    numDSWPDependencies += valueToSCCs.second.size();
  }
  for (auto SCCToEdges : LDI->sccdagAttrs.interIterDeps) {
    numHELIXDependencies += SCCToEdges.second.size();
  }
  if (this->verbose >= Verbosity::Minimal) {
    if (numDSWPDependencies > numHELIXDependencies) { 
      errs() << "Parallelizer:  In theory, HELIX would be more effective than DSWP\n";
    } else if (numDSWPDependencies < numHELIXDependencies) { 
      errs() << "Parallelizer:  In theory, DSWP would be more effective than HELIX\n";
    } else {
      errs() << "Parallelizer:  In theory, DSWP would be as effective as HELIX\n";
    }
  }

  /*
   * Parallelize the loop.
   */
  auto codeModified = false;
  ParallelizationTechnique *usedTechnique = nullptr;
  if (  true
        && (this->enabledTechniques.find(DOALL_ID) != this->enabledTechniques.end())
        && LDI->isTechniqueEnabled(DOALL_ID)
        && doall.canBeAppliedToLoop(LDI, par, h)
    ){

    /*
     * Apply DOALL.
     */
    doall.reset();
    codeModified = doall.apply(LDI, par, h);
    usedTechnique = &doall;

  } else if ( true
              && (this->enabledTechniques.find(HELIX_ID) != this->enabledTechniques.end())
              && LDI->isTechniqueEnabled(HELIX_ID)
              && helix.canBeAppliedToLoop(LDI, par, h)   
    ){

    /*
     * Apply HELIX
     */
    helix.reset();
    codeModified = helix.apply(LDI, par, h);

    auto function = helix.getTaskFunction();
    auto fPDG = getAnalysis<PDGAnalysis>().getFunctionPDG(*function);
    auto &LI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();
    auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*function).getSE();
    auto l = LI.getLoopsInPreorder()[0]; //TODO: SIMONE: how do we know that the loop we want to parallelize is [0] ?
    auto newLDI = new LoopDependenceInfo(function, fPDG, l, LI, SE);
    newLDI->copyParallelizationOptionsFrom(LDI);

    codeModified = helix.apply(newLDI, par, h);
    usedTechnique = &helix;

  } else if ( true
              && (this->enabledTechniques.find(DSWP_ID) != this->enabledTechniques.end())
              && LDI->isTechniqueEnabled(DSWP_ID)
              && dswp.canBeAppliedToLoop(LDI, par, h)
    ) {

    /*
     * Apply DSWP.
     */
    dswp.reset();
    codeModified = dswp.apply(LDI, par, h);
    usedTechnique = &dswp;
  }

  /*
   * Check if the loop has been parallelized.
   */
  if (!codeModified){
    return false;
  }

  /*
   * Fetch the environment array where the exit block ID has been stored.
   */
  auto envArray = usedTechnique->getEnvArray();
  assert(envArray != nullptr);

  /*
   * Fetch entry and exit point executed by the parallelized loop.
   */
  auto entryPoint = usedTechnique->getParLoopEntryPoint();
  auto exitPoint = usedTechnique->getParLoopExitPoint();
  assert(entryPoint != nullptr && exitPoint != nullptr);

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
    entryPoint,
    exitPoint, 
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
