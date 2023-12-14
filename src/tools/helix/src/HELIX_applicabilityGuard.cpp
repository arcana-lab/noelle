/*
 * Copyright 2016 - 2023   Simone Campanoni
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
#include "noelle/core/Architecture.hpp"
#include "noelle/core/ReductionSCC.hpp"
#include "noelle/tools/HELIX.hpp"
#include "noelle/tools/DOALL.hpp"

namespace arcana::noelle {

bool HELIX::canBeAppliedToLoop(LoopContent *LDI, Heuristics *h) const {

  /*
   * Check the parent class.
   */
  if (!ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::
          canBeAppliedToLoop(LDI, h)) {
    return false;
  }

  /*
   * Check if we are forced to parallelize
   */
  if (this->forceParallelization) {

    /*
     * HELIX is applicable.
     */
    return true;
  }

  /*
   * Ensure there is not too little execution that is too proportionally
   * sequential for HELIX
   */
  auto profiles = this->noelle.getProfiles();
  auto loopStructure = LDI->getLoopStructure();
  auto loopIDOpt = loopStructure->getID();
  assert(loopIDOpt);
  auto loopID = loopIDOpt.value();
  auto averageInstructions =
      profiles->getAverageTotalInstructionsPerIteration(loopStructure);
  auto averageInstructionThreshold = 20;
  auto hasLittleExecution = averageInstructions < averageInstructionThreshold;
  auto maximumSequentialFraction = .2;
  auto sequentialFraction = this->computeSequentialFractionOfExecution(LDI);
  auto hasProportionallySignificantSequentialExecution =
      sequentialFraction >= maximumSequentialFraction;
  if (hasLittleExecution && hasProportionallySignificantSequentialExecution) {
    errs()
        << "Parallelizer:    Loop " << loopID << " has " << averageInstructions
        << " number of sequential instructions on average per loop iteration\n";
    errs()
        << "Parallelizer:    Loop " << loopID << " has " << sequentialFraction
        << " % sequential execution per loop iteration\n";
    errs()
        << "Parallelizer:      It will be too heavily synchronized for HELIX. The thresholds are at least "
        << averageInstructionThreshold
        << " instructions per iteration or less than "
        << maximumSequentialFraction << " % sequential execution."
        << "\n";

    return false;
  }

  return true;
}

} // namespace arcana::noelle
