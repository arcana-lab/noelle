/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "HELIX.hpp"
#include "HELIXTask.hpp"

using namespace llvm ;

void HELIX::rewireLoopForIVsToIterateNthIterations(LoopDependenceInfo *LDI) {

  /*
   * Fetch loop and IV information.
   */
  auto task = static_cast<HELIXTask *>(this->tasks[0]);
  auto loopStructure = LDI->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();
  auto loopPreHeader = loopStructure->getPreHeader();
  auto preheaderClone = task->getCloneOfOriginalBasicBlock(loopPreHeader);
  auto headerClone = task->getCloneOfOriginalBasicBlock(loopHeader);
  auto ivManager = LDI->getInductionVariableManager();

  auto entryTerminator = task->getEntry()->getTerminator();
  IRBuilder<> entryBuilder(entryTerminator);

  // TODO: Refactor this and DOALL's implementation of it into ParallelizationTechnique
  auto fetchClone = [&](Value *original) -> Value * {
    if (isa<ConstantData>(original)) return original;

    auto liveInClone = task->getCloneOfOriginalLiveIn(original);
    if (liveInClone) return liveInClone;

    assert(isa<Instruction>(original));
    auto originalI = cast<Instruction>(original);
    assert(task->isAnOriginalInstruction(originalI));
    return task->getCloneOfOriginalInstruction(originalI);
  };

  /*
   * There are situations where the SCC containing an IV is not deemed fully clonable,
   * so we spill those IVs. Skip those when re-wiring the step size of IVs
   */
  auto sccdag = LDI->sccdagAttrs.getSCCDAG();
  std::set<InductionVariable *> ivInfos;
  for (auto ivInfo : ivManager->getInductionVariables(*loopStructure)) {
    auto loopEntryPHI = ivInfo->getLoopEntryPHI();

    /*
     * If the instruction was spilled, its clone will not be a PHI, rather a load
     */
    auto clonePHI = task->getCloneOfOriginalInstruction(loopEntryPHI);
    if (!isa<PHINode>(clonePHI)) continue;
    ivInfos.insert(ivInfo);
  }

  /*
   * Collect clones of step size deriving values for all induction variables
   * of the top level loop
   * TODO: Refactor this, as DOALL does the same
   */
  std::unordered_map<InductionVariable *, Value *> clonedStepSizeMap;
  for (auto ivInfo : ivInfos) {
    Value *clonedStepValue = nullptr;
    if (ivInfo->getSingleComputedStepValue()) {
      clonedStepValue = fetchClone(ivInfo->getSingleComputedStepValue());
    } else {

      /*
       * The step size is a composite SCEV. Fetch its instruction expansion,
       * cloning into the entry block of the function
       * 
       * NOTE: The step size is expected to be loop invariant
       */
      auto expandedInsts = ivInfo->getComputationOfStepValue();
      assert(expandedInsts.size() > 0);
      for (auto expandedInst : expandedInsts) {
        auto clonedInst = expandedInst->clone();
        task->addInstruction(expandedInst, clonedInst);
        entryBuilder.Insert(clonedInst);
      }

      /*
       * Wire the instructions in the expansion to use the cloned values
       */
      for (auto expandedInst : expandedInsts) {
        adjustDataFlowToUseClones(task->getCloneOfOriginalInstruction(expandedInst), 0);
      }
      clonedStepValue = task->getCloneOfOriginalInstruction(expandedInsts.back());
    }

    clonedStepSizeMap.insert(std::make_pair(ivInfo, clonedStepValue));
  }

  /*
   * Determine start value of the IV for the task
   * core_start: original_start + original_step_size * core_id
   */
  for (auto ivInfo : ivInfos) {
    auto startOfIV = fetchClone(ivInfo->getStartValue());
    auto stepOfIV = clonedStepSizeMap.at(ivInfo);
    auto originalIVPHI = ivInfo->getLoopEntryPHI();
    auto ivPHI = cast<PHINode>(fetchClone(originalIVPHI));

    auto nthCoreOffset = entryBuilder.CreateMul(
      stepOfIV,
      entryBuilder.CreateZExtOrTrunc(
        task->coreArg,
        ivPHI->getType()
      ),
      "stepSize_X_coreIdx"
    );
    auto offsetStartValue = entryBuilder.CreateAdd(startOfIV, nthCoreOffset, "startPlusOffset");

    ivPHI->setIncomingValueForBlock(preheaderClone, offsetStartValue);
  }

  /*
   * Determine additional step size to account for n cores each executing the task
   * jump_step_size: original_step_size * (num_cores - 1)
   */
  for (auto ivInfo : ivInfos) {
    auto stepOfIV = clonedStepSizeMap.at(ivInfo);
    auto originalIVPHI = ivInfo->getLoopEntryPHI();
    auto ivPHI = cast<PHINode>(fetchClone(originalIVPHI));

    auto jumpStepSize = entryBuilder.CreateMul(
      stepOfIV,
      entryBuilder.CreateSub(
        entryBuilder.CreateZExtOrTrunc(
          task->numCoresArg,
          ivPHI->getType()
        ),
        ConstantInt::get(ivPHI->getType(), 1)
      ),
      "nCoresStepSize"
    );

    for (auto i = 0; i < ivPHI->getNumIncomingValues(); ++i) {
      auto B = ivPHI->getIncomingBlock(i);
      if (preheaderClone == B) continue;

      IRBuilder<> latchBuilder(B->getTerminator());
      auto prevStepRecurrence = ivPHI->getIncomingValue(i);
      auto batchStepRecurrence = latchBuilder.CreateAdd(prevStepRecurrence, jumpStepSize, "nextStep");
      ivPHI->setIncomingValue(i, batchStepRecurrence);
    }
  }

  /*
   * If the loop is governed by an IV, ensure that:
   * 1) the condition catches iterating past the exit value
   * 2) all non-clonable instructions in the header instead execute in the body and after exiting the loop
   */
  auto loopGoverningIVAttr = LDI->getLoopGoverningIVAttribution();
  if (loopGoverningIVAttr) {
    auto &loopGoverningIV = loopGoverningIVAttr->getInductionVariable();
    LoopGoverningIVUtility ivUtility(loopGoverningIV, *loopGoverningIVAttr);

    auto originalCmpInst = loopGoverningIVAttr->getHeaderCmpInst();
    auto originalBrInst = loopGoverningIVAttr->getHeaderBrInst();
    auto cmpInst = cast<CmpInst>(task->getCloneOfOriginalInstruction(originalCmpInst));
    auto brInst = cast<BranchInst>(task->getCloneOfOriginalInstruction(originalBrInst));

    auto originalHeaderExit = loopGoverningIVAttr->getExitBlockFromHeader();
    auto cloneHeaderExit = task->getCloneOfOriginalBasicBlock(originalHeaderExit);

    auto headerSuccTrue = brInst->getSuccessor(0);
    auto headerSuccFalse = brInst->getSuccessor(1);
    auto isTrueExiting = headerSuccTrue == cloneHeaderExit;
    auto entryIntoBody = isTrueExiting ? headerSuccFalse : headerSuccTrue;
    ivUtility.updateConditionAndBranchToCatchIteratingPastExitValue(cmpInst, brInst, cloneHeaderExit);
    auto updatedCmpInst = cmpInst;

    /*
     * Collect instructions that cannot be in the header
     */
    std::vector<Instruction *> instsToMoveAndClone;
    for (auto &I : *loopHeader) {
		  auto scc = sccdag->sccOfValue(&I);
      auto sccInfo = LDI->sccdagAttrs.getSCCAttrs(scc);

      /*
       * Ensure the instruction is sequential and was not a spilled PHI
       * (spilled loads can be left in the header)
       */
      if (sccInfo->canExecuteReducibly()) continue;
      if (sccInfo->canBeCloned()) continue;
      if (isa<PHINode>(&I)) continue;

      auto cloneI = task->getCloneOfOriginalInstruction(&I);
      instsToMoveAndClone.push_back(cloneI);
    }

    /*
     * Move those instructions to the loop body (right at the beginning, in order)
     */
    auto firstBodyInst = entryIntoBody->getFirstNonPHIOrDbgOrLifetime();
    for (auto iIter = instsToMoveAndClone.rbegin(); iIter != instsToMoveAndClone.rend(); ++iIter) {
      auto inst = *iIter;
      inst->moveBefore(firstBodyInst);
      firstBodyInst = inst;
    }

    /*
     * Clone these instructions and execute them after exiting the loop ONLY IF
     * the previous iteration's IV value passes the loop guard.
     * 
     * Any of these that are live out values must replace their equivalent in the loop body
     * within the task's instruction mapping
     */
    auto taskFunction = task->getTaskBody();
    auto &cxt = taskFunction->getContext();
    auto checkForLastExecutionBlock = BasicBlock::Create(cxt, "", taskFunction);
    auto lastHeaderSequentialExecutionBlock = BasicBlock::Create(cxt, "", taskFunction);
    IRBuilder<> lastHeaderSequentialExecutionBuilder(lastHeaderSequentialExecutionBlock);
    for (auto I : instsToMoveAndClone) {
      auto cloneI = I->clone();
      lastHeaderSequentialExecutionBuilder.Insert(cloneI);
      task->addInstruction(I, cloneI);
    }
    lastHeaderSequentialExecutionBuilder.CreateBr(cloneHeaderExit);

    brInst->replaceSuccessorWith(cloneHeaderExit, checkForLastExecutionBlock);
    IRBuilder<> checkForLastExecutionBuilder(checkForLastExecutionBlock);

    /*
     * Compute the loop governing IV's value the previous iteration
     * (regardless of what core it would have executed on)
     */
    auto originalGoverningPHI = loopGoverningIV.getLoopEntryPHI();
    auto cloneGoverningPHI = task->getCloneOfOriginalInstruction(originalGoverningPHI);
    auto stepSize = clonedStepSizeMap.at(&loopGoverningIV);
    auto prevIterIVValue = checkForLastExecutionBuilder.CreateSub(cloneGoverningPHI, stepSize);

    /*
     * Guard against this previous iteration.
     * If the condition would have exited the loop, go straight to the task exit
     * If not, this was the last execution of the header
     */
    auto prevIterGuard = updatedCmpInst->clone();
    prevIterGuard->replaceUsesOfWith(cloneGoverningPHI, prevIterIVValue);
    checkForLastExecutionBuilder.Insert(prevIterGuard);

    auto exitBlock = task->getExit();
    auto condTrueSuccessor = isTrueExiting ? exitBlock : lastHeaderSequentialExecutionBlock;
    auto condFalseSuccessor = isTrueExiting ? lastHeaderSequentialExecutionBlock : exitBlock;
    checkForLastExecutionBuilder.CreateCondBr(prevIterGuard, condTrueSuccessor, condFalseSuccessor);
  }
}