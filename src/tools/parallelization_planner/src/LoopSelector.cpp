/*
 * Copyright 2019 - 2021  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Planner.hpp"

namespace llvm::noelle {

  void Planner::removeLoopsNotWorthParallelizing (
    Noelle &noelle, 
    Hot *profiles,
    StayConnectedNestedLoopForest *forest
    ){

    /*
     * Filter out loops that are not worth parallelizing.
     */
    errs() << "Planner:  Filter out loops not worth considering\n";
    auto filter = [this, forest, profiles](LoopStructure *ls) -> bool{

      /*
       * Fetch the loop ID.
       */
      auto loopID = ls->getID();

      /*
       * Check if the loop is executed at all.
       */
      if (  true
          && (!this->forceParallelization)
          && (profiles->getIterations(ls) == 0)
         ){
        errs() << "Planner:    Loop " << loopID << " did not execute\n";

        /*
         * Remove the loop.
         */
        return true;
      }

      /*
       * Check if the latency of each loop invocation is enough to justify the parallelization.
       */
      auto averageInstsPerInvocation = profiles->getAverageTotalInstructionsPerInvocation(ls);
      auto averageInstsPerInvocationThreshold = 2000;
      if (  true
          && (!this->forceParallelization)
          && (averageInstsPerInvocation < averageInstsPerInvocationThreshold)
         ){
        errs() << "Planner:    Loop " << loopID << " has " << averageInstsPerInvocation << " number of instructions per loop invocation\n";
        errs() << "Planner:      It is too low. The threshold is " << averageInstsPerInvocationThreshold << "\n";

        /*
         * Remove the loop.
         */
        return true;
      }

      /*
       * Check the number of iterations per invocation.
       */
      auto averageIterations = profiles->getAverageLoopIterationsPerInvocation(ls);
      auto averageIterationThreshold = 12;
      if (    true
          && (!this->forceParallelization)
          && (averageIterations < averageIterationThreshold)
         ){
        errs() << "Planner:    Loop " << loopID << " has " << averageIterations << " number of iterations on average per loop invocation\n";
        errs() << "Planner:      It is too low. The threshold is " << averageIterationThreshold << "\n";

        /*
         * Remove the loop.
         */
        return true;
      }

      /*
       * Check the minimum hotness
       */
      auto hotness = profiles->getDynamicTotalInstructionCoverage(ls) * 100;
      auto minimumHotness = 2.0;
      if (      true
            &&  (!this->forceParallelization)
            &&  (hotness < minimumHotness)
         ){
        errs() << "Planner:    Loop " << loopID << " has only " << hotness << "\% coverage\n";
        errs() << "Planner:      It is too low. The threshold is " << minimumHotness << "\%\n";

        /*
         * Remove the loop.
         */
        return true;
      }

      return false;
    };
    noelle.filterOutLoops(forest, filter);

    /*
     * Print the loops.
     */
    auto trees = forest->getTrees();
    errs() << "Planner:  There are " << trees.size() << " loop nesting trees in the program\n";
    for (auto tree : trees){

      /*
       * Print the root.
       */
      auto loopStructure = tree->getLoop();
      auto loopID = loopStructure->getID();

      /*
       * Print the tree.
       */
      auto printTree = [profiles](noelle::StayConnectedNestedLoopForestNode *n, uint32_t treeLevel) {

        /*
         * Fetch the loop information.
         */
        auto loopStructure = n->getLoop();
        auto loopID = loopStructure->getID();
        auto loopFunction = loopStructure->getFunction();
        auto loopHeader = loopStructure->getHeader();

        /*
         * Compute the print prefix.
         */
        std::string prefix{"Planner:    "};
        for (auto i = 1 ; i < treeLevel; i++){
          prefix.append("  ");
        }

        /*
         * Print the loop.
         */
        errs() << prefix << "ID: " << loopID << " (" << treeLevel << ")\n";
        errs() << prefix << "  Function: \"" << loopFunction->getName() << "\"\n";
        errs() << prefix << "  Loop: \"" << *loopHeader->getFirstNonPHI() << "\"\n";
        errs() << prefix << "  Loop nesting level: " << loopStructure->getNestingLevel() << "\n";

        /*
         * Check if there are profiles.
         */
        if (!profiles->isAvailable()){
          return false;
        }

        /*
         * Print the coverage of this loop.
         */
        auto hotness = profiles->getDynamicTotalInstructionCoverage(loopStructure) * 100;
        errs() << prefix << "  Hotness = " << hotness << " %\n"; 
        auto averageInstsPerInvocation = profiles->getAverageTotalInstructionsPerInvocation(loopStructure);
        errs() << prefix << "  Average instructions per invocation = " << averageInstsPerInvocation << " %\n"; 
        auto averageIterations = profiles->getAverageLoopIterationsPerInvocation(loopStructure);
        errs() << prefix << "  Average iterations per invocation = " << averageIterations << " %\n"; 
        errs() << prefix << "\n";

        return false;
      };
      tree->visitPreOrder(printTree);
    }

    return ;
  }

  std::vector<LoopDependenceInfo *> Planner::selectTheOrderOfLoopsToParallelize (
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
      auto optimizations = { LoopDependenceInfoOptimization::MEMORY_CLONING_ID, LoopDependenceInfoOptimization::THREAD_SAFE_LIBRARY_ID};
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
      if (  true
            && (!this->forceParallelization)
            && (savedTimeTotal < 2)
         ){
        errs() << "Planner: LoopSelector:  Loop " << ldi->getID() << " saves only " << savedTimeTotal << " when parallelized. Skip it\n";
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
     * Check if there are loops
     */
    if (selectedLoops.size() == 0){
      return {};
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
      errs() << "Planner: LoopSelector: Start\n";
      errs() << "Planner: LoopSelector:   Order of loops and their maximum savings\n";
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
         * Compute the coverage
         */
        auto hotness = profiles->getDynamicTotalInstructionCoverage(ls) * 100;

        /*
         * Print
         */
        errs() << "Planner: LoopSelector:    Loop " << l->getID() << "\n";
        errs() << "Planner: LoopSelector:      Function: \"" << loopFunction->getName() << "\"\n";
        errs() << "Planner: LoopSelector:      Loop nesting level: " << ls->getNestingLevel() << "\n";
        errs() << "Planner: LoopSelector:      \"" << *loopHeader->getFirstNonPHI() << "\"\n";
        errs() << "Planner: LoopSelector:      Coverage: " << hotness << "\%\n";
        errs() << "Planner: LoopSelector:      Whole-program savings = " << savedTimeTotal << "%\n";
        errs() << "Planner: LoopSelector:      Loop savings = " << savedTimeRelative << "%\n";
      }
      errs() << "Planner: LoopSelector: End\n";
    }

    return selectedLoops;
  }
}
