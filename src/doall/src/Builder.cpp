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
  IRBuilder<> headerBuilder(headerClone->getFirstNonPHIOrDbgOrLifetime());
  auto loopGoverningIVPHI = cast<PHINode>(fetchClone(iv.getHeaderPHI()));
  auto chunkPHI = headerBuilder.CreatePHI(chunkCounterType, iv.getHeaderPHI()->getNumIncomingValues());
  auto zeroValueForChunking = ConstantInt::get(chunkCounterType, 0);

  for (auto i = 0; i < loopGoverningIVPHI->getNumIncomingValues(); ++i) {
    auto B = loopGoverningIVPHI->getIncomingBlock(i);
    IRBuilder<> latchBuilder(B->getTerminator());

    if (preheaderClone == B) {
      chunkPHI->addIncoming(zeroValueForChunking, B);
      loopGoverningIVPHI->setIncomingValue(i, offsetStartValue);
    } else {
      auto chunkIncrement = latchBuilder.CreateAdd(chunkPHI, onesValueForChunking);
      auto isChunkCompleted = latchBuilder.CreateICmp(CmpInst::Predicate::ICMP_EQ, chunkIncrement, task->chunkSizeArg);
      auto chunkWrap = latchBuilder.CreateSelect(isChunkCompleted, zeroValueForChunking, chunkIncrement, "chunkWrap");
      chunkPHI->addIncoming(chunkWrap, B);

      /*
       * Iterate to next chunk if necessary
       */
      loopGoverningIVPHI->setIncomingValue(i, latchBuilder.CreateSelect(
        isChunkCompleted,
        latchBuilder.CreateAdd(loopGoverningIVPHI->getIncomingValue(i), chunkStepSize),
        loopGoverningIVPHI->getIncomingValue(i),
        "checkToJumpToNextChunk"
      ));

      /*
       * Check if previous iteration succeeds header condition
       * If it doesn't, exit 
       */
      // auto originalTerminator = B->getTerminator();
      // ivUtility.cloneConditionalCheckFor(
      //   latchBuilder.CreateAdd(loopGoverningIVPHI->getIncomingValue(i), oppositeStepOfIV),
      //   exitConditionValue,
      //   headerClone,
      //   task->loopExitBlocks[0],
      //   latchBuilder
      // );
      // originalTerminator->eraseFromParent();
    }
  }

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
   * The exit condition needs to be made non-strict to catch iterating past it
   */
  auto headerBr = cast<BranchInst>(task->instructionClones.at(ivAttribution->getHeaderBrInst()));
  ivUtility.updateConditionToCatchIteratingPastExitValue(
    cast<CmpInst>(task->instructionClones.at(ivAttribution->getHeaderCmpInst()))
  );

  if (headerBr->getSuccessor(0) != task->loopExitBlocks[0]) {
    headerBr->setSuccessor(1, headerBr->getSuccessor(0));
    headerBr->setSuccessor(0, task->loopExitBlocks[0]);
  }

  /*
   * TODO: Any other instructions in the header other than the IV comparison need
   * to be copied into the body and the exit block
   * 
   * The logic in the exit block must be guarded so only the "last" iteration executes it,
   * not any cores that pass the last iteration 
   */

  /*
   * Ensure that the loop is never entered if the first iteration this core tries to execute
   * would never have occured sequentially
   * 
   * NOTE: the core executing the first iteration must ignore this check on the
   * "previous" iteration since no such iteration normally would have occurred. This is done
   * by fixing the previous value to the start value for the core executing the first iteration
   */
  // auto boundedPrevValue = entryBuilder.CreateSelect(
  //   entryBuilder.CreateICmp(CmpInst::Predicate::ICMP_EQ, offsetStartValue, startOfIV, "isStartIter"),
  //   startOfIV,
  //   entryBuilder.CreateAdd(offsetStartValue, oppositeStepOfIV, "previousIterationIV"),
  //   "boundedPreviousIterationIV"
  // );
  // ivUtility.cloneConditionalCheckFor(boundedPrevValue, exitConditionValue, headerClone, task->exitBlock, entryBuilder);
  // temporaryBrToLoop->eraseFromParent();

  /*
   * TODO: This shouldn't be needed, right?
   * Loosen the CmpInst to handle iterating past the exit value
   */
  // auto clonedCmpInst = cast<CmpInst>(fetchClone(ivAttribution->getHeaderCmpInst()));
  // ivUtility.updateConditionToCatchIteratingPastExitValue(clonedCmpInst);

  /*
   * Generate outer loop header, latch, PHI, and stepper
   */
  // auto &cxt = task->F->getContext();
  // task->outermostLoopHeader = BasicBlock::Create(cxt, "", task->F);
  // task->outermostLoopLatch = BasicBlock::Create(cxt, "", task->F);
  // IRBuilder<> outerHBuilder(task->outermostLoopHeader);
  // IRBuilder<> outerLBuilder(task->outermostLoopLatch);
  // task->outermostLoopIV = outerHBuilder.CreatePHI(startOfIV->getType(), /*numReservedValues=*/2);
  // auto outerIVStepper = outerLBuilder.CreateAdd(task->outermostLoopIV, chunkStepSize);
  // outerLBuilder.CreateBr(task->outermostLoopHeader);

  /*
   * Create outer loop IV lifecycle
   */
  // task->outermostLoopIV->addIncoming(outerIVStartVal, task->entryBlock);
  // task->outermostLoopIV->addIncoming(outerIVStepper, task->outermostLoopLatch);

  /*
   * Upon simplifying the original loop IV, the cmp to value was set to
   * the exclusive upper bound, hence the unsigned less than comparison
   */
  // auto outerIVCmp = outerHBuilder.CreateICmpULT(
  //   task->outermostLoopIV,
  //   task->clonedIVBounds.cmpIVTo
  // );
  // auto innerHeader = task->basicBlockClones[loopHeader];
  // outerHBuilder.CreateCondBr(outerIVCmp, innerHeader, task->loopExitBlocks[0]);

  /*
   * Hoist any values used to derive the compared to value up to the entry block
   */
  // auto &valueChain = task->clonedIVBounds.cmpToDerivation;
  // if (valueChain.size() > 0) {
  //   entryBuilder.SetInsertPoint((Instruction*)task->clonedIVBounds.cmpIVTo);
  //   // The 0th value in the chain is the compare to value itself, which is already hoisted
  //   for (auto i = valueChain.size() - 1; i >= 1; --i) {
  //     valueChain[i]->removeFromParent();
  //     entryBuilder.Insert(valueChain[i]);
  //   }
  //   entryBuilder.SetInsertPoint(task->entryBlock);
  // }

  /*
   * Reset inner loop start value to 0
   */
  // auto PHIType = task->cloneOfOriginalIV->getType();
  // auto startValueIndex = -1;
  // bool entryIndexIs0 = task->cloneOfOriginalIV->getIncomingBlock(0) == loopPreHeader;
  // bool entryIndexIs1 = task->cloneOfOriginalIV->getIncomingBlock(1) == loopPreHeader;
  // assert(entryIndexIs0 || entryIndexIs1);
  // task->cloneOfOriginalIV->setIncomingValue(
  //   entryIndexIs0 ? 0 : 1,
  //   ConstantInt::get(PHIType, 0)
  // );

  /*
   * NOTE(angelo): Also map entry block to outer loop header
   *  this should be done for all PHIs in the inner loop at the same time
   *  to avoid code duplication. See API propagatePHINodesThroughOuterLoop
   */
  // task->cloneOfOriginalIV->setIncomingBlock(
  //   entryIndexIs0 ? 0 : 1,
  //   task->outermostLoopHeader
  // );

  /*
   * Revise latch stepper instruction to increment
   */
  // assert(task->originalIVAttrs->numberOfAccumulators() == 1);
  // auto originStepper = *(task->originalIVAttrs->getAccumulators().begin());
  // auto innerStepper = task->instructionClones[originStepper];
  // bool stepIndexIs0 = isa<ConstantInt>(innerStepper->getOperand(0));
  // bool stepIndexIs1 = isa<ConstantInt>(innerStepper->getOperand(1));
  // assert(stepIndexIs0 || stepIndexIs1);
  // innerStepper->setOperand(
  //   stepIndexIs0 ? 0 : 1,
  //   ConstantInt::get(PHIType, 1)
  // );

  /*
   * Create new, composite induction variable for inner loop
   * Insert that composite variable after the inner loop PHI
   */
  // auto iIter = innerHeader->begin();
  // while (isa<PHINode>(&*(iIter++)));
  // IRBuilder<> headerBuilder(&*(--iIter));
  // auto sumIV = (Instruction *)headerBuilder.CreateAdd(
  //   task->cloneOfOriginalIV,
  //   task->outermostLoopIV
  // );

  /*
   * Fetch the single PHI of the induction variable.
   */
  // auto singlePHI = task->originalIVAttrs->getSinglePHI();
  // assert(singlePHI != nullptr);

  /*
   * Replace uses of the induction variable
   * (not including uses in the header or by the stepper instruction
   */
  // for (auto &use : singlePHI->uses()) {
  //   auto cloneUser = (Instruction *)use.getUser();
  //   if (task->instructionClones.find(cloneUser) == task->instructionClones.end()) continue;
  //   auto cloneI = task->instructionClones[cloneUser];
  //   if (cloneI == innerStepper || cloneI->getParent() == innerHeader) continue;
  //   // NOTE: The replacement is from the ORIGINAL PHI IV to the sum, not the clone to the sum
  //   //  This horrendous incongruency is because this function acts before data flow is adjusted
  //   //  Once that is changed, this can be made symmetric
  //   ((User *)cloneI)->replaceUsesOfWith(singlePHI, sumIV);
  // }

  /*
   * Replace inner loop condition with less than total loop size condition
   */
  // task->cloneOfOriginalCmp->setPredicate(CmpInst::Predicate::ICMP_ULT);
  // task->cloneOfOriginalCmp->setOperand(0, sumIV);
  // task->cloneOfOriginalCmp->setOperand(1, task->clonedIVBounds.cmpIVTo);

  /*
   * Add a condition to check that the IV is less than chunk size
   */
  // auto castChunkSize = entryBuilder.CreateZExtOrTrunc(
  //   task->chunkSizeArg,
  //   task->cloneOfOriginalIV->getType()
  // );
  // headerBuilder.SetInsertPoint(task->cloneOfOriginalBr);
  // Value *chunkCmp = headerBuilder.CreateICmpULT(task->cloneOfOriginalIV, castChunkSize);

  /*
   * Ensure both above conditions are met, that the inner loop IV is within bounds
   */
  // Value *inBoundsIV = headerBuilder.CreateBinOp(Instruction::And, chunkCmp, task->cloneOfOriginalCmp);

  /*
   * Get the entry block into the loop body
   */
  // auto loopBodyIndexIs0 = task->cloneOfOriginalBr->getSuccessor(0) != LDI->loopExitBlocks[0];
  // auto loopBodyIndexIs1 = task->cloneOfOriginalBr->getSuccessor(1) != LDI->loopExitBlocks[0];
  // assert(loopBodyIndexIs0 || loopBodyIndexIs1);
  // auto innerBodyBB = task->cloneOfOriginalBr->getSuccessor(loopBodyIndexIs0 ? 0 : 1);

  /*
   * Revise branch to go to the loop body if the IV is in bounds,
   * and to the outer loop latch if not
   */
  // task->cloneOfOriginalBr->setCondition(inBoundsIV);
  // task->cloneOfOriginalBr->setSuccessor(0, task->basicBlockClones[innerBodyBB]);
  // task->cloneOfOriginalBr->setSuccessor(1, task->outermostLoopLatch);

  /*
   * Finally, define branch from entry to outer loop
   */
  // entryBuilder.CreateBr(task->outermostLoopHeader);
}

