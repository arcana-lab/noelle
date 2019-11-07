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

void DOALL::generateOuterLoopAndAdjustInnerLoop (
  LoopDependenceInfo *LDI
){
  auto task = (DOALLTask *)tasks[0];

  /*
   * Determine start value and step size for outer loop IV
   */
  IRBuilder<> entryBuilder(task->entryBlock);
  auto startOfIV = task->clonedIVBounds.start;
  auto coreIDTimesChunkSize = entryBuilder.CreateMul(task->coreArg, task->chunkSizeArg);
  auto nthCoreOffset = entryBuilder.CreateZExtOrTrunc(
    coreIDTimesChunkSize,
    startOfIV->getType()
  );
  auto outerIVStartVal = entryBuilder.CreateAdd(startOfIV, nthCoreOffset);

  /*
   * Since the step size of the original IV is simplified to be +1,
   * the offset of chunk size * num cores is the entire step size
   */
  auto numCoresOffset = entryBuilder.CreateZExtOrTrunc(
    entryBuilder.CreateMul(task->numCoresArg, task->chunkSizeArg),
    startOfIV->getType()
  );

  /*
   * Generate outer loop header, latch, PHI, and stepper
   */
  auto &cxt = task->F->getContext();
  task->outermostLoopHeader = BasicBlock::Create(cxt, "", task->F);
  task->outermostLoopLatch = BasicBlock::Create(cxt, "", task->F);
  IRBuilder<> outerHBuilder(task->outermostLoopHeader);
  IRBuilder<> outerLBuilder(task->outermostLoopLatch);
  task->outermostLoopIV = outerHBuilder.CreatePHI(startOfIV->getType(), /*numReservedValues=*/2);
  auto outerIVStepper = outerLBuilder.CreateAdd(task->outermostLoopIV, numCoresOffset);
  outerLBuilder.CreateBr(task->outermostLoopHeader);

  /*
   * Create outer loop IV lifecycle
   */
  task->outermostLoopIV->addIncoming(outerIVStartVal, task->entryBlock);
  task->outermostLoopIV->addIncoming(outerIVStepper, task->outermostLoopLatch);

  /*
   * Upon simplifying the original loop IV, the cmp to value was set to
   * the exclusive upper bound, hence the unsigned less than comparison
   */
  auto outerIVCmp = outerHBuilder.CreateICmpULT(
    task->outermostLoopIV,
    task->clonedIVBounds.cmpIVTo
  );
  auto innerHeader = task->basicBlockClones[LDI->header];
  outerHBuilder.CreateCondBr(outerIVCmp, innerHeader, task->loopExitBlocks[0]);

  /*
   * Hoist any values used to derive the compared to value up to the entry block
   */
  auto &valueChain = task->clonedIVBounds.cmpToDerivation;
  if (valueChain.size() > 0) {
    entryBuilder.SetInsertPoint((Instruction*)task->clonedIVBounds.cmpIVTo);
    // The 0th value in the chain is the compare to value itself, which is already hoisted
    for (auto i = valueChain.size() - 1; i >= 1; --i) {
      valueChain[i]->removeFromParent();
      entryBuilder.Insert(valueChain[i]);
    }
    entryBuilder.SetInsertPoint(task->entryBlock);
  }

  /*
   * Reset inner loop start value to 0
   */
  auto PHIType = task->cloneOfOriginalIV->getType();
  auto startValueIndex = -1;
  bool entryIndexIs0 = task->cloneOfOriginalIV->getIncomingBlock(0) == LDI->preHeader;
  bool entryIndexIs1 = task->cloneOfOriginalIV->getIncomingBlock(1) == LDI->preHeader;
  assert(entryIndexIs0 || entryIndexIs1);
  task->cloneOfOriginalIV->setIncomingValue(
    entryIndexIs0 ? 0 : 1,
    ConstantInt::get(PHIType, 0)
  );

  /*
   * NOTE(angelo): Also map entry block to outer loop header
   *  this should be done for all PHIs in the inner loop at the same time
   *  to avoid code duplication. See API propagatePHINodesThroughOuterLoop
   */
  task->cloneOfOriginalIV->setIncomingBlock(
    entryIndexIs0 ? 0 : 1,
    task->outermostLoopHeader
  );

  /*
   * Revise latch stepper instruction to increment
   */
  assert(task->originalIVAttrs->numberOfAccumulators() == 1);
  auto originStepper = *(task->originalIVAttrs->getAccumulators().begin());
  auto innerStepper = task->instructionClones[originStepper];
  bool stepIndexIs0 = isa<ConstantInt>(innerStepper->getOperand(0));
  bool stepIndexIs1 = isa<ConstantInt>(innerStepper->getOperand(1));
  assert(stepIndexIs0 || stepIndexIs1);
  innerStepper->setOperand(
    stepIndexIs0 ? 0 : 1,
    ConstantInt::get(PHIType, 1)
  );

  /*
   * Create new, composite induction variable for inner loop
   * Insert that composite variable after the inner loop PHI
   */
  auto iIter = innerHeader->begin();
  while (isa<PHINode>(&*(iIter++)));
  IRBuilder<> headerBuilder(&*(--iIter));
  auto sumIV = (Instruction *)headerBuilder.CreateAdd(
    task->cloneOfOriginalIV,
    task->outermostLoopIV
  );

  /*
   * Fetch the single PHI of the induction variable.
   */
  auto singlePHI = task->originalIVAttrs->getSinglePHI();
  assert(singlePHI != nullptr);

  /*
   * Replace uses of the induction variable
   * (not including uses in the header or by the stepper instruction
   */
  for (auto &use : singlePHI->uses()) {
    auto cloneUser = (Instruction *)use.getUser();
    if (task->instructionClones.find(cloneUser) == task->instructionClones.end()) continue;
    auto cloneI = task->instructionClones[cloneUser];
    if (cloneI == innerStepper || cloneI->getParent() == innerHeader) continue;
    // NOTE: The replacement is from the ORIGINAL PHI IV to the sum, not the clone to the sum
    //  This horrendous incongruency is because this function acts before data flow is adjusted
    //  Once that is changed, this can be made symmetric
    ((User *)cloneI)->replaceUsesOfWith(singlePHI, sumIV);
  }

  /*
   * Replace inner loop condition with less than total loop size condition
   */
  task->cloneOfOriginalCmp->setPredicate(CmpInst::Predicate::ICMP_ULT);
  task->cloneOfOriginalCmp->setOperand(0, sumIV);
  task->cloneOfOriginalCmp->setOperand(1, task->clonedIVBounds.cmpIVTo);

  /*
   * Add a condition to check that the IV is less than chunk size
   */
  auto castChunkSize = entryBuilder.CreateZExtOrTrunc(
    task->chunkSizeArg,
    task->cloneOfOriginalIV->getType()
  );
  headerBuilder.SetInsertPoint(task->cloneOfOriginalBr);
  Value *chunkCmp = headerBuilder.CreateICmpULT(task->cloneOfOriginalIV, castChunkSize);

  /*
   * Ensure both above conditions are met, that the inner loop IV is within bounds
   */
  Value *inBoundsIV = headerBuilder.CreateBinOp(Instruction::And, chunkCmp, task->cloneOfOriginalCmp);

  /*
   * Get the entry block into the loop body
   */
  auto loopBodyIndexIs0 = task->cloneOfOriginalBr->getSuccessor(0) != LDI->loopExitBlocks[0];
  auto loopBodyIndexIs1 = task->cloneOfOriginalBr->getSuccessor(1) != LDI->loopExitBlocks[0];
  assert(loopBodyIndexIs0 || loopBodyIndexIs1);
  auto innerBodyBB = task->cloneOfOriginalBr->getSuccessor(loopBodyIndexIs0 ? 0 : 1);

  /*
   * Revise branch to go to the loop body if the IV is in bounds,
   * and to the outer loop latch if not
   */
  task->cloneOfOriginalBr->setCondition(inBoundsIV);
  task->cloneOfOriginalBr->setSuccessor(0, task->basicBlockClones[innerBodyBB]);
  task->cloneOfOriginalBr->setSuccessor(1, task->outermostLoopLatch);

  /*
   * Finally, define branch from entry to outer loop
   */
  entryBuilder.CreateBr(task->outermostLoopHeader);
}

