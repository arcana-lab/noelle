/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
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
#include "Parallelizer.hpp"

namespace arcana::noelle {

bool Parallelizer::parallelizeLoop(LoopContent *loopContent,
                                   Noelle &par,
                                   Heuristics *h) {
  auto prefix = "Parallelizer: parallelizerLoop: ";

  /*
   * Assertions.
   */
  assert(loopContent != nullptr);
  assert(h != nullptr);

  /*
   * Allocate the parallelization techniques.
   */
  DSWP dswp{ par, this->forceParallelization, !this->forceNoSCCPartition };
  DOALL doall{ par };
  HELIX helix{ par, this->forceParallelization };
  std::vector<ParallelizationTechnique *> parallelizationTechniques{ &doall,
                                                                     &helix,
                                                                     &dswp };

  /*
   * Fetch the profiles.
   */
  auto profiles = par.getProfiles();

  /*
   * Fetch the managers.
   */
  auto cm = par.getConstantsManager();

  /*
   * Fetch the verbosity level.
   */
  auto verbose = par.getVerbosity();

  /*
   * Fetch the loop headers.
   */
  auto loopStructure = loopContent->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();
  auto loopPreHeader = loopStructure->getPreHeader();

  /*
   * Fetch the loop function.
   */
  auto loopFunction = loopStructure->getFunction();
  assert(par.verifyCode());

  /*
   * Print
   */
  if (verbose != Verbosity::Disabled) {

    /*
     * Get loop ID.
     */
    auto loopIDOpt = loopStructure->getID();
    assert(loopIDOpt);
    auto loopID = loopIDOpt.value();

    /*
     * Print the most important loop information.
     */
    errs() << prefix << "Start\n";
    errs() << prefix << "  Function = \"" << loopFunction->getName() << "\"\n";
    errs() << prefix << "  Loop " << loopID << " = \""
           << *loopHeader->getFirstNonPHI() << "\"\n";
    errs() << prefix << "  Nesting level = " << loopStructure->getNestingLevel()
           << "\n";
    errs() << prefix << "  Number of threads to extract = "
           << loopContent->getLoopTransformationsManager()
                  ->getMaximumNumberOfCores()
           << "\n";
    if (profiles->isAvailable()) {
      errs() << prefix << "  Coverage = "
             << (profiles->getDynamicTotalInstructionCoverage(loopStructure)
                 * 100.0)
             << "%\n";
    }

    /*
     * Print the loop environment.
     */
    errs() << prefix << "  Environment: live-in and live-out values\n";
    auto env = loopContent->getEnvironment();
    for (auto envID : env->getEnvIDsOfLiveInVars()) {
      auto producerOfLiveIn = env->getProducer(envID);
      errs() << prefix << "  Environment:   Live-in " << envID << " = "
             << *producerOfLiveIn << "\n";
    }
    for (auto envID : env->getEnvIDsOfLiveOutVars()) {
      auto producer = env->getProducer(envID);
      errs() << prefix << "  Environment:   Live-out " << envID << " = "
             << *producer << "\n";
    }
  }

  /*
   * Parallelize the loop.
   */
  auto codeModified = false;
  auto ltm = loopContent->getLoopTransformationsManager();
  ParallelizationTechnique *usedTechnique = nullptr;
  for (auto parallelizationTechnique : parallelizationTechniques) {

    /*
     * Fetch the information about the current parallelization technique.
     */
    auto parID = parallelizationTechnique->getParallelizationID();

    /*
     * Check if the current parallelization technique is applicable to the
     * current loop.
     */
    if (par.isTransformationEnabled(parID)
        && ltm->isTransformationEnabled(parID)
        && parallelizationTechnique->canBeAppliedToLoop(loopContent, h)) {

      /*
       * Parallelize the current loop with the current parallelization
       * technique.
       */
      codeModified = parallelizationTechnique->apply(loopContent, h);
      usedTechnique = parallelizationTechnique;
      break;
    }
  }

  /*
   * Check if the loop has been parallelized.
   */
  if (!codeModified) {
    errs() << prefix << "  The loop has not been parallelized\n";
    errs() << prefix << "Exit\n";
    return false;
  }

  /*
   * Fetch the environment array where the exit block ID has been stored.
   */
  assert(usedTechnique != nullptr);
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
   * Link the parallelized loop within the original function that includes the
   * sequential loop.
   */
  if (verbose != Verbosity::Disabled) {
    errs() << prefix << "  Link the parallelize loop\n";
  }
  auto exitBlockID = loopContent->getEnvironment()->getExitBlockID();
  auto constantValue =
      exitBlockID >= 0
          ? usedTechnique->getIndexOfEnvironmentVariable(exitBlockID)
          : -1;
  auto exitIndex = cm->getIntegerConstant(constantValue, 64);
  auto loopExitBlocks = loopStructure->getLoopExitBasicBlocks();
  auto linker = par.getLinker();
  linker->linkTransformedLoopToOriginalFunction(
      loopPreHeader,
      entryPoint,
      exitPoint,
      envArray,
      exitIndex,
      loopExitBlocks,
      usedTechnique->getMinimumNumberOfIdleCores());
  assert(par.verifyCode());

  // if (verbose >= Verbosity::Maximal) {
  //   loopFunction->print(errs() << "Final printout:\n"); errs() << "\n";
  // }
  if (verbose != Verbosity::Disabled) {
    errs() << prefix << "  The loop has been parallelized with "
           << usedTechnique->getName() << "\n";
    errs() << prefix << "Exit\n";
  }

  return true;
}

} // namespace arcana::noelle
