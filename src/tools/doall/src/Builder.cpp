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

namespace llvm::noelle { 

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
  auto invariantManager = LDI->getInvariantManager();
  auto loopSummary = LDI->getLoopStructure();
  auto loopHeader = loopSummary->getHeader();
  auto loopPreHeader = loopSummary->getPreHeader();
  auto preheaderClone = task->getCloneOfOriginalBasicBlock(loopPreHeader);
  auto headerClone = task->getCloneOfOriginalBasicBlock(loopHeader);
  auto allIVInfo = LDI->getInductionVariableManager();

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
   * Collect clones of step size deriving values for all induction variables
   * of the top level loop
   */
  auto clonedStepSizeMap = cloneIVStepValueComputation(LDI, 0, entryBuilder);

  /*
   * Determine start value of the IV for the task
   * core_start: original_start + original_step_size * core_id * chunk_size
   */
  for (auto ivInfo : allIVInfo->getInductionVariables(*loopSummary)) {
    auto startOfIV = fetchClone(ivInfo->getStartValue());
    auto stepOfIV = clonedStepSizeMap.at(ivInfo);
    auto ivPHI = cast<PHINode>(fetchClone(ivInfo->getLoopEntryPHI()));

    auto nthCoreOffset = entryBuilder.CreateMul(
      stepOfIV,
      entryBuilder.CreateZExtOrTrunc(
        entryBuilder.CreateMul(task->coreArg, task->chunkSizeArg, "coreIdx_X_chunkSize"),
        stepOfIV->getType()
      ),
      "stepSize_X_coreIdx_X_chunkSize"
    );

    auto offsetStartValue = IVUtility::offsetIVPHI(preheaderClone, ivPHI, startOfIV, nthCoreOffset);
    ivPHI->setIncomingValueForBlock(preheaderClone, offsetStartValue);
  }

  /*
   * Determine additional step size from the beginning of the next core's chunk
   * to the start of this core's next chunk
   * chunk_step_size: original_step_size * (num_cores - 1) * chunk_size
   */
  for (auto ivInfo : allIVInfo->getInductionVariables(*loopSummary)) {
    auto stepOfIV = clonedStepSizeMap.at(ivInfo);
    auto ivPHI = cast<PHINode>(fetchClone(ivInfo->getLoopEntryPHI()));
    auto onesValueForChunking = ConstantInt::get(chunkCounterType, 1);
    auto chunkStepSize = entryBuilder.CreateMul(
      stepOfIV,
      entryBuilder.CreateZExtOrTrunc(
        entryBuilder.CreateMul(
          entryBuilder.CreateSub(task->numCoresArg, onesValueForChunking, "numCoresMinus1"),
          task->chunkSizeArg,
          "numCoresMinus1_X_chunkSize"
        ),
        stepOfIV->getType()
      ),
      "stepSizeToNextChunk"
    );

    IVUtility::chunkInductionVariablePHI(preheaderClone, ivPHI, chunkPHI, chunkStepSize);
  }

  /*
   * The exit condition needs to be made non-strict to catch iterating past it
   */
  auto loopGoverningIVAttr = LDI->getLoopGoverningIVAttribution();
  LoopGoverningIVUtility ivUtility(*loopGoverningIVAttr);
  auto cmpInst = cast<CmpInst>(task->getCloneOfOriginalInstruction(loopGoverningIVAttr->getHeaderCmpInst()));
  auto brInst = cast<BranchInst>(task->getCloneOfOriginalInstruction(loopGoverningIVAttr->getHeaderBrInst()));
  auto basicBlockToJumpToWhenTheLoopEnds = task->getLastBlock(0);
  ivUtility.updateConditionAndBranchToCatchIteratingPastExitValue(cmpInst, brInst, basicBlockToJumpToWhenTheLoopEnds);
  auto updatedCmpInst = cmpInst;

  /*
   * The exit condition value does not need to be computed each iteration
   * and so the value's derivation can be hoisted into the preheader
   * 
   * Instructions that the PDG states are independent can include PHI nodes
   * Assert that any PHIs are invariant. Hoist one of those values (if instructions) to the preheader.
   */
  auto exitConditionValue = fetchClone(loopGoverningIVAttr->getExitConditionValue());
  if (auto exitConditionInst = dyn_cast<Instruction>(exitConditionValue)) {
    auto &derivation = ivUtility.getConditionValueDerivation();
    for (auto I : derivation) {
      assert(invariantManager->isLoopInvariant(I)
        && "DOALL exit condition value is not derived from loop invariant values!");

      /*
       * Fetch the clone of @I
       */
      auto cloneI = task->getCloneOfOriginalInstruction(I);

      if (auto clonePHI = dyn_cast<PHINode>(cloneI)) {
        auto usedValue = clonePHI->getIncomingValue(0);
        clonePHI->replaceAllUsesWith(usedValue);
        clonePHI->eraseFromParent();
        cloneI = dyn_cast<Instruction>(usedValue);
        if (!cloneI) continue;
      }

      cloneI->removeFromParent();
      entryBuilder.Insert(cloneI);
    }

    exitConditionInst->removeFromParent();
    entryBuilder.Insert(exitConditionInst);
  }

