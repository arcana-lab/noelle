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
   * Fetch loop and IV information.
   */
  auto loopSummary = LDI->getLoopSummary();
  auto loopHeader = loopSummary->getHeader();
  auto loopPreHeader = loopSummary->getPreHeader();
  auto preheaderClone = task->getCloneOfOriginalBasicBlock(loopPreHeader);
  auto headerClone = task->getCloneOfOriginalBasicBlock(loopHeader);
  auto allIVInfo = LDI->getInductionVariables();

  /*
   * Hook up preheader to header to enable induction variable manipulation
   */
  IRBuilder<> entryBuilder(task->getEntry());
  auto temporaryBrToLoop = entryBuilder.CreateBr(headerClone);
  entryBuilder.SetInsertPoint(temporaryBrToLoop);

  /*
   * Generate PHI to track progress on the current chunk
   */
  auto chunkCounterType = task->chunkSizeArg->getType();
  auto chunkPHI = IVUtility::createChunkPHI(preheaderClone, headerClone, chunkCounterType, task->chunkSizeArg);

  /*
   * Collect clones of step size deriving values
   */
  std::unordered_map<InductionVariable *, Value *> clonedStepSizeMap;
  for (auto ivInfo : allIVInfo->getInductionVariables(*loopSummary)) {
    Value *clonedStepValue = nullptr;
    if (ivInfo->getSimpleValueOfStepSize()) {
      clonedStepValue = fetchClone(ivInfo->getSimpleValueOfStepSize());
    } else {
      auto expandedInsts = ivInfo->getExpansionOfCompositeStepSize();
      assert(expandedInsts.size() > 0);
      for (auto expandedInst : expandedInsts) {
        auto clonedInst = expandedInst->clone();
        task->instructionClones.insert(std::make_pair(expandedInst, clonedInst));
        entryBuilder.Insert(clonedInst);
      }

      for (auto expandedInst : expandedInsts) {
        adjustDataFlowToUseClones(task->instructionClones.at(expandedInst), 0);
      }
      clonedStepValue = task->instructionClones.at(expandedInsts.back());
    }

    clonedStepSizeMap.insert(std::make_pair(ivInfo, clonedStepValue));
  }

  /*
   * Determine start value of the IV for the task
   * core_start: original_start + original_step_size * core_id * chunk_size
   */
  for (auto ivInfo : allIVInfo->getInductionVariables(*loopSummary)) {
    auto startOfIV = fetchClone(ivInfo->getStartAtHeader());
    auto stepOfIV = clonedStepSizeMap.at(ivInfo);
    auto ivPHI = cast<PHINode>(fetchClone(ivInfo->getHeaderPHI()));

    auto nthCoreOffset = entryBuilder.CreateMul(
      stepOfIV,
      entryBuilder.CreateZExtOrTrunc(
        entryBuilder.CreateMul(task->coreArg, task->chunkSizeArg, "coreIdx_X_chunkSize"),
        ivPHI->getType()
      ),
      "stepSize_X_coreIdx_X_chunkSize"
    );
    auto offsetStartValue = entryBuilder.CreateAdd(startOfIV, nthCoreOffset, "startPlusOffset");

    ivPHI->setIncomingValueForBlock(preheaderClone, offsetStartValue);
  }

  /*
   * Determine additional step size from the beginning of the next core's chunk
   * to the start of this core's next chunk
   * chunk_step_size: original_step_size * (num_cores - 1) * chunk_size
   */
  for (auto ivInfo : allIVInfo->getInductionVariables(*loopSummary)) {
    auto stepOfIV = clonedStepSizeMap.at(ivInfo);
    auto ivPHI = cast<PHINode>(fetchClone(ivInfo->getHeaderPHI()));

    auto onesValueForChunking = ConstantInt::get(chunkCounterType, 1);
    auto chunkStepSize = entryBuilder.CreateMul(
      stepOfIV,
      entryBuilder.CreateZExtOrTrunc(
        entryBuilder.CreateMul(
          entryBuilder.CreateSub(task->numCoresArg, onesValueForChunking, "numCoresMinus1"),
          task->chunkSizeArg,
          "numCoresMinus1_X_chunkSize"
        ),
        ivPHI->getType()
      ),
      "stepSizeToNextChunk"
    );

    IVUtility::chunkInductionVariablePHI(preheaderClone, ivPHI, chunkPHI, chunkStepSize);
  }

  /*
   * The exit condition needs to be made non-strict to catch iterating past it
   */
  auto loopGoverningIVAttr = LDI->getLoopGoverningIVAttribution();
  LoopGoverningIVUtility ivUtility(loopGoverningIVAttr->getInductionVariable(), *loopGoverningIVAttr);
  ivUtility.updateConditionAndBranchToCatchIteratingPastExitValue(
    cast<CmpInst>(task->getCloneOfOriginalInstruction(loopGoverningIVAttr->getHeaderCmpInst())),
    cast<BranchInst>(task->getCloneOfOriginalInstruction(loopGoverningIVAttr->getHeaderBrInst())),
    task->getLastBlock(0)
  );

  /*
   * The exit condition value does not need to be computed each iteration
   * and so the value's derivation can be hoisted into the preheader
   */
  auto exitConditionValue = fetchClone(loopGoverningIVAttr->getHeaderCmpInstConditionValue());
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
