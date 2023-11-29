/*
 * Copyright 2021 - 2022  Simone Campanoni
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

namespace arcana::noelle {

std::pair<uint64_t, uint64_t> TimeSaved::evaluateSavings(
    Noelle &noelle,
    noelle::LoopTree *tree,
    const std::map<LoopStructure *, uint64_t> &timeSaved,
    const std::map<LoopStructure *, bool> &doallLoops) {

  /*
   * Fetch the maximum time saved by any combination of loops nested from @tree
   * (including @tree).
   */
  auto considerAll = [](LoopStructure *l) -> bool { return true; };
  auto rootLoopMaxTimeSaved =
      this->evaluateSavings(noelle, tree, timeSaved, considerAll);

  /*
   * Fetch the maximum time saved by any combination of loops nested from @tree
   * (including @tree) when we only have DOALL technique.
   */
  auto considerDOALLOnly = [&doallLoops](LoopStructure *l) -> bool {
    if (doallLoops.at(l)) {
      return true;
    }
    return false;
  };
  auto rootLoopMaxTimeSavedWithDOALL =
      this->evaluateSavings(noelle, tree, timeSaved, considerDOALLOnly);

  return make_pair(rootLoopMaxTimeSaved, rootLoopMaxTimeSavedWithDOALL);
}

uint64_t TimeSaved::evaluateSavings(
    Noelle &noelle,
    noelle::LoopTree *tree,
    const std::map<LoopStructure *, uint64_t> &timeSaved,
    std::function<bool(LoopStructure *)> considerLoop) {

  /*
   * Find the maximum savings per internal node up to the root of the tree.
   */
  std::map<LoopStructure *, uint64_t> maxTimeSaved;
  auto f = [&maxTimeSaved,
            &timeSaved,
            &considerLoop](LoopTree *n, uint32_t treeLevel) -> bool {
    /*
     * Check if we should consider this loop.
     */
    auto ls = n->getLoop();
    assert(ls != nullptr);
    if (!considerLoop(ls)) {
      return false;
    }

    /*
     * Fetch the time spent by parallelizing the current loop only.
     */
    assert(timeSaved.find(ls) != timeSaved.end());
    auto loopSaved = timeSaved.at(ls);

    /*
     * Fetch the maximum time saved combining all children.
     */
    uint64_t maxChildrenSaved = 0;
    for (auto child : n->getChildren()) {

      /*
       * Check if we should consider this loop.
       */
      auto nestedLoop = child->getLoop();
      if (!considerLoop(nestedLoop)) {
        continue;
      }

      /*
       * Fetch the maximum time saved for the nested loop (or any combination of
       * its descendants).
       */
      auto nestedLoopMaxSave = maxTimeSaved[nestedLoop];

      /*
       * Accumulate
       */
      maxChildrenSaved += nestedLoopMaxSave;
    }

    /*
     * Compute the maximum time saved by any combination of loops chosen.
     */
    maxTimeSaved[ls] = std::max(maxChildrenSaved, loopSaved);

    return false;
  };
  tree->visitPostOrder(f);

  /*
   * Fetch the maximum time saved by any combination of loops nested from @tree
   * (including @tree).
   */
  auto rootLoop = tree->getLoop();
  auto rootLoopMaxTimeSaved = maxTimeSaved[rootLoop];

  return rootLoopMaxTimeSaved;
}

} // namespace arcana::noelle
