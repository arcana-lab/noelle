/*
 * Copyright 2019 - 2022  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Planner.hpp"
#include "TimingModel.hpp"

namespace arcana::noelle {

void Planner::removeLoopsNotWorthParallelizing(Noelle &noelle,
                                               Hot *profiles,
                                               LoopForest *forest) {

  /*
   * Check if we are force to consider all loops.
   */
  if (!this->forceParallelization) {

    /*
     * Filter out loops that are not worth parallelizing.
     */
    errs() << "Planner:  Filter out loops not worth considering\n";
    auto filter = [this, forest, profiles](LoopStructure *ls) -> bool {
      /*
       * Fetch the loop ID.
       */
      auto loopIDOpt = ls->getID();
      assert(loopIDOpt);
      auto loopID = loopIDOpt.value();

      /*
       * Check if the loop is executed at all.
       */
      if (profiles->getIterations(ls) == 0) {
        errs() << "Planner:    Loop " << loopID << " did not execute\n";

        /*
         * Remove the loop.
         */
        return true;
      }

      /*
       * Check if the latency of each loop invocation is enough to justify the
       * parallelization.
       */
      auto averageInstsPerInvocation =
          profiles->getAverageTotalInstructionsPerInvocation(ls);
      auto averageInstsPerInvocationThreshold = 2000;
      if (averageInstsPerInvocation < averageInstsPerInvocationThreshold) {
        errs() << "Planner:    Loop " << loopID << " has "
               << averageInstsPerInvocation
               << " number of instructions per loop invocation\n";
        errs() << "Planner:      It is too low. The threshold is "
               << averageInstsPerInvocationThreshold << "\n";

        /*
         * Remove the loop.
         */
        return true;
      }

      /*
       * Check the number of iterations per invocation.
       */
      auto averageIterations =
          profiles->getAverageLoopIterationsPerInvocation(ls);
      auto averageIterationThreshold = 12;
      if (averageIterations < averageIterationThreshold) {
        errs() << "Planner:    Loop " << loopID << " has " << averageIterations
               << " number of iterations on average per loop invocation\n";
        errs() << "Planner:      It is too low. The threshold is "
               << averageIterationThreshold << "\n";

        /*
         * Remove the loop.
         */
        return true;
      }

      /*
       * Check the minimum hotness
       */
      auto hotness = profiles->getDynamicTotalInstructionCoverage(ls) * 100;
      auto minimumHotness = 0.0;
      if (hotness < minimumHotness) {
        errs() << "Planner:    Loop " << loopID << " has only " << hotness
               << "\% coverage\n";
        errs() << "Planner:      It is too low. The threshold is "
               << minimumHotness << "\%\n";

        /*
         * Remove the loop.
         */
        return true;
      }

      return false;
    };
    noelle.filterOutLoops(forest, filter);
  }

  /*
   * Print the loops.
   */
  auto trees = forest->getTrees();
  errs() << "Planner:  There are " << trees.size()
         << " loop nesting trees in the program\n";
  for (auto tree : trees) {

    /*
     * Print the tree.
     */
    auto printTree = [profiles](noelle::LoopTree *n, uint32_t treeLevel) {
      /*
       * Fetch the loop information.
       */
      auto loopStructure = n->getLoop();
      auto loopFunction = loopStructure->getFunction();
      auto loopHeader = loopStructure->getHeader();

      /*
       * Fetch the loop ID.
       */
      auto loopIDOpt = loopStructure->getID();
      assert(loopIDOpt);
      auto loopID = loopIDOpt.value();

      /*
       * Compute the print prefix.
       */
      std::string prefix{ "Planner:    " };
      for (auto i = 1; i < treeLevel; i++) {
        prefix.append("  ");
      }

      /*
       * Print the loop.
       */
      errs() << prefix << "ID: " << loopID << " (" << treeLevel << ")\n";
      errs() << prefix << "  Function: \"" << loopFunction->getName() << "\"\n";
      errs()
          << prefix << "  Loop: \"" << *loopHeader->getFirstNonPHI() << "\"\n";
      errs() << prefix
             << "  Loop nesting level: " << loopStructure->getNestingLevel()
             << "\n";

      /*
       * Check if there are profiles.
       */
      if (!profiles->isAvailable()) {
        return false;
      }

      /*
       * Print the coverage of this loop.
       */
      auto hotness =
          profiles->getDynamicTotalInstructionCoverage(loopStructure) * 100;
      errs() << prefix << "  Hotness = " << hotness << " %\n";
      auto averageInstsPerInvocation =
          profiles->getAverageTotalInstructionsPerInvocation(loopStructure);
      errs() << prefix << "  Average instructions per invocation = "
             << averageInstsPerInvocation << " %\n";
      auto averageIterations =
          profiles->getAverageLoopIterationsPerInvocation(loopStructure);
      errs() << prefix
             << "  Average iterations per invocation = " << averageIterations
             << " %\n";
      errs() << prefix << "\n";

      return false;
    };
    tree->visitPreOrder(printTree);
  }

  return;
}

