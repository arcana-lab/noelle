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
#include "TimingModel.hpp"

namespace arcana::noelle {

LoopTimingModel::LoopTimingModel(Noelle &noelle, LoopContent &ldi)
  : n{ noelle },
    loop{ ldi } {
  return;
}

uint64_t LoopTimingModel::getTimeSavedByParallelizingLoop(void) {

  /*
   * Fetch the critical path per iteration
   */
  auto seq = this->getTimeSpentInCriticalPathPerIteration();

  /*
   * Fetch the loop structure
   */
  auto ls = this->loop.getLoopStructure();

  /*
   * Fetch the profiles
   */
  auto profiles = this->n.getProfiles();

  /*
   * Compute the total time saved.
   */
  auto instsPerIteration =
      profiles->getAverageTotalInstructionsPerIteration(ls);
  auto instsInBiggestSCCPerIteration =
      ((double)seq) / ((double)profiles->getIterations(ls));
  assert(instsInBiggestSCCPerIteration <= instsPerIteration);
  auto timeSavedPerIteration =
      (double)(instsPerIteration - instsInBiggestSCCPerIteration);
  auto timeSaved = timeSavedPerIteration * profiles->getIterations(ls);

  return timeSaved;
}

uint64_t LoopTimingModel::getTimeSpentInCriticalPathPerIteration(void) {

  /*
   * Fetch the set of sequential SCCs.
   */
  auto sequentialSCCs =
      DOALL::getSCCsThatBlockDOALLToBeApplicable(&this->loop, this->n);

  /*
   * Fetch the profiles
   */
  auto profiles = this->n.getProfiles();
  if (!profiles->isAvailable()) {
    errs() << "LoopTimingModel: WARNING: profiles were not available\n";
    return 0;
  }

  /*
   * Find the biggest sequential SCC.
   */
  uint64_t biggestSCCTime = 0;
  for (auto sequentialSCC : sequentialSCCs) {

    /*
     * Fetch the time spent in the current SCC.
     */
    auto sequentialSCCTime = profiles->getTotalInstructions(sequentialSCC);

    /*
     * Compute the biggest SCC.
     */
    if (sequentialSCCTime > biggestSCCTime) {
      biggestSCCTime = sequentialSCCTime;
    }
  }

  return biggestSCCTime;
}

} // namespace arcana::noelle
