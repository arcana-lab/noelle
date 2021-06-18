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
using namespace llvm::noelle;

namespace llvm::noelle {

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
      auto sequentialSCCs = DOALL::getSCCsThatBlockDOALLToBeApplicable(ldi, noelle);

      /*
       * Find the biggest sequential SCC.
       */
      uint64_t biggestSCCTime = 0;
      for (auto sequentialSCC : sequentialSCCs){

        /*
         * Fetch the time spent in the current SCC.
         */
        auto sequentialSCCTime = profiles->getTotalInstructions(sequentialSCC);

        /*
         * Compute the biggest SCC.
         */
        if (sequentialSCCTime > biggestSCCTime){
          biggestSCCTime = sequentialSCCTime;
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
     * Filter out loops that should not be parallelized.
     */
    for (auto loopPair : timeSavedLoops){

      /*
       * Fetch the loop.
       */
      auto ldi = loopPair.first;

      /*
       * Compute the total amount of time saved by parallelizing this loop.
       */
      auto savedTimeTotal = ((double)timeSavedLoops[ldi]) / ((double) profiles->getTotalInstructions());
      savedTimeTotal *= 100;
      
      /*
       * Check if the time saved is enough.
       */
      if (savedTimeTotal < 2){
        continue ;
      }

      /*
       * The loop is worth parallelizing it.
       *
       * Add it.
       */
      selectedLoops.push_back(ldi);
    }

    /*
     * Sort the loops depending on the amount of time that can be saved by a parallelization technique.
     */
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

        /*
         * Fetch the loop information.
         */
        auto ls = l->getLoopStructure();
        auto loopHeader = ls->getHeader();
        auto loopFunction = ls->getFunction();

        /*
         * Compute the savings
         */
        auto savedTimeRelative = ((double)timeSavedLoops[l]) / ((double) profiles->getTotalInstructions(ls));
        auto savedTimeTotal = ((double)timeSavedLoops[l]) / ((double) profiles->getTotalInstructions());
        savedTimeRelative *= 100;
        savedTimeTotal *= 100;

        /*
         * Print
         */
        errs() << "Parallelizer: LoopSelector:    Loop " << l->getID() << " " << ls->getID() << "\n";
        errs() << "Parallelizer: LoopSelector:      Function: \"" << loopFunction->getName() << "\"\n";
        errs() << "Parallelizer: LoopSelector:      Loop nesting level: " << ls->getNestingLevel() << "\n";
        errs() << "Parallelizer: LoopSelector:      \"" << *loopHeader->getFirstNonPHI() << "\"\n";
        errs() << "Parallelizer: LoopSelector:      Whole-program savings = " << savedTimeTotal << "%\n";
        errs() << "Parallelizer: LoopSelector:      Loop savings = " << savedTimeRelative << "%)\n";
      }
      errs() << "Parallelizer: LoopSelector: End\n";
    }

    return selectedLoops;
  }
}
