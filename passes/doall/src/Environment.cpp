#include "DOALL.hpp"

void DOALL::reproducePreEnv (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker
) {
  auto envUser = LDI->envBuilder->getUser(0);
  IRBuilder<> entryB(chunker->entryBlock);
  for (auto envInd : LDI->environment->getPreEnvIndices()) {
    envUser->createEnvPtr(entryB, envInd);
    auto envLoad = entryB.CreateLoad(envUser->getEnvPtr(envInd));
    auto producer = LDI->environment->producerAt(envInd);
    chunker->preEnvMap[producer] = cast<Value>(envLoad);
  }
}

void DOALL::storePostEnvironment (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker
) {
  auto envUser = LDI->envBuilder->getUser(0);
  IRBuilder<> entryB(chunker->entryBlock->getTerminator());
  for (auto envInd : LDI->environment->getPostEnvIndices()) {
    envUser->createReducableEnvPtr(entryB, envInd, NUM_CORES, chunker->coreArgVal);
    auto envPtr = envUser->getEnvPtr(envInd);

    auto producer = LDI->environment->producerAt(envInd);
    assert(isa<PHINode>(producer));

    // Ignore initial value of accumulation PHI, use binary op's identity value
    auto producerSCC = LDI->loopSCCDAG->sccOfValue(producer);
    auto firstAccumI = *(LDI->sccdagAttrs.getSCCAttrs(producerSCC)->PHIAccumulators.begin());
    auto &opToIdentity = LDI->sccdagAttrs.accumOpInfo.opIdentities;
    auto opIdentity = opToIdentity[firstAccumI->getOpcode()];
    Value *initVal = nullptr;
    Type *accumTy = producer->getType();
    if (accumTy->isIntegerTy()) initVal = ConstantInt::get(accumTy, opIdentity);
    if (accumTy->isFloatTy()) initVal = ConstantFP::get(accumTy, (float)opIdentity);
    if (accumTy->isDoubleTy()) initVal = ConstantFP::get(accumTy, (double)opIdentity);
    assert(initVal != nullptr);
    entryB.CreateStore(initVal, envPtr);

    // Store final value of accumulation PHI
    auto prodClone = cast<PHINode>(chunker->innerValMap[(Instruction*)producer]);
    auto innerExitBB = chunker->innerBBMap[LDI->loopExitBlocks[0]];
    IRBuilder<> exitingBuilder(innerExitBB->getTerminator());
    exitingBuilder.CreateStore(prodClone, envPtr);

    // Consolidate accumulator in outer loop
    auto initValPHIIndex = prodClone->getBasicBlockIndex(chunker->innerBBMap[LDI->preHeader]);
    IRBuilder<> chHeaderB(chunker->chHeader);
    chHeaderB.SetInsertPoint(&*chHeaderB.GetInsertBlock()->begin());
    auto accumOuterPHI = chHeaderB.CreatePHI(initVal->getType(), 2);

    accumOuterPHI->addIncoming(initVal, chunker->entryBlock);
    accumOuterPHI->addIncoming(prodClone, innerExitBB);

    prodClone->setIncomingValue(initValPHIIndex, accumOuterPHI);
  }
}

void DOALL::reducePostEnvironment (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker
) {
  auto &cxt = LDI->function->getContext();
  IRBuilder<> reduceBuilder(LDI->exitPointOfParallelizedLoop);

  for (auto envInd : LDI->environment->getPostEnvIndices()) {
    auto producer = LDI->environment->producerAt(envInd);
    auto producerSCC = LDI->loopSCCDAG->sccOfValue(producer);
    auto firstAccumI = *(LDI->sccdagAttrs.getSCCAttrs(producerSCC)->PHIAccumulators.begin());
    auto binOpCode = firstAccumI->getOpcode();
    binOpCode = LDI->sccdagAttrs.accumOpInfo.accumOpForType(binOpCode, producer->getType());
    auto binOp = (Instruction::BinaryOps)binOpCode;

    Value *accumVal = reduceBuilder.CreateLoad(LDI->envBuilder->getReducableEnvVar(envInd, 0));
    for (auto i = 1; i < NUM_CORES; ++i) {
      auto envVar = reduceBuilder.CreateLoad(LDI->envBuilder->getReducableEnvVar(envInd, i));
      accumVal = reduceBuilder.CreateBinOp(binOp, accumVal, envVar);
    }

    auto prodPHI = cast<PHINode>(producer);
    auto initValPHIIndex = prodPHI->getBasicBlockIndex(LDI->preHeader);
    auto initVal = prodPHI->getIncomingValue(initValPHIIndex);
    accumVal = reduceBuilder.CreateBinOp(binOp, accumVal, initVal);

    for (auto consumer : LDI->environment->consumersOf(producer)) {
      if (auto depPHI = dyn_cast<PHINode>(consumer)) {
        depPHI->addIncoming(accumVal, LDI->exitPointOfParallelizedLoop);
        continue;
      }
      producer->print(errs() << "Producer of environment variable:\t"); errs() << "\n";
      errs() << "Loop not in LCSSA!\n";
      abort();
    }
  }
}
