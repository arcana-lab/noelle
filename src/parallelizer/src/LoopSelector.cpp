/*
 * Copyright 2019 - 2020  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Parallelizer.hpp"

using namespace llvm;

std::vector<LoopDependenceInfo *> Parallelizer::selectTheOrderOfLoopsToParallelize (
  Noelle &noelle, 
  Hot *profiles,
  noelle::StayConnectedNestedLoopForestNode *tree
  ) {
  std::vector<LoopDependenceInfo *> selectedLoops{};

  /*
   * Fetch the verbosity.
   */
  auto verbose = noelle.getVerbosity();

  /*
   * Compute the amount of time that can be saved by a parallelization technique per loop.
   */
  std::map<LoopDependenceInfo *, uint64_t> timeSavedLoops;
  auto selector = [&noelle, &timeSavedLoops, profiles](StayConnectedNestedLoopForestNode *n, uint32_t treeLevel) -> bool {

    /*
     * Fetch the loop.
     */
    auto ls = n->getLoop();
    auto optimizations = { LoopDependenceInfoOptimization::MEMORY_CLONING_ID };
    auto ldi = noelle.getLoop(ls, optimizations);

    /*
     * Fetch the set of sequential SCCs.
     */
    auto SCCManager = ldi->getSCCManager();
    auto sequentialSCCs = SCCManager->getSCCsOfType(SCCAttrs::SCCType::SEQUENTIAL);

    /*
     * Find the biggest sequential SCC.
     */
    uint64_t biggestSCCTime = 0;
    for (auto sequentialSCC : sequentialSCCs){
      assert(sequentialSCC->mustExecuteSequentially());

      /*
       * Fetch the SCC.
       */
      auto currentSCC = sequentialSCC->getSCC();

      /*
       * Check if the SCC can be removed by a transformation.
       */
      if (sequentialSCC->isInductionVariableSCC()){
        continue ;
      }
      if (sequentialSCC->canBeCloned()){
        continue ;
      }
      if (sequentialSCC->canBeClonedUsingLocalMemoryLocations()){
        continue ;
      }

      auto areAllDataLCDsFromDisjointMemoryAccesses = true;
      auto domainSpaceAnalysis = ldi->getLoopIterationDomainSpaceAnalysis();
      ldi->sccdagAttrs.iterateOverLoopCarriedDataDependences(currentSCC, [
        &areAllDataLCDsFromDisjointMemoryAccesses, domainSpaceAnalysis
      ](DGEdge<Value> *dep) -> bool {
        if (dep->isControlDependence()) return false;

        if (!dep->isMemoryDependence()) {
          areAllDataLCDsFromDisjointMemoryAccesses = false;
          return true;
        }

        auto fromInst = dyn_cast<Instruction>(dep->getOutgoingT());
        auto toInst = dyn_cast<Instruction>(dep->getIncomingT());
        areAllDataLCDsFromDisjointMemoryAccesses &= fromInst && toInst && domainSpaceAnalysis->
          areInstructionsAccessingDisjointMemoryLocationsBetweenIterations(fromInst, toInst);
        return !areAllDataLCDsFromDisjointMemoryAccesses;
      });
      if (areAllDataLCDsFromDisjointMemoryAccesses) {
        continue;
      }

      /*
       * Fetch the time spent in the current SCC.
       */
      auto currentSCCTime = profiles->getTotalInstructions(currentSCC);

      /*
       * Compute the biggest SCC.
       */
      if (currentSCCTime > biggestSCCTime){
        biggestSCCTime = currentSCCTime;
      }
    }

    /*
     * Compute the maximum amount of time saved by any parallelization technique.
     */
    timeSavedLoops[ldi] = 0;
    if (profiles->getIterations(ls) > 0){
      auto instsPerIteration = profiles->getAverageTotalInstructionsPerIteration(ls);
      auto instsInBiggestSCCPerIteration = ((double)biggestSCCTime) / ((double)profiles->getIterations(ls));
      assert(instsInBiggestSCCPerIteration <= instsPerIteration);
      auto timeSavedPerIteration = (double)(instsPerIteration - instsInBiggestSCCPerIteration);
      auto timeSaved = timeSavedPerIteration * profiles->getIterations(ls);
      timeSavedLoops[ldi] = (uint64_t)timeSaved;
    }

    return false;
  };
  tree->visitPreOrder(selector);

  /*
   * Sort the loops depending on the amount of time that can be saved by a parallelization technique.
   */
  for (auto loopPair : timeSavedLoops){

    /*
     * Fetch the loop.
     */
    auto ldi = loopPair.first;

    /*
     * Add it.
     */
    selectedLoops.push_back(ldi);
  }
  auto compareOperator = [&timeSavedLoops](LoopDependenceInfo *l1, LoopDependenceInfo *l2){
    auto s1 = timeSavedLoops[l1];
    auto s2 = timeSavedLoops[l2];
    if (s1 != s2){
      return s1 > s2;
    }

    /*
     * The loops have the same saved time.
     * Sort them by nesting level.
     */
    auto l1LS = l1->getLoopStructure();
    auto l2LS = l2->getLoopStructure();
    return l1LS->getNestingLevel() < l2LS->getNestingLevel();
  };
  std::sort(selectedLoops.begin(), selectedLoops.end(), compareOperator);

  /*
   * Print the order and the savings.
   */
  if (verbose != Verbosity::Disabled) {
    errs() << "Parallelizer: LoopSelector: Start\n";
    errs() << "Parallelizer: LoopSelector:   Order of loops and their maximum savings\n";
    for (auto l : selectedLoops){
      auto ls = l->getLoopStructure();
      auto savedTimeRelative = ((double)timeSavedLoops[l]) / ((double) profiles->getTotalInstructions(ls));
      savedTimeRelative *= 100;
      errs() << "Parallelizer: LoopSelector:    Loop " << l->getID() << " savings = " << savedTimeRelative << "%\n";
    }
    errs() << "Parallelizer: LoopSelector: End\n";
  }

  return selectedLoops;
}
