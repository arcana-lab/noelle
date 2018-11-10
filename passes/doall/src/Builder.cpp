#include "DOALL.hpp"

void DOALL::generateOuterLoopAndAdjustInnerLoop (
  LoopDependenceInfoForParallelizer *LDI
){
  auto worker = (DOALLTechniqueWorker *)workers[0];

  /*
   * Determine start value and step size for outer loop IV
   */
  IRBuilder<> entryBuilder(worker->entryBlock);
  auto startOfIV = worker->clonedIVInfo.start;
  auto nthCoreOffset = entryBuilder.CreateZExtOrTrunc(
    entryBuilder.CreateMul(worker->coreArg, worker->chunkSizeArg),
    startOfIV->getType()
  );
  auto outerIVStartVal = entryBuilder.CreateAdd(startOfIV, nthCoreOffset);

  /*
   * Since the step size of the original IV is simplified to be +1,
   * the offset of chunk size * num cores is the entire step size
   */
  auto numCoresOffset = entryBuilder.CreateZExtOrTrunc(
    entryBuilder.CreateMul(worker->numCoresArg, worker->chunkSizeArg),
    startOfIV->getType()
  );

  /*
   * Generate outer loop header, latch, PHI, and stepper
   */
  auto &cxt = worker->F->getContext();
  worker->outerHeader = BasicBlock::Create(cxt, "", worker->F);
  worker->outerLatch = BasicBlock::Create(cxt, "", worker->F);
  IRBuilder<> outerHBuilder(worker->outerHeader);
  IRBuilder<> outerLBuilder(worker->outerLatch);
  worker->outerIV = outerHBuilder.CreatePHI(startOfIV->getType(), /*numReservedValues=*/2);
  auto outerIVStepper = outerLBuilder.CreateAdd(worker->outerIV, numCoresOffset);
  outerLBuilder.CreateBr(worker->outerHeader);

  /*
   * Create outer loop IV lifecycle
   */
  worker->outerIV->addIncoming(outerIVStartVal, worker->entryBlock);
  worker->outerIV->addIncoming(outerIVStepper, worker->outerLatch);

  /*
   * Upon simplifying the original loop IV, the cmp to value was set to
   * the exclusive upper bound, hence the unsigned less than comparison
   */
  auto outerIVCmp = outerHBuilder.CreateICmpULT(
    worker->outerIV,
    worker->clonedIVInfo.cmpIVTo
  );
  auto innerHeader = worker->basicBlockClones[LDI->header];
  outerHBuilder.CreateCondBr(outerIVCmp, innerHeader, worker->loopExitBlocks[0]);

  /*
   * Reset inner loop start value to 0
   */
  auto PHIType = worker->originalIVClone->getType();
  auto startValueIndex = -1;
  bool entryIndexIs0 = worker->originalIVClone->getIncomingBlock(0) == LDI->preHeader;
  bool entryIndexIs1 = worker->originalIVClone->getIncomingBlock(1) == LDI->preHeader;
  assert(entryIndexIs0 || entryIndexIs1);
  worker->originalIVClone->setIncomingValue(
    entryIndexIs0 ? 0 : 1,
    ConstantInt::get(PHIType, 0)
  );

  /*
   * NOTE(angelo): Also map entry block to outer loop header
   *  this should be done for all PHIs in the inner loop at the same time
   *  to avoid code duplication. See API propagatePHINodesThroughOuterLoop
   */
  worker->originalIVClone->setIncomingBlock(
    entryIndexIs0 ? 0 : 1,
    worker->outerHeader
  );

  /*
   * Revise latch stepper instruction to increment
   */
  auto &accumulators = worker->originalIVAttrs->PHIAccumulators;
  assert(accumulators.size() == 1);
  auto originStepper = *(accumulators.begin());
  auto innerStepper = worker->instructionClones[originStepper];
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
    worker->originalIVClone,
    worker->outerIV
  );

  /*
   * Replace uses of the induction variable
   * (not including uses in the header or by the stepper instruction
   */
  for (auto &use : worker->originalIVAttrs->singlePHI->uses()) {
    auto cloneUser = (Instruction *)use.getUser();
    cloneUser->print(errs() << "Potential user: "); errs() << "\n";
    if (worker->instructionClones.find(cloneUser) == worker->instructionClones.end()) continue;
    auto cloneI = worker->instructionClones[cloneUser];
    cloneI->print(errs() << " Is user: "); errs() << "\n";
    if (cloneI == innerStepper || cloneI->getParent() == innerHeader) continue;
    // NOTE: The replacement is from the ORIGINAL PHI IV to the sum, not the clone to the sum
    //  This horrendous incongruency is because this function acts before data flow is adjusted
    //  Once that is changed, this can be made symmetric
    ((User *)cloneI)->replaceUsesOfWith(worker->originalIVAttrs->singlePHI, sumIV);
  }

  /*
   * Replace inner loop condition with less than total loop size condition
   */
  auto innerCmp = worker->clonedIVInfo.cmp;
  innerCmp->setPredicate(CmpInst::Predicate::ICMP_ULT);
  innerCmp->setOperand(0, sumIV);
  innerCmp->setOperand(1, worker->clonedIVInfo.cmpIVTo);

  /*
   * Add a condition to check that the IV is less than chunk size
   */
  auto castChunkSize = entryBuilder.CreateZExtOrTrunc(
    worker->chunkSizeArg,
    worker->originalIVClone->getType()
  );
  auto innerBr = worker->clonedIVInfo.br;
  headerBuilder.SetInsertPoint(innerBr);
  Value *chunkCmp = headerBuilder.CreateICmpULT(worker->originalIVClone, castChunkSize);

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
  innerBr->setSuccessor(0, worker->basicBlockClones[innerBodyBB]);
  innerBr->setSuccessor(1, worker->outerLatch);

  /*
   * Finally, define branch from entry to outer loop
   */
  entryBuilder.CreateBr(worker->outerHeader);
}

void DOALL::propagatePHINodesThroughOuterLoop (
  LoopDependenceInfoForParallelizer *LDI
) {
  auto worker = (DOALLTechniqueWorker *)workers[0];

  /*
   * Collect all PHIs (that aren't the IV)
   */
  auto innerHeader = worker->basicBlockClones[LDI->header];
  std::set<PHINode *> phis;
  for (auto &I : *innerHeader) {
    if (!isa<PHINode>(&I)) break ;
    // Ignore the inner loop IV
    if (&I == worker->originalIVClone) continue ;
    phis.insert((PHINode *)&I);
  }

  /*
   * Create equivalent outer loop PHIs for these inner loop PHIs
   */
  IRBuilder<> outerBuilder(&*worker->outerHeader->begin());
  for (auto phi : phis) {
    auto outerPHI = outerBuilder.CreatePHI(phi->getType(), /*numReservedValues=*/2);

    /*
     * Adjust inner loop PHI to have the outer loop header as an incoming block
     *  instead of the entry block (as the pre header mapped to the entry block)
     */
    auto innerEntryIndex = phi->getBasicBlockIndex(worker->entryBlock);
    phi->setIncomingBlock(innerEntryIndex, worker->outerHeader);

    /*
     * If incoming is pre header, use initial value of inner PHI
     * Otherwise, route from outer loop latch using current value of inner PHI
     * As for the inner loop PHI, replace initial value with outer header PHI
     */
    auto startVal = phi->getIncomingValue(innerEntryIndex);
    outerPHI->addIncoming(startVal, worker->entryBlock);
    outerPHI->addIncoming(phi, worker->outerLatch);
    phi->setIncomingValue(innerEntryIndex, outerPHI);
  }
}
