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
     * FIXME: If we identify an IV as reducible, we produce a reducible execution
     * for it even if that is unnecessary. Fix that, then remove this check
     */ 
    auto scc = sccdag->sccOfValue(loopEntryPHI);
    auto sccInfo = LDI->sccdagAttrs.getSCCAttrs(scc);
    if (sccInfo->canExecuteReducibly()) continue;

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

    /*
     * If the step value is constant or a value present in the original loop, use its clone
     * TODO: Refactor this logic as a helper: tryAndFetchClone that doesn't assert a clone must exist
     */
    auto singleComputedStepValue = ivInfo->getSingleComputedStepValue();
    if (singleComputedStepValue) {
      Value *clonedStepValue = nullptr;
      if (isa<ConstantData>(singleComputedStepValue)
        || task->isAnOriginalLiveIn(singleComputedStepValue)) {
        clonedStepValue = singleComputedStepValue;
      } else if (auto singleComputedStepInst = dyn_cast<Instruction>(singleComputedStepValue)) {
        clonedStepValue = task->getCloneOfOriginalInstruction(singleComputedStepInst);
      }

      if (clonedStepValue) {
        clonedStepSizeMap.insert(std::make_pair(ivInfo, clonedStepValue));
        continue;
      }
    }

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
    auto clonedStepValue = task->getCloneOfOriginalInstruction(expandedInsts.back());
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

    ivUtility.updateConditionAndBranchToCatchIteratingPastExitValue(cmpInst, brInst, cloneHeaderExit);
    auto updatedCmpInst = cmpInst;
    auto updatedBrInst = brInst;

    auto headerSuccTrue = updatedBrInst->getSuccessor(0);
    auto headerSuccFalse = updatedBrInst->getSuccessor(1);
    auto isTrueExiting = headerSuccTrue == cloneHeaderExit;
    auto entryIntoBody = isTrueExiting ? headerSuccFalse : headerSuccTrue;

    /*
     * Collect instructions that cannot be in the header
     */
    std::vector<Instruction *> originalInstsBeingDuplicated;
    for (auto &I : *loopHeader) {
		  auto scc = sccdag->sccOfValue(&I);
      auto sccInfo = LDI->sccdagAttrs.getSCCAttrs(scc);
      auto sccType = sccInfo->getType();

      // I.print(errs() << "Investigating: "); errs() << "\n";

      /*
       * Ensure the original instruction was not independent, not a PHI, not clonable
       * and not part of this loop governing IV attribution
       * 
       * HACK: We don't have a way to ask if an instruction is repeatable, so to be safe,
       * anything that isn't belonging to an IV is duplicated
       */
      if (isa<PHINode>(&I)) continue;
      if (originalCmpInst == &I || originalBrInst == &I) continue;
      if (sccInfo->isInductionVariableSCC()) continue;

      // I.print(errs() << "Duplicating: "); errs() << "\n";

      originalInstsBeingDuplicated.push_back(&I);
    }

    /*
     * Move those instructions to the loop body (right at the beginning, in order)
     */
    auto firstBodyInst = entryIntoBody->getFirstNonPHIOrDbgOrLifetime();
    for (auto iIter = originalInstsBeingDuplicated.rbegin(); iIter != originalInstsBeingDuplicated.rend(); ++iIter) {
      auto originalI = *iIter;
      auto cloneI = task->getCloneOfOriginalInstruction(originalI);
      cloneI->moveBefore(firstBodyInst);
      firstBodyInst = cloneI;
    }

    auto taskFunction = task->getTaskBody();
    auto &cxt = taskFunction->getContext();
    auto checkForLastExecutionBlock = BasicBlock::Create(cxt, "", taskFunction);
    this->lastIterationExecutionBlock = BasicBlock::Create(cxt, "", taskFunction);
    IRBuilder<> lastIterationExecutionBuilder(this->lastIterationExecutionBlock);

    /*
     * Clone these instructions and execute them after exiting the loop ONLY IF
     * the previous iteration's IV value passes the loop guard.
     */
    for (auto originalI : originalInstsBeingDuplicated) {
      auto cloneI = task->getCloneOfOriginalInstruction(originalI);
      auto duplicateI = cloneI->clone();
      lastIterationExecutionBuilder.Insert(duplicateI);
      this->lastIterationExecutionDuplicateMap.insert(std::make_pair(originalI, duplicateI));
    }

    /*
     * Re-wire the cloned last execution instructions together
     */
    for (auto originalI : originalInstsBeingDuplicated) {
      auto duplicateI = this->lastIterationExecutionDuplicateMap.at(originalI);
      for (auto originalJ : originalInstsBeingDuplicated) {
        if (originalI == originalJ) continue;

        auto cloneJ = task->getCloneOfOriginalInstruction(originalJ);
        auto duplicateJ = this->lastIterationExecutionDuplicateMap.at(originalJ);
        duplicateI->replaceUsesOfWith(cloneJ, duplicateJ);
      }
    }

    lastIterationExecutionBuilder.CreateBr(cloneHeaderExit);
    updatedBrInst->replaceSuccessorWith(cloneHeaderExit, checkForLastExecutionBlock);
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
     * If the condition would have exited the loop, skip the last execution block
     * If not, this was the last execution of the header
     */
    auto prevIterGuard = updatedCmpInst->clone();
    prevIterGuard->replaceUsesOfWith(cloneGoverningPHI, prevIterIVValue);
    checkForLastExecutionBuilder.Insert(prevIterGuard);
    auto shortCircuitExit = task->getExit();
    auto prevIterGuardTrueSucc = isTrueExiting ? cloneHeaderExit : this->lastIterationExecutionBlock;
    auto prevIterGuardFalseSucc = isTrueExiting ? this->lastIterationExecutionBlock : cloneHeaderExit;
    checkForLastExecutionBuilder.CreateCondBr(prevIterGuard, prevIterGuardTrueSucc, prevIterGuardFalseSucc);

    // cmpInst->printAsOperand(errs() << "Cmp inst: "); errs() << "\n";
    // cloneHeaderExit->printAsOperand(errs() << "Header exit: "); errs() << "\n";
    // lastHeaderSequentialExecutionBlock->printAsOperand(errs() << "Last exec exit: "); errs() << "\n";
    // prevIterGuardTrueSucc->printAsOperand(errs() << "Block if prev guard is true: "); errs() << "\n";
    // prevIterGuardFalseSucc->printAsOperand(errs() << "Block if prev guard is false: "); errs() << "\n";

    /*
     * Create a PHI in the header exit block to represent potential reducible live out values properly
     * This has to happen because we duplicated logic. The correct live out is EITHER:
     * 1) the duplicated loop carried value within the last iteration block
     * 2) the loop carried value within the body from the previous iteration executed on this core
     * 
     * NOTE: Helix only has one task, as each core executes the same task
     */
    IRBuilder<> cloneHeaderExitBuilder(cloneHeaderExit->getFirstNonPHI());
    auto envUser = this->envBuilder->getUser(0);

    for (auto envIndex : envUser->getEnvIndicesOfLiveOutVars()) {
      auto originalProducer = (Instruction*)LDI->environment->producerAt(envIndex);
      if (this->lastIterationExecutionDuplicateMap.find(originalProducer) == this->lastIterationExecutionDuplicateMap.end()) continue;

      /*
       * This producer was duplicated. We need a PHI after the last iteration block to track whether
       * the duplicate executed or not (whether this core executed the last iteration or not)
       */
      auto cloneProducerInBody = task->getCloneOfOriginalInstruction(originalProducer);
      auto duplicateProducerInLastIterationBlock = this->lastIterationExecutionDuplicateMap.at(originalProducer);
      auto producerType = originalProducer->getType();

      /*
       * Create a PHI, recieving the body value if the last iteration didn't execute on this core,
       * and receiving the last iteration value if the last iteration did execute on this core
       */
      auto phi = cloneHeaderExitBuilder.CreatePHI(producerType, 2);
      phi->addIncoming(cloneProducerInBody, checkForLastExecutionBlock);
      phi->addIncoming(duplicateProducerInLastIterationBlock, lastIterationExecutionBlock);

      /*
       * Map from the original value of this producer to the PHI tracking the last value of this producer
       * NOTE: This is needed later when storing live outs
       */
      this->lastIterationExecutionDuplicateMap.erase(originalProducer);
      this->lastIterationExecutionDuplicateMap.insert(std::make_pair(originalProducer, phi));

    }

  }
}