/*
 * Copyright 2019 - 2020 Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "EnablersManager.hpp"

using namespace llvm;

bool EnablersManager::applyEnablers (
    LoopDependenceInfo *LDI,
    Parallelization &par,
    LoopDistribution &loopDist,
    LoopUnroll &loopUnroll
  ){

  /*
   * Apply loop distribution.
   */
  errs() << "EnablersManager:   Try to apply loop distribution\n";
  if (this->applyLoopDistribution(LDI, par, loopDist)){
    errs() << "EnablersManager:     Distributed loop\n";
    return true;
  }

  /*
   * Apply loop unrolling.
   */
  errs() << "EnablersManager:   Try to apply loop unrolling\n";
  if (this->applyLoopUnroll(LDI, par, loopUnroll)){
    errs() << "EnablersManager:     Unrolled loop\n";
    return true;
  }

  return false;
}

bool EnablersManager::applyLoopDistribution (
    LoopDependenceInfo *LDI,
    Parallelization &par,
    LoopDistribution &loopDist
  ){

  /*
   * Fetch the SCCDAG of the loop.
   */
  auto SCCDAG = LDI->sccdagAttrs.getSCCDAG();

  /*
   * Define the set of SCCs to bring outside the loop.
   */
  std::set<SCC *> SCCsToBringOutsideParallelizedLoop{};

  /*
   * Collect all sequential SCCs.
   */
  std::set<SCC *> sequentialSCCs{};
  auto collectSequentialSCCsFunction = [LDI,&sequentialSCCs](SCC *currentSCC) -> bool {

    /*
     * Fetch the SCC metadata.
     */
    auto sccInfo = LDI->sccdagAttrs.getSCCAttrs(currentSCC);

    /*
     * Check if the current SCC can be removed (e.g., because it is due to induction variables).
     * If it is, then we do not need to remove it from the loop to be parallelized.
     */
    if (!sccInfo->mustExecuteSequentially()) {
      return false;
    }

    /*
     * The current SCC must run sequentially.
     */
    sequentialSCCs.insert(currentSCC);
    return false;
  } ;
  SCCDAG->iterateOverSCCs(collectSequentialSCCsFunction);

  /*
   * Check every sequential SCC of the loop and decide which ones to bring outside the loop to parallelize.
   */
  for (auto SCC : sequentialSCCs){

    /*
     * Try to bring the sequential SCC outside the loop.
     */
    std::set<Instruction *> instsRemoved;
    std::set<Instruction *> instsAdded;
    auto splitted = loopDist.splitLoop(*LDI, SCC, instsRemoved, instsAdded);
    if (!splitted){
      continue ;
    }

    /*
     * The SCC has been pulled out the loop.
     * We need to update all metadata about loops.
     * To do so, we currently quit and rerun noelle-enable
     */
    return true;
  }
  
  return false;
}

bool EnablersManager::applyLoopUnroll (
    LoopDependenceInfo *LDI,
    Parallelization &par,
    LoopUnroll &loopUnroll
  ){

  /*
   * We want to fully unroll a loop if this can help the parallelization of an outer loop that includes it.
   *
   * One condition that allow this improvement is when the inner loop iterates over function pointers.
   * For example:
   * while (...){
   *    for (auto i=0; i < 10; i++){
   *      auto functionPtr = array[i];
   *      (*functionPtr)(...)
   *    }
   * }
   */

  /*
   * Fetch the loop.
   */
  auto ls = LDI->getLoopSummary();

  /*
   * Check if the loop belongs within another loop.
   */
  if (ls->getNestingLevel() == 1){

    /*
     * This is an outermost loop in a function.
     */
    return false;
  }

  /*
   * Fully unroll the loop.
   */
  auto &loopFunction = *ls->getFunction();
  auto& LS = getAnalysis<LoopInfoWrapperPass>(loopFunction).getLoopInfo();
  auto& DT = getAnalysis<DominatorTreeWrapperPass>(loopFunction).getDomTree();
  auto& SE = getAnalysis<ScalarEvolutionWrapperPass>(loopFunction).getSE();
  auto& AC = getAnalysis<AssumptionCacheTracker>().getAssumptionCache(loopFunction);
  auto modified = loopUnroll.fullyUnrollLoop(*LDI, LS, DT, SE, AC);
   
  return modified;
}
