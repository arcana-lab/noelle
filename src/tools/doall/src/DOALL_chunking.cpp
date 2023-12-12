/*
 * Copyright 2016 - 2023  Angelo Matni, Simone Campanoni
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
#include "noelle/core/LoopIterationSCC.hpp"
#include "noelle/core/ReductionSCC.hpp"
#include "noelle/core/SingleAccumulatorRecomputableSCC.hpp"
#include "noelle/tools/DOALL.hpp"
#include "noelle/tools/DOALLTask.hpp"

namespace arcana::noelle {

void DOALL::rewireLoopToIterateChunks(LoopContent *LDI, DOALLTask *task) {

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
   * Generate PHI to track progress on the current chunk
   */
  IRBuilder<> entryBuilder(task->getEntry());
  auto jumpToLoop = task->getEntry()->getTerminator();
  entryBuilder.SetInsertPoint(jumpToLoop);
  auto chunkCounterType = task->chunkSizeArg->getType();
  auto chunkPHI = IVUtility::createChunkPHI(preheaderClone,
                                            headerClone,
                                            chunkCounterType,
                                            task->chunkSizeArg);

  /*
   * Collect clones of step size deriving values for all induction variables
   * of the parallelized loop.
   */
  auto clonedStepSizeMap =
      this->cloneIVStepValueComputation(LDI, 0, entryBuilder);

  /*
   * Determine start value of the IV for the task
   * The start value of an IV depends on the first iteration executed by a task.
   * This value, for a given task, is
   *    = original_start + (original_step_size * task_instance_id * chunk_size)
   *
   * where task_logical_id is the dynamic ID that spawn tasks will have, which
   * start at 0 (for the first task instance), 1 (for the second task instance),
   * until N-1 (for the last task instance).
   */
  for (auto ivInfo : allIVInfo->getInductionVariables(*loopSummary)) {
    auto startOfIV = this->fetchCloneInTask(task, ivInfo->getStartValue());
    auto stepOfIV = clonedStepSizeMap.at(ivInfo);
    auto loopEntryPHI = ivInfo->getLoopEntryPHI();
    auto ivPHI = cast<PHINode>(this->fetchCloneInTask(task, loopEntryPHI));

    auto nthCoreOffset = IVUtility::scaleInductionVariableStep(
        preheaderClone,
        ivPHI,
        stepOfIV,
        entryBuilder.CreateMul(task->taskInstanceID,
                               task->chunkSizeArg,
                               "coreIdx_X_chunkSize"));

    auto offsetStartValue =
        IVUtility::offsetIVPHI(preheaderClone, ivPHI, startOfIV, nthCoreOffset);
    ivPHI->setIncomingValueForBlock(preheaderClone, offsetStartValue);
  }

  /*
   * Determine additional step size
   *   from the beginning of the chunk that will be executed by the next task
   *   to the start of the next chunk that task-instance will execute.
   * The step size is this:
   *   chunk_step_size: original_step_size * (num_task_instances - 1) *
   * chunk_size
   */
  for (auto ivInfo : allIVInfo->getInductionVariables(*loopSummary)) {
    auto stepOfIV = clonedStepSizeMap.at(ivInfo);
    auto cloneLoopEntryPHI =
        this->fetchCloneInTask(task, ivInfo->getLoopEntryPHI());
    assert(cloneLoopEntryPHI != nullptr);
    auto ivPHI = cast<PHINode>(cloneLoopEntryPHI);
    auto onesValueForChunking = ConstantInt::get(chunkCounterType, 1);
    auto chunkStepSize = IVUtility::scaleInductionVariableStep(
        preheaderClone,
        ivPHI,
        stepOfIV,
        entryBuilder.CreateMul(entryBuilder.CreateSub(task->numTaskInstances,
                                                      onesValueForChunking,
                                                      "numCoresMinus1"),
                               task->chunkSizeArg,
                               "numCoresMinus1_X_chunkSize"));

    auto chunkedIVValues = IVUtility::chunkInductionVariablePHI(preheaderClone,
                                                                ivPHI,
                                                                chunkPHI,
                                                                chunkStepSize);
    this->IVValueJustBeforeEnteringBody[ivPHI] = chunkedIVValues;
  }

  /*
   * Fetch the SCCDAG of the loop.
   */
  auto sccManager = LDI->getSCCManager();
  auto sccdag = sccManager->getSCCDAG();

  /*
   * Generates code for periodic variable SCCs to match the DOALL chunking
   * strategy.
   */
  for (auto scc : sccdag->getSCCs()) {
    auto sccInfo = sccManager->getSCCAttrs(scc);
    auto periodicVariableSCC = dyn_cast<PeriodicVariableSCC>(sccInfo);
    if (periodicVariableSCC == nullptr)
      continue;

    if (this->verbose >= Verbosity::Maximal) {
      errs()
          << "DOALL: periodic variable with initial value "
          << *periodicVariableSCC->getInitialValue() << "\n"
          << "       and period " << *periodicVariableSCC->getPeriod() << "\n"
          << "       and step " << *periodicVariableSCC->getStepValue() << "\n";
    }

    /*
     * Retrieve the relevant values of the periodic variable SCC.
     */
    auto initialValue = periodicVariableSCC->getInitialValue();
    auto period = periodicVariableSCC->getPeriod();
    auto step = periodicVariableSCC->getStepValue();
    auto phi =
        periodicVariableSCC->getPhiThatAccumulatesValuesBetweenLoopIterations();
    assert(
        phi->getNumIncomingValues() == 2
        && "DOALL: PHINode in periodic variable SCC doesn't have exactly two entries!");
    auto taskPHI = cast<PHINode>(task->getCloneOfOriginalInstruction(phi));

    unsigned entryBlock, loopBlock;
    if (phi->getIncomingValue(0) == initialValue) {
      entryBlock = 0;
      loopBlock = 1;
    } else {
      assert(phi->getIncomingValue(1) == initialValue
             && "DOALL: periodic variable SCC selected the wrong PHINode!");
      entryBlock = 1;
      loopBlock = 0;
    }
    auto taskLoopBlock =
        task->getCloneOfOriginalBasicBlock(phi->getIncomingBlock(loopBlock));
    auto loopValue = phi->getIncomingValue(loopBlock);
    auto taskLoopValue =
        task->getCloneOfOriginalInstruction(cast<Instruction>(loopValue));

    /*
     * Calculate the periodic variable's initial value for the task.
     * This value is: initialValue + step_size * ((task_id * chunk_size) %
     * period)
     */
    auto coreIDxChunkSize = entryBuilder.CreateMul(task->taskInstanceID,
                                                   task->chunkSizeArg,
                                                   "coreIdx_X_chunkSize");
    auto numSteps =
        entryBuilder.CreateSRem(coreIDxChunkSize, period, "numSteps");
    auto numStepsTrunc = entryBuilder.CreateTrunc(numSteps, step->getType());
    auto numStepsxStepSize =
        entryBuilder.CreateMul(step, numStepsTrunc, "stepSize_X_numSteps");
    auto numStepsxStepSizeTrunc =
        entryBuilder.CreateTrunc(numStepsxStepSize, initialValue->getType());
    auto chunkInitialValue = entryBuilder.CreateAdd(initialValue,
                                                    numStepsxStepSizeTrunc,
                                                    "initialValuePlusStep");
    taskPHI->setIncomingValue(entryBlock, chunkInitialValue);

    /*
     * Determine value of the start of this core's next chunk
     * from the beginning of the next core's chunk.
     * Formula: (next_chunk_initialValue + (step_size * (num_cores - 1) *
     * chunk_size)) % period
     */
    auto onesValueForChunking = ConstantInt::get(chunkCounterType, 1);
    auto numCoresMinus1 = entryBuilder.CreateSub(task->numTaskInstances,
                                                 onesValueForChunking,
                                                 "numCoresMinus1");
    auto chunkStepSize = entryBuilder.CreateMul(numCoresMinus1,
                                                task->chunkSizeArg,
                                                "numCoresMinus1_X_chunkSize");
    auto chunkStepSizeTrunc =
        entryBuilder.CreateTrunc(chunkStepSize, step->getType());
    auto chunkStep =
        entryBuilder.CreateMul(chunkStepSizeTrunc, step, "chunkStep");

    /*
     * Add the instructions for the calculation of the next chunk's start value
     * in the loop's body.
     */
    IRBuilder<> loopBuilder(taskLoopBlock);
    loopBuilder.SetInsertPoint(taskLoopBlock->getTerminator());
    auto chunkStepTrunc =
        loopBuilder.CreateTrunc(chunkStep, taskLoopValue->getType());
    auto nextChunkValueBeforeMod =
        loopBuilder.CreateAdd(taskLoopValue,
                              chunkStepTrunc,
                              "nextChunkValueBeforeMod");
    auto periodTrunc =
        loopBuilder.CreateTrunc(period, taskLoopValue->getType());
    auto nextChunkValue = loopBuilder.CreateSRem(nextChunkValueBeforeMod,
                                                 periodTrunc,
                                                 "nextChunkValue");

    /*
     * Determine if we have reached the end of the chunk, and choose the
     * periodic variable's next value accordingly.
     */
    auto isChunkCompleted =
        cast<SelectInst>(chunkPHI->getIncomingValueForBlock(taskLoopBlock))
            ->getCondition();
    auto nextValue = loopBuilder.CreateSelect(isChunkCompleted,
                                              nextChunkValue,
                                              taskLoopValue,
                                              "nextValue");
    taskPHI->setIncomingValueForBlock(taskLoopBlock, nextValue);
  }

  /*
   * The exit condition needs to be made non-strict to catch iterating past it
   */
  auto loopGoverningIVAttr = allIVInfo->getLoopGoverningInductionVariable();
  LoopGoverningIVUtility ivUtility(loopSummary,
                                   *allIVInfo,
                                   *loopGoverningIVAttr);
  auto cmpInst = cast<CmpInst>(task->getCloneOfOriginalInstruction(
      loopGoverningIVAttr
          ->getHeaderCompareInstructionToComputeExitCondition()));
  auto brInst = cast<BranchInst>(task->getCloneOfOriginalInstruction(
      loopGoverningIVAttr->getHeaderBrInst()));
  auto basicBlockToJumpToWhenTheLoopEnds = task->getLastBlock(0);
  ivUtility.updateConditionAndBranchToCatchIteratingPastExitValue(
      cmpInst,
      brInst,
      basicBlockToJumpToWhenTheLoopEnds);
  auto updatedCmpInst = cmpInst;

  /*
   * The exit condition value does not need to be computed each iteration and so
   * the value's derivation can be hoisted into the preheader.
   *
   * Instructions that the PDG states are independent can include PHI nodes.
   * Assert that any PHIs are invariant. Hoist one of those values (if
   * instructions) to the preheader.
   */
  auto exitConditionValue =
      this->fetchCloneInTask(task,
                             loopGoverningIVAttr->getExitConditionValue());
  assert(exitConditionValue != nullptr);
  if (auto exitConditionInst = dyn_cast<Instruction>(exitConditionValue)) {
    auto &derivation = ivUtility.getConditionValueDerivation();
    for (auto I : derivation) {
      assert(
          invariantManager->isLoopInvariant(I)
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
        if (!cloneI) {
          continue;
        }
      }

      cloneI->removeFromParent();
      entryBuilder.Insert(cloneI);
    }

    exitConditionInst->removeFromParent();
    entryBuilder.Insert(exitConditionInst);
  }

  /*
   * NOTE: When loop governing IV attribution allows for any other instructions
   * in the header other than those of the IV and its comparison, those
   * unrelated instructions should be copied into the body and the exit block
   * (to preserve the number of times they execute)
   *
   * The logic in the exit block must be guarded so only the "last" iteration
   * executes it, not any cores that pass the last iteration. This is further
   * complicated because the mapping of live-out environment producing
   * instructions might need to be updated with the peeled instructions in the
   * exit block
   *
   * A temporary mitigation is to transform loop latches with conditional
   * branches that verify if the next iteration would ever occur. This still
   * requires live outs to be propagated from both the header and the latches
   */

  /*
   * Identify any instructions in the header that are NOT sensitive to the
   * number of times they execute: 1) IV instructions, including the comparison
   * and branch of the loop governing IV 2) The PHI used to chunk iterations 3)
   * Any PHIs of reducible variables 4) Any loop invariant instructions that
   * belong to independent-execution SCCs
   */
  std::unordered_set<Instruction *> repeatableInstructions;
  std::unordered_set<Instruction *> reducibleHeaderPHIsWithHeaderLogic;

  /*
   * Collect (1) by iterating the InductionVariableManager
   */
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
  for (auto sccInfo : nonDOALLSCCs) {
    auto reductionSCC = dyn_cast<ReductionSCC>(sccInfo);
    if (reductionSCC == nullptr)
      continue;

    auto headerPHI =
        reductionSCC->getPhiThatAccumulatesValuesBetweenLoopIterations();
    assert(headerPHI != nullptr);

    auto hasInstsInHeader = false;
    auto scc = sccInfo->getSCC();
    for (auto nodePair : scc->internalNodePairs()) {
      auto value = nodePair.first;
      auto inst = cast<Instruction>(value);
      if (inst->getParent() != loopHeader)
        continue;

      auto instClone = task->getCloneOfOriginalInstruction(inst);
      repeatableInstructions.insert(instClone);
      hasInstsInHeader = true;
    }

    if (hasInstsInHeader) {
      auto headerPHIClone = task->getCloneOfOriginalInstruction(headerPHI);
      reducibleHeaderPHIsWithHeaderLogic.insert(headerPHIClone);
    }
  }

  /*
   * Collect (4) by identifying header instructions belonging to independent
   * SCCs that are loop invariant
   */
  for (auto &I : *loopHeader) {
    auto scc = sccdag->sccOfValue(&I);
    auto sccInfo = sccManager->getSCCAttrs(scc);
    if (!isa<LoopIterationSCC>(sccInfo))
      continue;

    auto isInvariant = invariantManager->isLoopInvariant(&I);
    if (!isInvariant)
      continue;

    repeatableInstructions.insert(task->getCloneOfOriginalInstruction(&I));
  }

  /*
   * Fetch the required information to generate any extra condition code needed.
   */
  auto loopGoverningIV = loopGoverningIVAttr->getInductionVariable();
  auto loopGoverningPHI =
      task->getCloneOfOriginalInstruction(loopGoverningIV->getLoopEntryPHI());
  auto origValueUsedToCompareAgainstExitConditionValue =
      loopGoverningIVAttr->getValueToCompareAgainstExitConditionValue();
  auto valueUsedToCompareAgainstExitConditionValue =
      task->getCloneOfOriginalInstruction(
          origValueUsedToCompareAgainstExitConditionValue);
  assert(valueUsedToCompareAgainstExitConditionValue != nullptr);
  auto stepSize = clonedStepSizeMap.at(loopGoverningIV);

  /*
   * Check if we need to check whether we need to add a condition to execute
   * instructions in the new header for tasks that are executing the header in
   * iterations after the last one.
   */
  auto requiresConditionBeforeEnteringHeader = false;
  for (auto &I : *headerClone) {
    if (repeatableInstructions.find(&I) == repeatableInstructions.end()) {
      requiresConditionBeforeEnteringHeader = true;
      break;
    }
  }

  if (!requiresConditionBeforeEnteringHeader) {

    /*
     * We have to handle the special case where there are reducible SCCs (i.e.,
     * reducable variables at the source code level) for which some of the
     * non-PHI instructions are also contained in the header. For example,
     * consider the following code:
     *
     * BB0:
     *   ...
     *   br %BB1
     *
     * BB1:
     *  %v2 = PHI [%v1, BB1], [%v0, BB0]
     *  ...
     *  %v1 = add %v2, 1
     *  ...
     *  br %c %BB1, %BB2
     *
     * BB2:
     *  return
     *
     *
     * This is a special case because there are two values that we could use to
     * store into the reduction variable:
     * 1) the PHI instruction (e.g., %v2)
     * 2) the non-PHI instruction that does the accumulation (e.g., %v1) We need
     * to use the right value depending on whether the header would NOT have
     * executed its last iteration. If that is the case, then we need to use the
     * PHI instruction. Otherwise, if the last instance of the header was meant
     * to be executed, then we need to use the non-PHI instruction.
     *
     * To solve this problem, we are going to inject a new SelectInst that
     * checks whether the last execution of the header was meant to be executed.
     * This SelectInst will be inserted into the basic block that leaves the
     * task, just before storing the right value into the reduction variable of
     * the current task.
     */
    auto env = LDI->getEnvironment();
    auto envUser = this->envBuilder->getUser(0);
    std::vector<std::pair<Instruction *, Instruction *>>
        headerPHICloneAndProducerPairs;
    for (auto envID : envUser->getEnvIDsOfLiveOutVars()) {

      /*
       * Fetch the clone of the producer of the current live-out variable.
       * Fetch the header PHI of the live-out variable.
       */
      auto producer = cast<Instruction>(env->getProducer(envID));
      assert(producer != nullptr);
      auto scc = sccdag->sccOfValue(producer);
      assert(scc != nullptr);
      auto sccInfo = sccManager->getSCCAttrs(scc);
      assert(sccInfo != nullptr);

      /*
       * Check if the current live-out variable is reducible
       *
       * Live-out variables that are not reducible are handled separetely by
       * "generateCodeToStoreLiveOutVariables"
       */
      if (!isa<ReductionSCC>(sccInfo)) {
        continue;
      }
      auto reductionSCC = cast<ReductionSCC>(sccInfo);

      /*
       * Check whether the header PHI is part of the set of PHIs we need to
       * guard
       */
      auto headerPHI =
          reductionSCC->getPhiThatAccumulatesValuesBetweenLoopIterations();
      assert(headerPHI != nullptr);
      auto clonePHI = task->getCloneOfOriginalInstruction(headerPHI);
      assert(clonePHI != nullptr);
      if (reducibleHeaderPHIsWithHeaderLogic.find(clonePHI)
          != reducibleHeaderPHIsWithHeaderLogic.end()) {
        headerPHICloneAndProducerPairs.push_back(
            std::make_pair(clonePHI, producer));
      }
    }

    /*
     * Produce exit block SelectInst for all reducible SCCs that have header
     * logic
     */
    if (headerPHICloneAndProducerPairs.size() > 0) {
      auto startValue =
          this->fetchCloneInTask(task, loopGoverningIV->getStartValue());

      /*
       * Piece together the condition for all the SelectInst:
       * ((prev loop-governing IV's value triggered exiting the loop) && (IV
       * header PHI != start value)) ? header phi // this will contain the
       * pre-header value or the previous latch value : original producer //
       * this will be the live out value from the header
       */
      IRBuilder<> exitBuilder(
          task->getLastBlock(0)->getFirstNonPHIOrDbgOrLifetime());
      auto prevIterationValue =
          ivUtility.generateCodeToComputeValueToUseForAnIterationAgo(
              exitBuilder,
              loopGoverningPHI,
              stepSize);
      auto headerToExitCmp = updatedCmpInst->clone();
      headerToExitCmp->replaceUsesOfWith(
          valueUsedToCompareAgainstExitConditionValue,
          prevIterationValue);
      exitBuilder.Insert(headerToExitCmp);
      auto wasNotFirstIteration =
          exitBuilder.CreateICmpNE(loopGoverningPHI, startValue);
      auto skipLastHeader =
          exitBuilder.CreateAnd(wasNotFirstIteration, headerToExitCmp);

      /*
       * Use SelectInst created above to propagate the correct live out value
       * for all reducible SCCs that have header logic
       */
      for (auto pair : headerPHICloneAndProducerPairs) {
        auto headerPHIClone = pair.first;
        auto producer = pair.second;
        auto producerClone = task->getCloneOfOriginalInstruction(producer);
        auto lastReducedInst =
            cast<Instruction>(exitBuilder.CreateSelect(skipLastHeader,
                                                       headerPHIClone,
                                                       producerClone));

        /*
         * HACK: Replace original producer clone entry with new SelectInst
         * What would be cleaner is to invoke task->addLiveOut(producer,
         * lastReducedInst) but this would require ParallelizationTechnique to
         * support the possibility that its internal liveOutClones map could
         * contain values with no equivalent in the original live out SCC.
         * TODO: Update
         * fetchOrCreatePHIForIntermediateProducerValueOfReducibleLiveOutVariable
         * to support finding potentially newly created values that are inserted
         * into the liveOutClones map via the addLiveOut API
         */
        task->addInstruction(producer, lastReducedInst);
      }
    }

    /*
     * There is no need for pre-header / latch guards, so we return
     * TODO: Isolate reducible live out guards and pre-header / latch guards to
     * helper methods so this function's control flow is simpler
     */
    return;
  }

  /*
   * The new header includes instructions that should be executed only if we
   * know that we didn't pass the last iteration. Hence, we need to add code to
   * check this condition before entering the header. Such code needs to be
   * added for all predecessors of the header: pre-header and latches.
   */

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
     * Fetch the value of the loop governing IV that would have been used to
     * check whether the previous iteration was the last one. To do so, we need
     * to fetch the value of the loop-governing IV updated by the current
     * iteration, which could be the IV value after updating it by adding the
     * chunking size. So for example, if
     * - the current core excuted the iterations 0, 1, and 2 and
     * - the chunking size is 3 and
     * - there are 2 cores, then
     * at the end of the iteration 2 (i.e., at the latch) of core 0 the updated
     * loop-governing IV is 2 (the current value used in the compare
     * instruction)
     * + 1 (the normal IV increment)
     * + 3 (the chunking size) * (2 - 1) (the other cores)
     * ----
     *   6
     *
     * The problem is that we don't know if the header of the iteration 6 should
     * be executed at all as the loop might have ended at an earlier iteration
     * (e.g., 4). So we need to check whether the previous iteration (5 in the
     * example) was actually executed. To this end, we need to compare the
     * previous iteration IV value (e.g., 5) against the exit condition.
     *
     * Fetch the updated loop-governing IV (6 in the example above).
     */
    auto currentIVValue =
        cast<PHINode>(loopGoverningPHI)->getIncomingValueForBlock(cloneLatch);

    /*
     * Compute the value that this IV had at the iteration before (5 in the
     * example above).
     */
    auto prevIterationValue =
        ivUtility
            .generateCodeToComputePreviousValueUsedToCompareAgainstExitConditionValue(
                latchBuilder,
                currentIVValue,
                stepSize);

    /*
     * Compare the previous-iteration IV value against the exit condition
     */
    auto clonedCmpInst = updatedCmpInst->clone();
    clonedCmpInst->replaceUsesOfWith(
        valueUsedToCompareAgainstExitConditionValue,
        prevIterationValue);
    latchBuilder.Insert(clonedCmpInst);
    latchBuilder.CreateCondBr(clonedCmpInst,
                              task->getLastBlock(0),
                              headerClone);
  }

  /*
   * In the preheader, assert that either the first iteration is being executed
   * OR that the previous iteration would have executed. The reason we must also
   * check if this is the first iteration is if the IV condition is such that <=
   * 1 iteration would ever occur
   */
  auto preheaderTerminator = preheaderClone->getTerminator();
  preheaderTerminator->eraseFromParent();
  IRBuilder<> preheaderBuilder(preheaderClone);
  auto offsetStartValue =
      cast<PHINode>(loopGoverningPHI)->getIncomingValueForBlock(preheaderClone);
  auto prevIterationValue =
      ivUtility.generateCodeToComputeValueToUseForAnIterationAgo(
          preheaderBuilder,
          offsetStartValue,
          stepSize);

  auto clonedExitCmpInst = updatedCmpInst->clone();
  clonedExitCmpInst->replaceUsesOfWith(
      valueUsedToCompareAgainstExitConditionValue,
      prevIterationValue);
  preheaderBuilder.Insert(clonedExitCmpInst);

  auto startValue =
      this->fetchCloneInTask(task, loopGoverningIV->getStartValue());
  assert(startValue != nullptr);
  auto isNotFirstIteration =
      preheaderBuilder.CreateICmpNE(offsetStartValue, startValue);
  preheaderBuilder.CreateCondBr(
      preheaderBuilder.CreateAnd(isNotFirstIteration, clonedExitCmpInst),
      task->getExit(),
      headerClone);
}

} // namespace arcana::noelle
