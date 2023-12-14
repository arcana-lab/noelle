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
#include "TimeSaved.hpp"
#include "TimingModel.hpp"

namespace arcana::noelle {

std::vector<LoopContent *> TimeSaved::selectTheOrderOfLoopsToParallelize(
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
    uint64_t timeSaved = 0;
    if (profiles->getIterations(ls) > 0) {
      timeSaved = loopTimeModel->getTimeSavedByParallelizingLoop();
    }
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
  errs() << "TimeSaved: LoopSelector: Start\n";
  errs()
      << "TimeSaved: LoopSelector:   Order of loops and their maximum savings\n";
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
    errs() << "TimeSaved: LoopSelector:    Loop " << loopID << "\n";
    errs() << "TimeSaved: LoopSelector:      Function: \""
           << loopFunction->getName() << "\"\n";
    errs() << "TimeSaved: LoopSelector:      Loop nesting level: "
           << ls->getNestingLevel() << "\n";
    errs() << "TimeSaved: LoopSelector:      \""
           << *loopHeader->getFirstNonPHI() << "\"\n";
    errs() << "TimeSaved: LoopSelector:      Coverage: " << hotness << "\%\n";
    errs() << "TimeSaved: LoopSelector:      Whole-program savings = "
           << savedTimeTotal << "%\n";
    errs()
        << "TimeSaved: LoopSelector:      Loop savings = " << savedTimeRelative
        << "%\n";
  }
  errs() << "TimeSaved: LoopSelector: End\n";

  return selectedLoops;
}
} // namespace arcana::noelle
