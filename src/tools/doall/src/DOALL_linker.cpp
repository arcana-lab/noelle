/*
 * Copyright 2016 - 2023  Simone Campanoni
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
#include "noelle/core/ReductionSCC.hpp"
#include "noelle/core/InductionVariableSCC.hpp"
#include "noelle/tools/DOALL.hpp"
#include "noelle/tools/DOALLTask.hpp"

namespace arcana::noelle {

void DOALL::invokeParallelizedLoop(LoopContent *LDI) {

  /*
   * Create the environment.
   */
  this->allocateEnvironmentArray(LDI);
  this->populateLiveInEnvironment(LDI);

  /*
   * Fetch the pointer to the environment.
   */
  auto envPtr = envBuilder->getEnvironmentArrayVoidPtr();

  /*
   * Fetch the number of cores
   */
  auto ltm = LDI->getLoopTransformationsManager();
  auto cm = this->n.getConstantsManager();
  auto numCores = cm->getIntegerConstant(ltm->getMaximumNumberOfCores(), 64);

  /*
   * Fetch the chunk size.
   */
  auto chunkSize = cm->getIntegerConstant(ltm->getChunkSize(), 64);

  /*
   * Call the dispatcher that will dispatch the tasks that execute the
   * parallelized loop.
   */
  IRBuilder<> doallBuilder(this->entryPointOfParallelizedLoop);
  auto doallCallInst = doallBuilder.CreateCall(
      this->taskDispatcher,
      ArrayRef<Value *>(
          { tasks[0]->getTaskBody(), envPtr, numCores, chunkSize }));

  /*
   * Get the return value of the dispatcher, which has the information about how
   * many threads have been spawn.
   */
  auto numThreadsUsed =
      doallBuilder.CreateExtractValue(doallCallInst, (uint64_t)0);

  /*
   * Propagate the live-out variables computed within tasks to the code outside
   * the parallelized loop.
   */
  auto latestBBAfterDOALLCall =
      this->performReductionToAllReducableLiveOutVariables(LDI, numThreadsUsed);

  /*
   * Jump to the unique successor of the loop.
   * This loop can only have one successor because it is a DOALL.
   */
  IRBuilder<> afterDOALLBuilder{ latestBBAfterDOALLCall };
  afterDOALLBuilder.CreateBr(this->exitPointOfParallelizedLoop);
}

} // namespace arcana::noelle
