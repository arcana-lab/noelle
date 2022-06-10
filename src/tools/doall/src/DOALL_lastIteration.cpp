/*
 * Copyright 2021 - 2022  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "DOALL.hpp"
#include "DOALLTask.hpp"

namespace llvm::noelle{

BasicBlock * DOALL::getBasicBlockExecutedOnlyByLastIterationBeforeExitingTask (LoopDependenceInfo *LDI) {
  assert(LDI != nullptr);

  /*
   * Fetch the task.
   */
  auto task = (DOALLTask *)this->tasks[0];
  assert(task != nullptr);
  auto taskFunction = task->getTaskBody();

  /*
   * Fetch the last basic block executed before leaving the task.
   */
  auto lastBB = task->getExit();
  assert(lastBB != nullptr);

  /*
   * Collect clones of step size deriving values for all induction variables
   * of the top level loop
   */
  IRBuilder<> entryBuilder(task->getEntry());
  auto jumpToLoop = task->getEntry()->getTerminator();
  entryBuilder.SetInsertPoint(jumpToLoop);
  auto clonedStepSizeMap = this->cloneIVStepValueComputation(LDI, 0, entryBuilder);

  /*
   * Create a new basic block that will be executed after leaving the loop and only if the current task has executed the last iteration of the loop.
   */
  auto loopStructure = LDI->getLoopStructure();
  auto newBB = BasicBlock::Create(taskFunction->getContext(), "code_executed_only_by_the_last_loop_iteration", taskFunction);
  IRBuilder<> newBBBuilder(newBB);

  /*
   * Split the last basic block to inject the condition to jump to the new basic block
   */
  assert(lastBB->size() > 0);
  auto splitPoint = lastBB->getTerminator();
  auto newLastBB = lastBB->splitBasicBlock(splitPoint, "very_last_bb_before_exiting_task");
  assert(newLastBB != nullptr);
  newBBBuilder.CreateBr(newLastBB);
  auto newTerm = lastBB->getTerminator();
  newTerm->eraseFromParent();
  IRBuilder<> lastBBBuilder(lastBB);

  /*
   * Generate the code to identify whether we have executed the last loop iteration.
   *
   * Step 0: create the IV utility for the loop governing IV.
   */
  auto allIVInfo = LDI->getInductionVariableManager();
  auto loopGoverningIVAttr = LDI->getLoopGoverningIVAttribution();
  LoopGoverningIVUtility ivUtility(loopStructure, *allIVInfo, *loopGoverningIVAttr);

  /*
   * Step 1: Compute the value that the loop governing IV had at the iteration before.
   */
  auto &loopGoverningIV = loopGoverningIVAttr->getInductionVariable();
  auto loopGoverningPHI = task->getCloneOfOriginalInstruction(loopGoverningIV.getLoopEntryPHI());
  auto stepSize = clonedStepSizeMap.at(&loopGoverningIV);
  auto prevIterationValue = ivUtility.generateCodeToComputePreviousValueUsedToCompareAgainstExitConditionValue(lastBBBuilder, loopGoverningPHI, stepSize);

  /*
   * Step 2: Add the conditional branch to jump to the new basic block
   *         To this end, compare the previous-iteration IV value against the exit condition
   */
  auto originalCmpInst = loopGoverningIVAttr->getHeaderCompareInstructionToComputeExitCondition();
  auto cmpInst = cast<CmpInst>(task->getCloneOfOriginalInstruction(originalCmpInst));
  auto clonedCmpInst = cmpInst->clone();
  auto origValueUsedToCompareAgainstExitConditionValue = loopGoverningIVAttr->getValueToCompareAgainstExitConditionValue();
  auto valueUsedToCompareAgainstExitConditionValue = task->getCloneOfOriginalInstruction(origValueUsedToCompareAgainstExitConditionValue);
  clonedCmpInst->replaceUsesOfWith(valueUsedToCompareAgainstExitConditionValue, prevIterationValue);
  lastBBBuilder.Insert(clonedCmpInst);
  lastBBBuilder.CreateCondBr(clonedCmpInst, newLastBB, newBB);

  return newBB;
}

}
