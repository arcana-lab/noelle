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
#include "noelle/tools/HELIX.hpp"

namespace arcana::noelle {

BasicBlock *HELIX::getBasicBlockExecutedOnlyByLastIterationBeforeExitingTask(
    LoopContent *LDI,
    uint32_t taskIndex,
    BasicBlock &bb) {
  assert(LDI != nullptr);
  assert(taskIndex == 0);
  assert(this->tasks.size() > 0);

  /*
   * Fetch the task.
   */
  auto task = (HELIXTask *)this->tasks[taskIndex];
  assert(task != nullptr);
  auto taskFunction = task->getTaskBody();

  /*
   * Check if we have a sequential prologue
   */
  auto allIVInfo = LDI->getInductionVariableManager();
  auto loopGoverningIVAttr = allIVInfo->getLoopGoverningInductionVariable();
  if (false || (loopGoverningIVAttr == nullptr)
      || (this->doesHaveASequentialPrologue(LDI))) {
    return &bb;
  }

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
   * Create a new basic block that will be executed after leaving the loop and
   * only if the current task has executed the last iteration of the loop.
   */
  auto cfgTransformer = this->noelle.getCFGTransformer();
  assert(bb.size() > 0);
  auto splitPoint = bb.getTerminator();
  auto addConditionalBranch = [&bb, LDI, task, &clonedStepSizeMap, allIVInfo](
                                  BasicBlock *newBB,
                                  BasicBlock *newJoinBB) {
    IRBuilder<> lastBBBuilder(&bb);

    /*
     * Fetch the information about the loop.
     */
    auto loopStructure = LDI->getLoopStructure();
    auto loopGoverningIVAttr = allIVInfo->getLoopGoverningInductionVariable();
    assert(loopGoverningIVAttr != nullptr);

    /*
     * Generate the code to identify whether we have executed the last loop
     * iteration.
     *
     * Step 0: create the IV utility for the loop governing IV.
     */
    LoopGoverningIVUtility ivUtility(loopStructure,
                                     *allIVInfo,
                                     *loopGoverningIVAttr);

    /*
     * Step 1: Compute the value that the loop governing IV had at the iteration
     * before.
     */
    auto loopGoverningIV = loopGoverningIVAttr->getInductionVariable();
    auto loopGoverningPHI =
        task->getCloneOfOriginalInstruction(loopGoverningIV->getLoopEntryPHI());
    auto stepSize = clonedStepSizeMap.at(loopGoverningIV);
    auto prevIterationValue =
        ivUtility
            .generateCodeToComputePreviousValueUsedToCompareAgainstExitConditionValue(
                lastBBBuilder,
                loopGoverningPHI,
                stepSize);

    /*
     * Step 2: understand whether the true successor of the header branch jumps
     * into the loop or not.
     */
    auto originalBrInst = loopGoverningIVAttr->getHeaderBrInst();
    auto brInst = task->getCloneOfOriginalInstruction(originalBrInst);
    assert(brInst != nullptr);
    auto trueSucc = brInst->getSuccessor(0);
    bool jumpInLoopCondition;
    if (trueSucc == &bb) {
      jumpInLoopCondition = false;
    } else {
      jumpInLoopCondition = true;
    }

    /*
     * Step 3: Add the conditional branch to jump to the new basic block.
     *
     * To this end, compare the previous-iteration IV value against the exit
     * condition
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
    if (jumpInLoopCondition) {
      lastBBBuilder.CreateCondBr(clonedCmpInst, newJoinBB, newBB);
    } else {
      lastBBBuilder.CreateCondBr(clonedCmpInst, newBB, newJoinBB);
    }

    return;
  };
  auto newBB = cfgTransformer.branchToANewBasicBlockAndBack(
      splitPoint,
      "code_executed_only_by_the_last_loop_iteration",
      "very_last_bb_before_exiting_task",
      addConditionalBranch);

  return newBB;
}

} // namespace arcana::noelle
