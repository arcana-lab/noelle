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
#include "noelle/tools/DOALL.hpp"
#include "noelle/tools/DOALLTask.hpp"

namespace llvm::noelle {

BasicBlock *DOALL::getBasicBlockExecutedOnlyByLastIterationBeforeExitingTask(
    LoopDependenceInfo *LDI,
    uint32_t taskIndex,
    BasicBlock &bb) {
  assert(LDI != nullptr);
  assert(taskIndex == 0);
  assert(this->tasks.size() > 0);

  /*
   * Fetch the task.
   */
  auto task = (DOALLTask *)this->tasks[taskIndex];
  assert(task != nullptr);
  auto taskFunction = task->getTaskBody();

  /*
   * Collect clones of step size deriving values for all induction variables
   * of the top level loop
   */
  IRBuilder<> entryBuilder(task->getEntry());
  auto jumpToLoop = task->getEntry()->getTerminator();
  entryBuilder.SetInsertPoint(jumpToLoop);
  auto clonedStepSizeMap =
      this->cloneIVStepValueComputation(LDI, 0, entryBuilder);

  /*
   * Split the last basic block to inject the condition to jump to the new basic
   * block.
   * The new basic block will execute after leaving the loop and
   * only if the current task has executed the last iteration of the loop.
   */
  assert(bb.size() > 0);
  auto splitPoint = bb.getTerminator();
  auto addConditionalBranch = [this, &bb, LDI, task, &clonedStepSizeMap](
                                  BasicBlock *newBB,
                                  BasicBlock *newJoinBB) {
    IRBuilder<> lastBBBuilder(&bb);

    /*
     * Generate the code to identify whether we have executed the last loop
     * iteration.
     *
     * Step 0: create the IV utility for the loop governing IV.
     */
    auto loopStructure = LDI->getLoopStructure();
    auto allIVInfo = LDI->getInductionVariableManager();
    auto loopGoverningIVAttr = LDI->getLoopGoverningIVAttribution();
    LoopGoverningIVUtility ivUtility(loopStructure,
                                     *allIVInfo,
                                     *loopGoverningIVAttr);

    /*
     * Step 1: find the value of the loop governing IV that was updated to
     * (potentially) skip to the next chunk.
     */
    auto &loopGoverningIV = loopGoverningIVAttr->getInductionVariable();
    auto originalLoopGoverningPHI = cast<PHINode>(
        task->getCloneOfOriginalInstruction(loopGoverningIV.getLoopEntryPHI()));
    auto &setOfLoopGoverningLastValues =
        this->IVValueJustBeforeEnteringBody.at(originalLoopGoverningPHI);
    assert(setOfLoopGoverningLastValues.size() > 0);
    auto valueOfLoopGoverningIVAfterConsideringChunking =
        *setOfLoopGoverningLastValues.begin();
    assert(valueOfLoopGoverningIVAfterConsideringChunking != nullptr);

    /*
     * Step 2: find the value of the loop governing IV that was used to do the
     * last loop-condition check (whether to run the next iteration or not).
     */
    auto loopGoverningIVLastValue = cast<Instruction>(this->fetchClone(
        loopGoverningIVAttr->getValueToCompareAgainstExitConditionValue()));
    assert(loopGoverningIVLastValue != nullptr);
    auto loopGoverningIVLastValueBB = loopGoverningIVLastValue->getParent();

    /*
     * Step 3: add the PHI to merge the loop governing IV last value with its
     * value when the loop was exited directly side-stepping the basic block
     * that skip to the next chunk.
     *
     * The loop governing IV value
     * @valueOfLoopGoverningIVAfterConsideringChunking is computed in the basic
     * block where all IVs are fast-forwarded to their value at the beginning of
     * the next chunk. This basic block is side-stepped if the last iteration
     * left the loop. This is why we need to introduce a new PHI.
     */
    auto firstNotPHI = bb.getFirstNonPHI();
    if (firstNotPHI != nullptr) {
      lastBBBuilder.SetInsertPoint(firstNotPHI);
    }
    auto valueOfLoopGoverningIVAfterConsideringChunkingBB =
        valueOfLoopGoverningIVAfterConsideringChunking->getParent();
    auto loopGoverningExitPHI = lastBBBuilder.CreatePHI(
        valueOfLoopGoverningIVAfterConsideringChunking->getType(),
        2);
    auto foundBB = false;
    for (auto predBB : predecessors(&bb)) {
      if (predBB == valueOfLoopGoverningIVAfterConsideringChunkingBB) {
        foundBB = true;
        break;
      }
    }
    if (foundBB) {
      loopGoverningExitPHI->addIncoming(
          valueOfLoopGoverningIVAfterConsideringChunking,
          valueOfLoopGoverningIVAfterConsideringChunkingBB);
    }
    for (auto predBB : predecessors(&bb)) {
      if (predBB == loopGoverningIVLastValueBB) {
        loopGoverningExitPHI->addIncoming(loopGoverningIVLastValue,
                                          loopGoverningIVLastValueBB);
        break;
      }
    }
    lastBBBuilder.SetInsertPoint(&bb);

    /*
     * Step 4: Compute the value that the loop governing IV had when the task
     * left the loop.
     */
    auto stepSize = clonedStepSizeMap.at(&loopGoverningIV);
    auto prevIterationValue =
        ivUtility
            .generateCodeToComputePreviousValueUsedToCompareAgainstExitConditionValue(
                lastBBBuilder,
                loopGoverningExitPHI,
                stepSize);

    /*
     * Step 5: Add the conditional branch to jump to the new basic block.
     *         To this end, compare the previous-iteration IV value against the
     * exit condition.
     */
    auto originalCmpInst =
        loopGoverningIVAttr
            ->getHeaderCompareInstructionToComputeExitCondition();
    auto cmpInst =
        cast<CmpInst>(task->getCloneOfOriginalInstruction(originalCmpInst));
    auto clonedCmpInst = cmpInst->clone();
    auto origValueUsedToCompareAgainstExitConditionValue =
        loopGoverningIVAttr->getValueToCompareAgainstExitConditionValue();
    auto valueUsedToCompareAgainstExitConditionValue =
        task->getCloneOfOriginalInstruction(
            origValueUsedToCompareAgainstExitConditionValue);
    clonedCmpInst->replaceUsesOfWith(
        valueUsedToCompareAgainstExitConditionValue,
        prevIterationValue);
    lastBBBuilder.Insert(clonedCmpInst);
    lastBBBuilder.CreateCondBr(clonedCmpInst, newJoinBB, newBB);

    /*
     * Step 6: update the condition to check if the last loop iteration (last of
     * the sequential original loop) was executed by the current task.
     */
    ivUtility.updateConditionToCheckIfTheLastLoopIterationWasExecuted(
        cast<CmpInst>(clonedCmpInst));

    return;
  };
  auto cfgTransformer = this->noelle.getCFGTransformer();
  auto newBB = cfgTransformer.branchToANewBasicBlockAndBack(
      splitPoint,
      "code_executed_only_by_the_last_loop_iteration",
      "very_last_bb_before_exiting_task",
      addConditionalBranch);

  return newBB;
}

} // namespace llvm::noelle
