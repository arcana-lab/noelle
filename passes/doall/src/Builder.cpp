#include "DOALL.hpp"

void DOALL::generateOuterLoopAndAdjustInnerLoop (
  LoopDependenceInfoForParallelizer *LDI
){
  auto task = (DOALLTaskExecution *)tasks[0];

  /*
   * Determine start value and step size for outer loop IV
   */
  IRBuilder<> entryBuilder(task->entryBlock);
  auto startOfIV = task->clonedIVInfo.start;
  auto nthCoreOffset = entryBuilder.CreateZExtOrTrunc(
    entryBuilder.CreateMul(task->coreArg, task->chunkSizeArg),
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
    task->clonedIVInfo.cmpIVTo
  );
  auto innerHeader = task->basicBlockClones[LDI->header];
  outerHBuilder.CreateCondBr(outerIVCmp, innerHeader, task->loopExitBlocks[0]);

  /*
   * Reset inner loop start value to 0
   */
  auto PHIType = task->originalIVClone->getType();
  auto startValueIndex = -1;
  bool entryIndexIs0 = task->originalIVClone->getIncomingBlock(0) == LDI->preHeader;
  bool entryIndexIs1 = task->originalIVClone->getIncomingBlock(1) == LDI->preHeader;
  assert(entryIndexIs0 || entryIndexIs1);
  task->originalIVClone->setIncomingValue(
    entryIndexIs0 ? 0 : 1,
    ConstantInt::get(PHIType, 0)
  );

  /*
   * NOTE(angelo): Also map entry block to outer loop header
   *  this should be done for all PHIs in the inner loop at the same time
   *  to avoid code duplication. See API propagatePHINodesThroughOuterLoop
   */
  task->originalIVClone->setIncomingBlock(
    entryIndexIs0 ? 0 : 1,
    task->outermostLoopHeader
  );

  /*
   * Revise latch stepper instruction to increment
   */
  auto &accumulators = task->originalIVAttrs->PHIAccumulators;
  assert(accumulators.size() == 1);
  auto originStepper = *(accumulators.begin());
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
    task->originalIVClone,
    task->outermostLoopIV
  );

  /*
   * Replace uses of the induction variable
   * (not including uses in the header or by the stepper instruction
   */
  for (auto &use : task->originalIVAttrs->singlePHI->uses()) {
    auto cloneUser = (Instruction *)use.getUser();
    cloneUser->print(errs() << "Potential user: "); errs() << "\n";
    if (task->instructionClones.find(cloneUser) == task->instructionClones.end()) continue;
    auto cloneI = task->instructionClones[cloneUser];
    cloneI->print(errs() << " Is user: "); errs() << "\n";
    if (cloneI == innerStepper || cloneI->getParent() == innerHeader) continue;
    // NOTE: The replacement is from the ORIGINAL PHI IV to the sum, not the clone to the sum
    //  This horrendous incongruency is because this function acts before data flow is adjusted
    //  Once that is changed, this can be made symmetric
    ((User *)cloneI)->replaceUsesOfWith(task->originalIVAttrs->singlePHI, sumIV);
  }

  /*
   * Replace inner loop condition with less than total loop size condition
   */
  auto innerCmp = task->clonedIVInfo.cmp;
  innerCmp->setPredicate(CmpInst::Predicate::ICMP_ULT);
  innerCmp->setOperand(0, sumIV);
  innerCmp->setOperand(1, task->clonedIVInfo.cmpIVTo);

  /*
   * Add a condition to check that the IV is less than chunk size
   */
  auto castChunkSize = entryBuilder.CreateZExtOrTrunc(
    task->chunkSizeArg,
    task->originalIVClone->getType()
  );
  auto innerBr = task->clonedIVInfo.br;
  headerBuilder.SetInsertPoint(innerBr);
  Value *chunkCmp = headerBuilder.CreateICmpULT(task->originalIVClone, castChunkSize);

  /*
   * Ensure both above conditions are met, that the inner loop IV is within bounds
   */
  Value *inBoundsIV = headerBuilder.CreateBinOp(Instruction::And, chunkCmp, innerCmp);

  /*
   * Get the entry block into the loop body
   */
  auto loopBodyIndexIs0 = innerBr->getSuccessor(0) != LDI->loopExitBlocks[0];
  auto loopBodyIndexIs1 = innerBr->getSuccessor(1) != LDI->loopExitBlocks[0];
  assert(loopBodyIndexIs0 || loopBodyIndexIs1);
  auto innerBodyBB = innerBr->getSuccessor(loopBodyIndexIs0 ? 0 : 1);

  /*
   * Revise branch to go to the loop body if the IV is in bounds,
   * and to the outer loop latch if not
   */
  innerBr->setCondition(inBoundsIV);
  innerBr->setSuccessor(0, task->basicBlockClones[innerBodyBB]);
  innerBr->setSuccessor(1, task->outermostLoopLatch);

  /*
   * Finally, define branch from entry to outer loop
   */
  entryBuilder.CreateBr(task->outermostLoopHeader);
}

void DOALL::propagatePHINodesThroughOuterLoop (
  LoopDependenceInfoForParallelizer *LDI
) {
  auto task = (DOALLTaskExecution *)tasks[0];

  /*
   * Collect all PHIs (that aren't the IV)
   */
  auto innerHeader = task->basicBlockClones[LDI->header];
  std::set<PHINode *> phis;
  for (auto &I : *innerHeader) {
    if (!isa<PHINode>(&I)) break ;
    // Ignore the inner loop IV
    if (&I == task->originalIVClone) continue ;
    phis.insert((PHINode *)&I);
  }

  /*
   * Create equivalent outer loop PHIs for these inner loop PHIs
   */
  IRBuilder<> outerBuilder(&*task->outermostLoopHeader->begin());
  for (auto phi : phis) {
    auto outerPHI = outerBuilder.CreatePHI(phi->getType(), /*numReservedValues=*/2);

    /*
     * Adjust inner loop PHI to have the outer loop header as an incoming block
     *  instead of the entry block (as the pre header mapped to the entry block)
     */
    auto innerEntryIndex = phi->getBasicBlockIndex(task->entryBlock);
    phi->setIncomingBlock(innerEntryIndex, task->outermostLoopHeader);

    /*
     * If incoming is pre header, use initial value of inner PHI
     * Otherwise, route from outer loop latch using current value of inner PHI
     * As for the inner loop PHI, replace initial value with outer header PHI
     */
    auto startVal = phi->getIncomingValue(innerEntryIndex);
    outerPHI->addIncoming(startVal, task->entryBlock);
    outerPHI->addIncoming(phi, task->outermostLoopLatch);
    phi->setIncomingValue(innerEntryIndex, outerPHI);
  }
}
