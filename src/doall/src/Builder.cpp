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
  std::unordered_map<InductionVariable *, Value *> clonedStepSizeMap;
  for (auto ivInfo : allIVInfo->getInductionVariables(*loopSummary)) {
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
  auto cmpInst = cast<CmpInst>(task->getCloneOfOriginalInstruction(loopGoverningIVAttr->getHeaderCmpInst()));
  auto brInst = cast<BranchInst>(task->getCloneOfOriginalInstruction(loopGoverningIVAttr->getHeaderBrInst()));
  ivUtility.updateConditionAndBranchToCatchIteratingPastExitValue(cmpInst, brInst, task->getLastBlock(0));
  auto updatedCmpInst = cmpInst;

  /*
   * The exit condition value does not need to be computed each iteration
   * and so the value's derivation can be hoisted into the preheader
   * 
   * NOTE: As of now, instructions which the PDG states are independent can include
   * PHI nodes containing the re-loading of a global. Without further analysis,
   * the following simplification would hoist that PHI out of its block.
   */
  // auto exitConditionValue = fetchClone(loopGoverningIVAttr->getHeaderCmpInstConditionValue());
  // if (auto exitConditionInst = dyn_cast<Instruction>(exitConditionValue)) {
  //   auto &derivation = ivUtility.getConditionValueDerivation();
  //   for (auto I : derivation) {
  //     auto cloneI = task->getCloneOfOriginalInstruction(I);
  //     cloneI->removeFromParent();
  //     entryBuilder.Insert(cloneI);
  //   }

  //   exitConditionInst->removeFromParent();
  //   entryBuilder.Insert(exitConditionInst);
  // }

  /*
   * NOTE: When loop governing IV attribution allows for any bther instructions in the header
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
  auto sccdag = LDI->sccdagAttrs.getSCCDAG();
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
  auto nonDOALLSCCs = LDI->sccdagAttrs.getSCCsWithLoopCarriedDataDependencies();
  for (auto scc : nonDOALLSCCs) {
    auto sccInfo = LDI->sccdagAttrs.getSCCAttrs(scc);
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
	 * NOTE: This should be done with an API call to LoopStructure, but until that API is
	 * more robust, we simply check that the instruction consumes no data dependencies in the SCCDAG
	 */
  for (auto &I : *loopHeader) {
		auto scc = sccdag->sccOfValue(&I);
    auto sccInfo = LDI->sccdagAttrs.getSCCAttrs(scc);
		if (!sccInfo->canExecuteIndependently()) continue;

		auto node = scc->fetchNode(&I);
		bool consumesDataDependence = true;
		for (auto edge : node->getIncomingEdges()) {
			consumesDataDependence |= edge->isDataDependence();
		}
		if (consumesDataDependence) continue;

		repeatableInstructions.insert(task->getCloneOfOriginalInstruction(&I));
	}

  bool requiresConditionBeforeEnteringHeader = false;
  for (auto &I : *headerClone) {
    if (repeatableInstructions.find(&I) == repeatableInstructions.end()) {
      assert(!isa<PHINode>(I)
        && "All PHIs (which have loop carried dependencies) must be chunked or reducible");
      requiresConditionBeforeEnteringHeader = true;
      break;
    }
  }

  if (requiresConditionBeforeEnteringHeader) {
    auto &loopGoverningIV = loopGoverningIVAttr->getInductionVariable();
    auto loopGoverningPHI = task->getCloneOfOriginalInstruction(loopGoverningIV.getLoopEntryPHI());
    auto stepSize = clonedStepSizeMap.at(&loopGoverningIV);

    /*
     * In each latch, assert that the previous iteration would have executed
     */
    for (auto latch : loopSummary->getLatches()) {
      BasicBlock *cloneLatch = task->getCloneOfOriginalBasicBlock(latch);
      // cloneLatch->print(errs() << "Addressing latch:\n");
      auto latchTerminator = cloneLatch->getTerminator();
      latchTerminator->eraseFromParent();
      IRBuilder<> latchBuilder(cloneLatch);

      auto currentIVValue = cast<PHINode>(loopGoverningPHI)->getIncomingValueForBlock(cloneLatch);
      auto prevIterationValue = latchBuilder.CreateSub(currentIVValue, stepSize);
      auto clonedCmpInst = updatedCmpInst->clone();
      clonedCmpInst->replaceUsesOfWith(loopGoverningPHI, prevIterationValue);
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
    auto prevIterationValue = preheaderBuilder.CreateSub(offsetStartValue, stepSize);

    auto clonedExitCmpInst = updatedCmpInst->clone();
    clonedExitCmpInst->replaceUsesOfWith(loopGoverningPHI, prevIterationValue);
    preheaderBuilder.Insert(clonedExitCmpInst);
    auto startValue = fetchClone(loopGoverningIV.getStartValue());
    auto isNotFirstIteration = preheaderBuilder.CreateICmpNE(offsetStartValue, startValue);
    preheaderBuilder.CreateCondBr(
      preheaderBuilder.CreateAnd(isNotFirstIteration, clonedExitCmpInst),
      task->getExit(),
      headerClone
    );
  }
}