void DOALL::propagatePHINodesThroughOuterLoop (
  LoopDependenceInfo *LDI
) {

//   /*
//    * Fetch the task
//    */
//   auto task = (DOALLTask *)tasks[0];

//   /*
//    * Fetch the header.
//    */
//   auto loopSummary = LDI->getLoopSummary();
//   auto loopHeader = loopSummary->getHeader();

//   /*
//    * Collect all PHIs (that aren't the IV)
//    */
//   auto innerHeader = task->basicBlockClones[loopHeader];
//   std::set<PHINode *> phis;
//   for (auto &I : *innerHeader) {
//     if (!isa<PHINode>(&I)) break ;
//     // Ignore the inner loop IV
//     if (&I == task->cloneOfOriginalIV) continue ;
//     phis.insert((PHINode *)&I);
//   }

//   /*
//    * Create equivalent outer loop PHIs for these inner loop PHIs
//    */
//   IRBuilder<> outerBuilder(&*task->outermostLoopHeader->begin());
//   std::unordered_map<PHINode *, PHINode *> innerToOuterPHI;
//   for (auto phi : phis) {
//     auto outerPHI = outerBuilder.CreatePHI(phi->getType(), /*numReservedValues=*/2);
//     innerToOuterPHI[phi] = outerPHI;

//     /*
//      * Adjust inner loop PHI to have the outer loop header as an incoming block
//      *  instead of the entry block (as the pre header mapped to the entry block)
//      */
//     auto innerEntryIndex = phi->getBasicBlockIndex(task->entryBlock);
//     phi->setIncomingBlock(innerEntryIndex, task->outermostLoopHeader);

//     /*
//      * Route the inner PHI and outer PHI values
//      */
//     auto startVal = phi->getIncomingValue(innerEntryIndex);
//     outerPHI->addIncoming(startVal, task->entryBlock);
//     outerPHI->addIncoming(phi, task->outermostLoopLatch);
//     phi->setIncomingValue(innerEntryIndex, outerPHI);
//   }

//   /*
//    * Associate live-out values with their new outer PHI counterpart instead of
//    * the inner PHI value so that when the the outer PHI is stored into the
//    * environment, as it is the most up-to-date value
//    */
//   for (auto envIndex : LDI->environment->getEnvIndicesOfLiveOutVars()) {
//     auto originalProducer = LDI->environment->producerAt(envIndex);
//     assert(isa<Instruction>(originalProducer));
//     auto originalProducerI = cast<Instruction>(originalProducer);
//     auto clonedProducer = task->instructionClones[originalProducerI];

//     /*
//      * NOTE: In the case of DOALL, all live out variables must be reducible
//      * All reducible variables are represented with PHIs in DOALL structured
//      * loops, so the PHI would have been mapped into the outer loop.
//      */
//     assert(isa<PHINode>(clonedProducer));
//     auto clonedPHI = cast<PHINode>(clonedProducer);
//     assert(innerToOuterPHI.find(clonedPHI) != innerToOuterPHI.end());

//     /*
//      * Remap clone of environment producer from the inner to the outer PHI
//      */
//     auto outerPHI = innerToOuterPHI[clonedPHI];
//     task->instructionClones[originalProducerI] = outerPHI;
//   }
}