  /*
   * NOTE: When loop governing IV attribution allows for any other instructions in the header
   * other than those of the IV and its comparison, those unrelated instructions should be
   * copied into the body and the exit block (to preserve the number of times they execute)
   * 
   * The logic in the exit block must be guarded so only the "last" iteration executes it,
   * not any cores that pass the last iteration. This is further complicated because the mapping
   * of live-out environment producing instructions might need to be updated with the peeled
   * instructions in the exit block
   * 
   * A temporary mitigation is to transform loop latches with conditional branches that
   * verify if the next iteration would ever occur. This still requires live outs to be propagated
   * from both the header and the latches
   */

  /*
	 * Identify any instructions in the header that are NOT sensitive to the number of times they execute:
	 * 1) IV instructions, including the comparison and branch of the loop governing IV
	 * 2) The PHI used to chunk iterations 
	 * 3) Any PHIs of reducible variables
	 * 4) Any loop invariant instructions that belong to independent-execution SCCs
   */
  std::set<Instruction *> repeatableInstructions;

	/*
	 * Collect (1) by iterating the InductionVariableManager
	 */
  auto sccManager = LDI->getSCCManager();
  auto sccdag = sccManager->getSCCDAG();
  for (auto ivInfo : allIVInfo->getInductionVariables(*loopSummary)) {
    for (auto I : ivInfo->getAllInstructions()) {
      repeatableInstructions.insert(task->getCloneOfOriginalInstruction(I));
    }
  }
  repeatableInstructions.insert(cmpInst);
  repeatableInstructions.insert(brInst);

	/*
	 * Collect (2)
	 */
  repeatableInstructions.insert(chunkPHI);

	/*
	 * Collect (3) by identifying all reducible SCCs
	 */
  auto nonDOALLSCCs = sccManager->getSCCsWithLoopCarriedDataDependencies();
  for (auto scc : nonDOALLSCCs) {
    auto sccInfo = sccManager->getSCCAttrs(scc);
    if (!sccInfo->canExecuteReducibly()) continue;

    // HACK:
    for (auto nodePair : scc->internalNodePairs()) {
      auto value = nodePair.first;
      auto inst = cast<Instruction>(value);
      if (inst->getParent() != loopHeader) continue;

      auto instClone = task->getCloneOfOriginalInstruction(inst);
      repeatableInstructions.insert(instClone);
    }

    // assert(sccInfo->getSingleHeaderPHI());
    // repeatableInstructions.insert(task->getCloneOfOriginalInstruction(sccInfo->getSingleHeaderPHI()));
  }

	/*
	 * Collect (4) by identifying header instructions belonging to independent SCCs that are loop invariant
	 */
  for (auto &I : *loopHeader) {
		auto scc = sccdag->sccOfValue(&I);
    auto sccInfo = sccManager->getSCCAttrs(scc);
		if (!sccInfo->canExecuteIndependently()) continue;

    auto isInvariant = invariantManager->isLoopInvariant(&I);
    if (!isInvariant) continue;

		repeatableInstructions.insert(task->getCloneOfOriginalInstruction(&I));
	}

  /*
   * Check if we need to check whether we need to add a condition to execute instructions in the new header for tasks that are executing the header in iterations after the last one.
   */
  auto requiresConditionBeforeEnteringHeader = false;
  for (auto &I : *headerClone) {
    if (repeatableInstructions.find(&I) == repeatableInstructions.end()) {
      requiresConditionBeforeEnteringHeader = true;
      break;
    }
  }
  if (!requiresConditionBeforeEnteringHeader) {
    return ;
  }

