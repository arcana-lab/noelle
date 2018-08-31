#include "DOALL.hpp"

void DOALL::reproducePreEnv (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker
) {

  IRBuilder<> entryB(chunker->entryBlock);
  for (auto envInd : LDI->environment->getPreEnvIndices()) {
    auto producer = LDI->environment->producerAt(envInd);
    auto envIndV = cast<Value>(ConstantInt::get(par.int64, envInd));
    auto envPtr = entryB.CreateInBoundsGEP(
      LDI->envArray,
      ArrayRef<Value*>({
        chunker->zeroV,
        envIndV
      })
    );
    auto prodPtr = entryB.CreateBitCast(
      entryB.CreateLoad(envPtr),
      PointerType::getUnqual(producer->getType())
    );
    chunker->preEnvMap[producer] = cast<Value>(entryB.CreateLoad(prodPtr));
  }
}

void DOALL::storePostEnvironment (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker
) {

  IRBuilder<> entryB(chunker->entryBlock->getTerminator());
  for (auto envInd : LDI->environment->getPostEnvIndices()) {
    auto producer = LDI->environment->producerAt(envInd);
    assert(isa<PHINode>(producer));

    auto envIndV = cast<Value>(ConstantInt::get(par.int64, envInd));
    auto envPtr = entryB.CreateInBoundsGEP(LDI->envArray, ArrayRef<Value*>({ chunker->zeroV, envIndV }));
    auto reduceArr = entryB.CreateBitCast(
      entryB.CreateLoad(envPtr),
      PointerType::getUnqual(ArrayType::get(PointerType::getUnqual(par.int8), NUM_CORES))
    );
    auto reduceArrPtr = entryB.CreateInBoundsGEP(reduceArr, ArrayRef<Value*>({ chunker->zeroV, chunker->coreArgVal }));
    auto reducePtr = entryB.CreateBitCast(
      entryB.CreateLoad(reduceArrPtr),
      PointerType::getUnqual(producer->getType())
    );

    // Ignore initial value of accumulation PHI, use binary op's identity value
    Value *initVal = nullptr;
    unordered_map<unsigned, unsigned> binOpToIdentity = {
      { Instruction::Add, 0 },
      { Instruction::FAdd, 0 },
      { Instruction::Sub, 0 },
      { Instruction::FSub, 0 },
      { Instruction::Mul, 1 },
      { Instruction::FMul, 1 }
    };
    for (auto iNodePair : LDI->loopSCCDAG->sccOfValue(producer)->internalNodePairs()) {
      auto I = cast<Instruction>(iNodePair.first);
      if (isa<PHINode>(I)) continue;
      assert(binOpToIdentity.find(I->getOpcode()) != binOpToIdentity.end());
      initVal = ConstantInt::get(producer->getType(), binOpToIdentity[I->getOpcode()]);
      break;
    }
    entryB.CreateStore(initVal, reducePtr);

    // Store final value of accumulation PHI
    auto prodClone = cast<PHINode>(chunker->innerValMap[(Instruction*)producer]);
    auto innerExitBB = chunker->innerBBMap[LDI->loopExitBlocks[0]];
    IRBuilder<> exitingBuilder(innerExitBB->getTerminator());
    exitingBuilder.CreateStore(prodClone, reducePtr);

    // Consolidate accumulator in outer loop
    auto initValPHIIndex = prodClone->getBasicBlockIndex(chunker->innerBBMap[LDI->preHeader]);
    IRBuilder<> chHeaderB(chunker->chHeader);
    chHeaderB.SetInsertPoint(&*chHeaderB.GetInsertBlock()->begin());
    auto accumOuterPHI = chHeaderB.CreatePHI(prodClone->getType(), 2);
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
    producer->print(errs() << "Producer: "); errs() << "\n";
    auto envIndV = cast<Value>(ConstantInt::get(par.int64, envInd));
    auto envPtr = reduceBuilder.CreateInBoundsGEP(
      LDI->envArray,
      ArrayRef<Value*>({
        chunker->zeroV,
        envIndV
      })
    );
    auto ptrTy_int8 = PointerType::getUnqual(par.int8);
    auto reduceArr = reduceBuilder.CreateBitCast(
      reduceBuilder.CreateLoad(envPtr),
      PointerType::getUnqual(ArrayType::get(ptrTy_int8, NUM_CORES))
    );

    // REFACTOR(angelo): query for binary operator done in multiple places
    auto producerSCC = LDI->loopSCCDAG->sccOfValue(producer);
    Instruction::BinaryOps binOp;
    for (auto iNodePair : LDI->loopSCCDAG->sccOfValue(producer)->internalNodePairs()) {
      auto I = cast<Instruction>(iNodePair.first);
      if (isa<PHINode>(I)) continue;
      binOp = static_cast<Instruction::BinaryOps>(I->getOpcode());
      break;
    }
    // HACK(angelo): Reducing negation is still done using addition
    if (binOp == Instruction::Sub) {
      binOp = Instruction::Add;
    } else if (binOp == Instruction::FSub) {
      binOp = Instruction::FAdd;
    }

    Value *accumVal = nullptr;
    for (auto i = 0; i < NUM_CORES; ++i) {
      auto indVal = cast<Value>(ConstantInt::get(par.int64, i));
      auto reduceArrPtr = reduceBuilder.CreateInBoundsGEP(reduceArr, ArrayRef<Value*>({ chunker->zeroV, indVal }));
      auto reducePtr = reduceBuilder.CreateBitCast(
        reduceBuilder.CreateLoad(reduceArrPtr),
        PointerType::getUnqual(producer->getType())
      );
      auto reduceVal = reduceBuilder.CreateLoad(reducePtr);
      accumVal = accumVal 
        ? reduceBuilder.CreateBinOp(binOp, accumVal, reduceVal)
        : reduceVal;
      accumVal->print(errs() << "Accum val after: "); errs() << "\n";
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

Value *DOALL::createEnvArray (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker,
  IRBuilder<> entryBuilder,
  IRBuilder<> parBuilder
) {

  auto storeEnvAllocaInArray = [&](Value *arr, int envIndex, AllocaInst *alloca) -> void {
    arr->print(errs() << "Index " << envIndex << ", Array: "); errs() << "\n";
    alloca->print(errs() << "Alloca "); errs() << "\n";
    auto indValue = cast<Value>(ConstantInt::get(par.int64, envIndex));
    auto envPtr = entryBuilder.CreateInBoundsGEP(arr, ArrayRef<Value*>({ chunker->zeroV, indValue }));
    auto depCast = entryBuilder.CreateBitCast(envPtr, PointerType::getUnqual(alloca->getType()));
    auto store = entryBuilder.CreateStore(alloca, depCast);
    store->print(errs() << "Store "); errs() << "\n\n";
  };

  /*
   * Create empty environment array for producers, exit block tracking
   */
  for (auto envIndex : LDI->environment->getPreEnvIndices()) {
    LDI->environment->producerAt(envIndex)->print(errs() << "Producer "); errs() << "\n";
    Type *envType = LDI->environment->typeOfEnv(envIndex);
    auto varAlloca = entryBuilder.CreateAlloca(envType);

    storeEnvAllocaInArray(LDI->envArray, envIndex, varAlloca);

    /*
     * Insert pre-loop producers into the environment array
     */
    parBuilder.CreateStore(LDI->environment->producerAt(envIndex), varAlloca);
  }
  for (auto envIndex : LDI->environment->getPostEnvIndices()) {
    LDI->environment->producerAt(envIndex)->print(errs() << "Producer "); errs() << "\n";
    auto reduceArrType = ArrayType::get(PointerType::getUnqual(par.int8), NUM_CORES);
    auto reduceArrAlloca = entryBuilder.CreateAlloca(reduceArrType);

    storeEnvAllocaInArray(LDI->envArray, envIndex, reduceArrAlloca);

    Type *envType = LDI->environment->typeOfEnv(envIndex);
    for (auto i = 0; i < NUM_CORES; ++i) {
      auto varAlloca = entryBuilder.CreateAlloca(envType);

      storeEnvAllocaInArray(reduceArrAlloca, i, varAlloca);
    }
  }
  
  return cast<Value>(parBuilder.CreateBitCast(LDI->envArray, PointerType::getUnqual(par.int8)));
}