std::vector<LoopContent *> Planner::selectTheOrderOfLoopsToParallelize(
    Noelle &noelle,
    Hot *profiles,
    noelle::LoopTree *tree,
    uint64_t &maxTimeSaved,
    uint64_t &maxTimeSavedWithDOALLOnly) {
  std::vector<LoopContent *> selectedLoops{};

  /*
   * Fetch the verbosity.
   */
  auto verbose = noelle.getVerbosity();

  /*
   * Compute the amount of time that can be saved by a parallelization technique
   * per loop.
   */
  std::map<LoopContent *, uint64_t> timeSavedLoops;
  std::map<LoopStructure *, bool> doallLoops;
  std::map<LoopStructure *, uint64_t> timeSavedPerLoop;
  auto selector = [&noelle,
                   &timeSavedLoops,
                   &timeSavedPerLoop,
                   profiles,
                   &doallLoops](LoopTree *n, uint32_t treeLevel) -> bool {
    /*
     * Fetch the loop.
     */
    auto ls = n->getLoop();
    auto optimizations = { LoopContentOptimization::MEMORY_CLONING_ID,
                           LoopContentOptimization::THREAD_SAFE_LIBRARY_ID };
    auto ldi = noelle.getLoopContent(ls, optimizations);

    /*
     * Compute the timing model for this loop.
     */
    auto loopTimeModel = new LoopTimingModel(noelle, *ldi);

    /*
     * Tag DOALL loops.
     */
    if (loopTimeModel->getTimeSpentInCriticalPathPerIteration() == 0) {
      doallLoops[ls] = true;
    } else {
      doallLoops[ls] = false;
    }

    /*
     * Compute the maximum amount of time saved by any parallelization
     * technique.
     */
    auto timeSaved = loopTimeModel->getTimeSavedByParallelizingLoop();
    timeSavedLoops[ldi] = (uint64_t)timeSaved;
    timeSavedPerLoop[ls] = (uint64_t)timeSaved;

    return false;
  };
  tree->visitPreOrder(selector);

  /*
   * Filter out loops that should not be parallelized.
   */
  for (auto loopPair : timeSavedLoops) {

    /*
     * Fetch the loop.
     */
    auto ldi = loopPair.first;
    auto ls = ldi->getLoopStructure();

    /*
     * Get loop ID.
     */
    auto loopIDOpt = ls->getID();
    assert(loopIDOpt);
    auto loopID = loopIDOpt.value();

    /*
     * Compute the total amount of time saved by parallelizing this loop.
     */
    auto savedTimeTotal = ((double)timeSavedLoops[ldi])
                          / ((double)profiles->getTotalInstructions());
    savedTimeTotal *= 100;

    /*
     * Check if the time saved is enough.
     */
    if ((!this->forceParallelization) && (savedTimeTotal < 2)) {
      errs() << "Planner: LoopSelector:  Loop " << loopID << " saves only "
             << savedTimeTotal << " when parallelized. Skip it\n";
      continue;
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
  if (selectedLoops.size() == 0) {
    return {};
  }

  /*
   * Evaluate savings
   */
  auto timeSavedPair =
      this->evaluateSavings(noelle, tree, timeSavedPerLoop, doallLoops);
  maxTimeSaved = timeSavedPair.first;
  maxTimeSavedWithDOALLOnly = timeSavedPair.second;

  /*
   * Sort the loops depending on the amount of time that can be saved by a
   * parallelization technique.
   */
  auto compareOperator = [&timeSavedLoops](LoopContent *l1, LoopContent *l2) {
    auto s1 = timeSavedLoops[l1];
    auto s2 = timeSavedLoops[l2];
    if (s1 != s2) {
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
    errs()
        << "Planner: LoopSelector:   Order of loops and their maximum savings\n";
    for (auto l : selectedLoops) {

      /*
       * Fetch the loop information.
       */
      auto ls = l->getLoopStructure();
      auto loopHeader = ls->getHeader();
      auto loopFunction = ls->getFunction();

      /*
       * Get loop ID.
       */
      auto loopIDOpt = ls->getID();
      assert(loopIDOpt);
      auto loopID = loopIDOpt.value();

      /*
       * Compute the savings
       */
      auto savedTimeRelative = ((double)timeSavedLoops[l])
                               / ((double)profiles->getTotalInstructions(ls));
      auto savedTimeTotal = ((double)timeSavedLoops[l])
                            / ((double)profiles->getTotalInstructions());
      savedTimeRelative *= 100;
      savedTimeTotal *= 100;

      /*
       * Compute the coverage
       */
      auto hotness = profiles->getDynamicTotalInstructionCoverage(ls) * 100;

      /*
       * Print
       */
      errs() << "Planner: LoopSelector:    Loop " << loopID << "\n";
      errs() << "Planner: LoopSelector:      Function: \""
             << loopFunction->getName() << "\"\n";
      errs() << "Planner: LoopSelector:      Loop nesting level: "
             << ls->getNestingLevel() << "\n";
      errs() << "Planner: LoopSelector:      \""
             << *loopHeader->getFirstNonPHI() << "\"\n";
      errs() << "Planner: LoopSelector:      Coverage: " << hotness << "\%\n";
      errs() << "Planner: LoopSelector:      Whole-program savings = "
             << savedTimeTotal << "%\n";
      errs()
          << "Planner: LoopSelector:      Loop savings = " << savedTimeRelative
          << "%\n";
    }
    errs() << "Planner: LoopSelector: End\n";
  }

  return selectedLoops;
}
} // namespace arcana::noelle
