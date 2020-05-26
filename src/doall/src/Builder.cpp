/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "DOALL.hpp"
#include "DOALLTask.hpp"

void DOALL::rewireLoopToIterateChunks (
  LoopDependenceInfo *LDI
){

  /*
   * Fetch the task.
   */
  auto task = (DOALLTask *)tasks[0];

  /*
   * Fetch the header.
   */
  auto loopSummary = LDI->getLoopSummary();
  auto loopHeader = loopSummary->getHeader();
  auto loopPreHeader = loopSummary->getPreHeader();
  auto preheaderClone = task->basicBlockClones.at(loopPreHeader);
  auto headerClone = task->basicBlockClones.at(loopHeader);

  /*
   * Fetch IV information
   */
  auto ivAttribution = LDI->getLoopGoverningIVAttribution();
  auto &iv = ivAttribution->getInductionVariable();
  auto ivType = iv.getHeaderPHI()->getType();
  auto chunkCounterType = task->chunkSizeArg->getType();
  auto startOfIV = fetchClone(iv.getStartAtHeader());
  auto stepOfIV = fetchClone(iv.getStepSize());
  auto exitConditionValue = fetchClone(ivAttribution->getHeaderCmpInstConditionValue());
  LoopGoverningIVUtility ivUtility(iv, *ivAttribution);

  /*
   * Determine start value for outer loop IV
   * core_start: original_start + original_step_size * core_id * chunk_size
   */
  IRBuilder<> entryBuilder(task->entryBlock);
  auto temporaryBrToLoop = entryBuilder.CreateBr(headerClone);
  entryBuilder.SetInsertPoint(temporaryBrToLoop);
  auto nthCoreOffset = entryBuilder.CreateMul(
    stepOfIV,
    entryBuilder.CreateZExtOrTrunc(
      entryBuilder.CreateMul(task->coreArg, task->chunkSizeArg, "coreIdx_X_chunkSize"),
      ivType
    ),
    "stepSize_X_coreIdx_X_chunkSize"
  );
  auto offsetStartValue = entryBuilder.CreateAdd(startOfIV, nthCoreOffset, "startPlusOffset");

  /*
   * Determine additional step size from the beginning of the next core's chunk
   * to the start of this core's next chunk
   * chunk_step_size: original_step_size * (num_cores - 1) * chunk_size
   */
  auto onesValueForChunking = ConstantInt::get(chunkCounterType, 1);
  auto chunkStepSize = entryBuilder.CreateMul(
    stepOfIV,
    entryBuilder.CreateZExtOrTrunc(
      entryBuilder.CreateMul(
        entryBuilder.CreateSub(task->numCoresArg, onesValueForChunking, "numCoresMinus1"),
        task->chunkSizeArg,
        "numCoresMinus1_X_chunkSize"
      ),
      ivType
    ),
    "stepSizeToNextChunk"
  );
  auto oppositeStepOfIV = entryBuilder.CreateMul(stepOfIV, ConstantInt::get(ivType, -1), "negatedStepSize");

  /*
   * Generate PHI to track progress on the current chunk
   * Update IV PHI latch value to increment to the next chunk if the current chunk is finished
   * If incrementing to next chunk, check if previous iteration IV value passes header condition
   */
  auto chunkPHI = ivUtility.createChunkPHI(preheaderClone, headerClone, chunkCounterType, task->chunkSizeArg);
  auto loopGoverningIVPHI = cast<PHINode>(fetchClone(iv.getHeaderPHI()));
  loopGoverningIVPHI->setIncomingValueForBlock(preheaderClone, offsetStartValue);
  ivUtility.chunkLoopGoverningPHI(preheaderClone, loopGoverningIVPHI, chunkPHI, chunkStepSize);

  /*
   * The exit condition needs to be made non-strict to catch iterating past it
   */
  ivUtility.updateConditionAndBranchToCatchIteratingPastExitValue(
    cast<CmpInst>(task->instructionClones.at(ivAttribution->getHeaderCmpInst())),
    cast<BranchInst>(task->instructionClones.at(ivAttribution->getHeaderBrInst())),
    task->loopExitBlocks[0]
  );

  /*
   * The exit condition value does not need to be computed each iteration
   * and so the value's derivation can be hoisted into the preheader
   */
  if (auto exitConditionInst = dyn_cast<Instruction>(exitConditionValue)) {
    auto &derivation = ivUtility.getConditionValueDerivation();
    for (auto I : derivation) {
      I->removeFromParent();
      entryBuilder.Insert(I);
    }

    exitConditionInst->removeFromParent();
    entryBuilder.Insert(exitConditionInst);
  }

  /*
   * NOTE: When loop governing IV attribution allows for any bther instructions in the header
   * other than those of the IV and its comparison, those unrelated instructions need to be
   * copied into the body and the exit block (to preserve the number of times they execute)
   * 
   * The logic in the exit block must be guarded so only the "last" iteration executes it,
   * not any cores that pass the last iteration 
   */

}