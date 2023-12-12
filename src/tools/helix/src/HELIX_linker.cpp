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
#include "noelle/tools/HELIX.hpp"

namespace arcana::noelle {

void HELIX::invokeParallelizedLoop(LoopContent *LDI,
                                   uint64_t numberOfSequentialSegments) {

  /*
   * Fetch the managers.
   */
  auto cm = this->noelle.getConstantsManager();

  /*
   * Fetch the loop function.
   */
  auto loopSummary = LDI->getLoopStructure();
  auto loopFunction = loopSummary->getFunction();

  /*
   * Create the environment.
   * This will append store instructions to entryPointOfParallelizedLoop to
   * initialize the environment array.
   */
  this->allocateEnvironmentArray(LDI);
  this->populateLiveInEnvironment(LDI);

  /*
   * Fetch the pointer to the environments
   */
  auto envPtr = envBuilder->getEnvironmentArrayVoidPtr();
  auto loopCarriedEnvPtr =
      this->loopCarriedLoopEnvironmentBuilder->getEnvironmentArrayVoidPtr();

  /*
   * Fetch the number of cores
   */
  auto ltm = LDI->getLoopTransformationsManager();
  auto numCores = cm->getIntegerConstant(ltm->getMaximumNumberOfCores(), 64);

  /*
   * Fetch the chunk size.
   */
  auto numOfSS = cm->getIntegerConstant(numberOfSequentialSegments, 64);

  /*
   * Call the function that incudes the parallelized loop.
   */
  IRBuilder<> helixBuilder(this->entryPointOfParallelizedLoop);
  auto runtimeCall = helixBuilder.CreateCall(
      this->taskDispatcherSS,
      ArrayRef<Value *>({ (Value *)tasks[0]->getTaskBody(),
                          envPtr,
                          loopCarriedEnvPtr,
                          numCores,
                          numOfSS }));
  auto numThreadsUsed =
      helixBuilder.CreateExtractValue(runtimeCall, (uint64_t)0);

  /*
   * Propagate the last value of live-out variables to the code outside the
   * parallelized loop.
   */
  auto latestBBAfterCall =
      this->performReductionToAllReducableLiveOutVariables(LDI, numThreadsUsed);

  IRBuilder<> afterCallBuilder{ latestBBAfterCall };
  afterCallBuilder.CreateBr(this->exitPointOfParallelizedLoop);
}

} // namespace arcana::noelle