void DOALL::propagatePHINodesThroughOuterLoop (
  LoopDependenceInfo *LDI
) {
  auto task = (DOALLTask *)tasks[0];

  /*
   * Collect all PHIs (that aren't the IV)
   */
  auto innerHeader = task->basicBlockClones[LDI->header];
  std::set<PHINode *> phis;
  for (auto &I : *innerHeader) {
    if (!isa<PHINode>(&I)) break ;
    // Ignore the inner loop IV
    if (&I == task->cloneOfOriginalIV) continue ;
    phis.insert((PHINode *)&I);
  }

  /*
   * Create equivalent outer loop PHIs for these inner loop PHIs
   */
  IRBuilder<> outerBuilder(&*task->outermostLoopHeader->begin());
  std::unordered_map<PHINode *, PHINode *> innerToOuterPHI;
  for (auto phi : phis) {
    auto outerPHI = outerBuilder.CreatePHI(phi->getType(), /*numReservedValues=*/2);
    innerToOuterPHI[phi] = outerPHI;

    /*
     * Adjust inner loop PHI to have the outer loop header as an incoming block
     *  instead of the entry block (as the pre header mapped to the entry block)
     */
    auto innerEntryIndex = phi->getBasicBlockIndex(task->entryBlock);
    phi->setIncomingBlock(innerEntryIndex, task->outermostLoopHeader);

    /*
     * Route the inner PHI and outer PHI values
     */
    auto startVal = phi->getIncomingValue(innerEntryIndex);
    outerPHI->addIncoming(startVal, task->entryBlock);
    outerPHI->addIncoming(phi, task->outermostLoopLatch);
    phi->setIncomingValue(innerEntryIndex, outerPHI);
  }

  /*
   * Associate live-out values with their new outer PHI counterpart instead of
   * the inner PHI value so that when the the outer PHI is stored into the
   * environment, as it is the most up-to-date value
   */
  for (auto envIndex : LDI->environment->getEnvIndicesOfLiveOutVars()) {
    auto originalProducer = LDI->environment->producerAt(envIndex);
    assert(isa<Instruction>(originalProducer));
    auto originalProducerI = cast<Instruction>(originalProducer);
    auto clonedProducer = task->instructionClones[originalProducerI];

    /*
     * NOTE: In the case of DOALL, all live out variables must be reducible
     * All reducible variables are represented with PHIs in DOALL structured
     * loops, so the PHI would have been mapped into the outer loop.
     */
    assert(isa<PHINode>(clonedProducer));
    auto clonedPHI = cast<PHINode>(clonedProducer);
    assert(innerToOuterPHI.find(clonedPHI) != innerToOuterPHI.end());

    /*
     * Remap clone of environment producer from the inner to the outer PHI
     */
    auto outerPHI = innerToOuterPHI[clonedPHI];
    task->instructionClones[originalProducerI] = outerPHI;
  }
}
