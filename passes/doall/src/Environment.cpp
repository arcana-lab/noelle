#include "DOALL.hpp"

void DOALL::generateCodeToLoadAllLiveInVariables (
  LoopDependenceInfoForParallelizer *LDI,
  std::unique_ptr<ChunkerInfo> &chunker
) {

  /*
   * Define the function to use.
   */
  auto mapFunction = [&chunker](Value *originalProducer, Value *generatedLoad){
    chunker->preEnvMap[originalProducer] = cast<Value>(generatedLoad);
  };

  /*
   * Generate loads to load live-in variables.
   */
  this->generateCodeToLoadAllLiveInVariables(LDI, chunker->entryBlock, mapFunction);
}

void DOALL::storePostEnvironment (
  LoopDependenceInfoForParallelizer *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker
) {
  auto envUser = envBuilder->getUser(0);
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