  /*
   * The new header includes instructions that should be executed only if we know that we didn't pass the last iteration.
   * Hence, we need to add code to check this condition before entering the header.
   * Such code needs to be added for all predecessors of the header: pre-header and latches.
   *
   * Fetch the required information to generate the code.
   */
  auto &loopGoverningIV = loopGoverningIVAttr->getInductionVariable();
  auto loopGoverningPHI = task->getCloneOfOriginalInstruction(loopGoverningIV.getLoopEntryPHI());
  auto origValueUsedToCompareAgainstExitConditionValue = loopGoverningIVAttr->getValueToCompareAgainstExitConditionValue();
  auto valueUsedToCompareAgainstExitConditionValue = task->getCloneOfOriginalInstruction(origValueUsedToCompareAgainstExitConditionValue);
  assert(valueUsedToCompareAgainstExitConditionValue != nullptr);
  auto stepSize = clonedStepSizeMap.at(&loopGoverningIV);
  auto mappingFunction = [task] (Value *v) -> Value * {
    auto i = dyn_cast<Instruction>(v);
    if (i == nullptr){
      return nullptr;
    }
    auto c = task->getCloneOfOriginalInstruction(i);
    return c;
  };

  /*
   * In each latch, check whether we passed the last iteration.
   */
  for (auto latch : loopSummary->getLatches()) {

    /*
     * Fetch the latch in the loop within the task.
     */
    auto cloneLatch = task->getCloneOfOriginalBasicBlock(latch);

    /*
     * Remove the old terminator because it will replace with the check.
     */
    auto latchTerminator = cloneLatch->getTerminator();
    latchTerminator->eraseFromParent();
    IRBuilder<> latchBuilder(cloneLatch);

    /*
     * Fetch the value of the loop governing IV that would have been used to check whether the previous iteration was the last one.
     * To do so, we need to fetch the value of the loop-governing IV updated by the current iteration, which could be the IV value after updating it by adding the chunking size.
     * So for example, if 
     * - the current core excuted the iterations 0, 1, and 2 and 
     * - the chunking size is 3 and 
     * - there are 2 cores, then 
     * at the end of the iteration 2 (i.e., at the latch) of core 0 the updated loop-governing IV is 
     *   2 (the current value used in the compare instruction) 
     * + 1 (the normal IV increment) 
     * + 3 (the chunking size) * (2 - 1) (the other cores) 
     * ----
     *   6
     * 
     * The problem is that we don't know if the header of the iteration 6 should be executed at all as the loop might have ended at an earlier iteration (e.g., 4).
     * So we need to check whether the previous iteration (5 in the example) was actually executed.
     * To this end, we need to compare the previous iteration IV value (e.g., 5) against the exit condition.
     *
     * Fetch the updated loop-governing IV (6 in the example above).
     */
    auto currentIVValue = cast<PHINode>(loopGoverningPHI)->getIncomingValueForBlock(cloneLatch);

    /*
     * Compute the value that this IV had at the iteration before (5 in the example above).
     */
    auto prevIterationValue = ivUtility.generateCodeToComputePreviousValueUsedToCompareAgainstExitConditionValue(latchBuilder, currentIVValue, cloneLatch, stepSize);

    /*
     * Compare the previous-iteration IV value against the exit condition
     */
    auto clonedCmpInst = updatedCmpInst->clone();
    clonedCmpInst->replaceUsesOfWith(valueUsedToCompareAgainstExitConditionValue, prevIterationValue);
    latchBuilder.Insert(clonedCmpInst);
    latchBuilder.CreateCondBr(clonedCmpInst, task->getLastBlock(0), headerClone);
  }

  /*
   * In the preheader, assert that either the first iteration is being executed OR
   * that the previous iteration would have executed. The reason we must also check
   * if this is the first iteration is if the IV condition is such that <= 1
   * iteration would ever occur
   */
  auto preheaderTerminator = preheaderClone->getTerminator();
  preheaderTerminator->eraseFromParent();
  IRBuilder<> preheaderBuilder(preheaderClone);
  auto offsetStartValue = cast<PHINode>(loopGoverningPHI)->getIncomingValueForBlock(preheaderClone);
  auto prevIterationValue = ivUtility.generateCodeToComputeValueToUseForAnIterationAgo(preheaderBuilder, offsetStartValue, stepSize);

  auto clonedExitCmpInst = updatedCmpInst->clone();
  clonedExitCmpInst->replaceUsesOfWith(valueUsedToCompareAgainstExitConditionValue, prevIterationValue);
  preheaderBuilder.Insert(clonedExitCmpInst);

  auto startValue = fetchClone(loopGoverningIV.getStartValue());
  auto isNotFirstIteration = preheaderBuilder.CreateICmpNE(offsetStartValue, startValue);
  preheaderBuilder.CreateCondBr(
    preheaderBuilder.CreateAnd(isNotFirstIteration, clonedExitCmpInst),
    task->getExit(),
    headerClone
  );

  return ;
}

}
