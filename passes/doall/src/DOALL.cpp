#include "DOALL.hpp"

// TODO(angelo): replace with values passed into this library
#define NUM_CORES 4
#define CHUNK_SIZE 8

using namespace llvm;

DOALL::DOALL (Module &module, Verbosity v)
  :
  module{module},
  verbose{v}
  {

  this->doallDispatcher = this->module.getFunction("doallDispatcher");

  return ;
}

bool DOALL::apply (LoopDependenceInfo *LDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) {
  errs() << "DOALL: Start\n";

  auto chunker = this->createChunkingFuncAndArgs(LDI, par);

  /*
   * Create some utility values
   */
  chunker->zeroV = cast<Value>(ConstantInt::get(par.int64, 0));

  this->reproduceOriginLoop(LDI, par, chunker);
  this->reproducePreEnv(LDI, par, chunker);
  this->mapOriginLoopValueUses(LDI, par, chunker);
  this->collectOriginIVValues(LDI, par, chunker);
  this->createOuterLoop(LDI, par, chunker);
  this->alterInnerLoopToIterateChunks(LDI, par, chunker);

  IRBuilder<> exitB(exitBlock);
  exitB.CreateRetVoid();

  /*
   * Store post environment reducable values
   */
  entryB.SetInsertPoint(entryBlock->getTerminator());
  for (auto envInd : LDI->environment->getPostEnvIndices()) {
    auto producer = LDI->environment->producerAt(envInd);
    auto envIndV = cast<Value>(ConstantInt::get(par.int64, envInd));
    auto envPtr = entryB.CreateInBoundsGEP(LDI->envArray, ArrayRef<Value*>({ zeroV, envIndV }));
    auto reduceArr = entryB.CreateBitCast(
      entryB.CreateLoad(envPtr),
      PointerType::getUnqual(ArrayType::get(PointerType::getUnqual(par.int8), NUM_CORES))
    );
    auto reduceArrPtr = entryB.CreateInBoundsGEP(reduceArr, ArrayRef<Value*>({ zeroV, coreVal }));
    auto reducePtr = entryB.CreateBitCast(
      entryB.CreateLoad(reduceArrPtr),
      PointerType::getUnqual(producer->getType())
    );

    // Store initial value of accumulation PHI
    assert(isa<PHINode>(producer));
    auto prodClone = cast<PHINode>(instrArgMap[producer]);
    auto initValPHIIndex = prodClone->getBasicBlockIndex(innerBBMap[LDI->preHeader]);
    auto initVal = prodClone->getIncomingValue(initValPHIIndex);
    if (auto constVal = dyn_cast<ConstantInt>(initVal)) {
      initVal = ConstantInt::get(prodClone->getType(), constVal->getValue());
    } 
    entryB.CreateStore(initVal, reducePtr);

    // Store final value of accumulation PHI
    auto innerExitBB = innerBBMap[LDI->loopExitBlocks[0]];
    IRBuilder<> exitingBuilder(innerExitBB->getTerminator());
    exitingBuilder.CreateStore(prodClone, reducePtr);

    // Consolidate accumulator in outer loop
    chHeaderB.SetInsertPoint(&*chHeaderB.GetInsertBlock()->begin());
    auto accumOuterPHI = chHeaderB.CreatePHI(prodClone->getType(), 2);
    accumOuterPHI->addIncoming(initVal, entryBlock);
    accumOuterPHI->addIncoming(prodClone, innerExitBB);

    prodClone->setIncomingValue(initValPHIIndex, accumOuterPHI);
  }

  addChunkFunctionExecutionAsideOriginalLoop(LDI, par, chunker);

  chunker->print(errs() << "Finalized chunker:\n"); errs() << "\n";

  errs() << "DOALL: Exit\n";
  return true;
}

std::unique_ptr<ChunkerInfo> DOALL::createChunkingFuncAndArgs (
  LoopDependenceInfo *LDI,
  Parallelization &par
) {

  auto chunker = std::make_unique<ChunkerInfo>();
  auto M = LDI->function->getParent();
  auto &cxt = M->getContext();

  auto voidTy = Type::getVoitTy(cxt);
  auto ptrTy_int8 = PointerType::getUnqual(par.int8);
  auto funcArgTypes = ArrayRef<Type*>({
    ptrTy_int8,
    par.int64,
    par.int64,
    par.int64
  });
  auto funcTy = FunctionType::get(voidTy, funcArgTypes, false);
  chunker->f = cast<Function>(M->getOrInsertFunction("", funcTy));

  /*
   * Create entry and exit blocks
   * Create outer loop header and latch
   */
  chunker->entryBlock = chunker->createChunkerBB();
  chunker->exitBlock = chunker->createChunkerBB();
  chunker->chHeader = chunker->createChunkerBB();
  chunker->chLatch = chunker->createChunkerBB();

  /*
   * Collect arguments of chunker function
   */
  auto argIter = chunker->f->arg_begin();
  chunker->envArgVal = (Value *) &*(argIter++);
  chunker->coreArgVal = (Value *) &*(argIter++); 
  chunker->numCoresArgVal = (Value *) &*(argIter++);
  chunker->chunkSizeArgVal = (Value *) &*(argIter++);

  /*
   * Create environment context within chunking function
   */
  LDI->envArrayType = ArrayType::get(
    ptrTy_int8,
    LDI->environment->envSize()
  );
  IRBuilder<> entryB(chunker->entryBlock);
  LDI->envArray = entryB.CreateBitCast(
    chunker->envArgVal,
    PointerType::getUnqual(LDI->envArrayType)
  );

  return chunker;
}

void DOALL::reproduceOriginLoop (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker
) {

  /*
   * Create inner loop
   */
  for (auto originBB : LDI->liSummary.topLoop->bbs) {
    auto cloneBB = chunker->createChunkerBB();
    IRBuilder<> builder(cloneBB);
    chunker->innerBBMap[originBB] = cloneBB;
    for (auto &I : *originBB) {
      auto cloneI = builder.Insert(I.clone());
      chunker->innerValMap[&I] = cloneI;
    }
  }

  // Map inner loop preheader to outer loop header
  chunker->innerBBMap[LDI->preHeader] = chunker->chHeader;

  // Map single exit block of inner loop to outer loop latch
  chunker->innerBBMap[LDI->loopExitBlocks[0]] = chunker->chLatch;
}

void DOALL::reproducePreEnv (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker
) {
  IRBuilder<> entryB(chunker->entryBuilder);
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

void DOALL::mapOriginLoopValueUses (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker
) {
  for (auto iPair : chunker->innerValMap) {
    auto cloneI = iPair.second;
    if (auto terminator = dyn_cast<TerminatorInst>(cloneI)) {
      for (int i = 0; i < terminator->getNumSuccessors(); ++i) {
        auto succBB = terminator->getSuccessor(i);
        assert(chunker->innerBBMap.find(succBB) != chunker->innerBBMap.end());
        terminator->setSuccessor(i, chunker->innerBBMap[succBB]);
      }
      continue;
    }

    if (auto phi = dyn_cast<PHINode>(cloneI)) {
      for (int i = 0; i < phi->getNumIncomingValues(); ++i) {
        auto cloneBB = chunker->innerBBMap[phi->getIncomingBlock(i)];
        phi->setIncomingBlock(i, cloneBB);
      }
    }

    for (auto &op : cloneI->operands()) {
      auto opV = op.get();
      if (chunker->preEnvMap.find(opV) != chunker->preEnvMap.end()) {
        op.set(chunker->preEnvMap[opV]);
      }
      if (chunker->innerValMap.find(opV) != chunker->innerValMap.end()) {
        op.set(chunker->innerValMap[opV]);
      }
      // TODO(angelo): Add exhaustive search of types to parallelization
      // utilities for use in DSWP and here in DOALL
    }
  }
}

// TODO(angelo): Have LoopSummary scrape this info from IVs
void DOALL::collectOriginIVValues (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker
) {

  /*
   * Find origin loop's induction variable
   */
  auto originHeaderBr = LDI->header->getTerminator();
  assert(isa<BranchInst>(originHeaderBr));
  chunker->originHeaderBr = cast<BranchInst>(originHeaderBr);
  auto originCond = chunker->originHeaderBr->getCondition();
  assert(isa<CmpInst>(originCond));
  chunker->originCmp = cast<CmpInst>(originCond);

  int opIndex = 0;
  for (auto &condOp : cast<User>(chunker->originCmp)->operands()) {
    bool isPHI = isa<PHINode>(condOp);
    if (isPHI) {
      chunker->originCmpPHIIndex = opIndex;
      chunker->originIV = cast<PHINode>(condOp);
    } else {
      chunker->originCmpMaxIndex = opIndex;
      chunker->maxIV = condOp;
    }
    opIndex++;
  }
  assert(chunker->originIV != nullptr && chunker->maxIV != nullptr);

  /*
   * Determine step size of induction variable
   */
  for (auto user : chunker->originIV->users()) {
    auto scev = SE.getSCEV((Value *)user);
    switch (scev->getSCEVType()) {
    case scAddExpr:
    case scAddRecExpr:
      chunker->stepperIV = (Value *)user;
      Value *lhs = user->getOperand(0);
      Value *rhs = user->getOperand(1);
      if (isa<ConstantInt>(lhs)) {
        chunker->originStepSize = cast<ConstantInt>(lhs);
        chunker->stepSizeIVIndex = 0;
      } else if (isa<ConstantInt>(rhs)) {
        chunker->originStepSize = cast<ConstantInt>(rhs);
        chunker->stepSizeIVIndex = 1;
      } else continue;
      break;
    }
  }
  assert(chunker->stepperIV != nullptr && chunker->originStepSize != nullptr);

  /*
   * Get start value off of original loop IV
   */
  chunker->startValIVIndex = chunker->originIV->getBasicBlockIndex(LDI->preHeader);
  chunker-starterIV = chunker->originIV->getIncomingValue(chunker->startValIVIndex);
}

void DOALL::createOuterLoop (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker
) {
  IRBuilder<> entryB(chunker->entryBlock);
  IRBuilder<> chHeaderB(chunker->chHeader);
  IRBuilder<> chLatchB(chunker->chLatch);

  /*
   * Determine start value and step size for outer chunking loop
   */
  auto cloneStarterIV = chunker->starterIV;
  if (!isa<ConstantInt>(chunker->starterIV)) {
    cloneStarterIV = chunker->preEnvMap[chunker->starterIV];
  }
  auto chIVStart = entryB.CreateMul(chunker->coreArgVal, chunker->chunkSizeArgVal);
  chIVStart = entryB.CreateAdd(chIVStart, cloneStarterIV);

  // ASSUMPTION: Monotonically increasing IV
  auto chIVStepSize = entryB.CreateMul(chunker->numCoresArgVal, chunker->chunkSizeArgVal);
  chIVStepSize = entryB.CreateMul(chIVStepSize, chunker->originStepSize);
  auto chIVInc = chLatchB.CreateAdd(chIV, chIVStepSize);

  auto chIV = chHeaderB.CreatePHI(par.int64, /*numReservedValues=*/2);
  chIV->addIncoming(chIVStart, chunker->entryBlock);
  chIV->addIncoming(chIVInc, chunker->chLatch);

  entryB.CreateBr(chunker->chHeader);
  chLatchB.CreateBr(chHeader);

  Value *cloneMaxIV = chunker->maxIV;
  if (!isa<ConstantInt>(chunker->maxIV)) {
    cloneMaxIV = chunker->innerValMap[chunker->maxIV];
  }

  auto strictMaxIVPredicate = chunker->originCmp->getPredicate();
  auto chIVOnLeft = chunker->originCondPHIIndex == 0;

  // HACK: Make the condition stronger so that chunks don't skip over the equality condition
  if (strictMaxIVPredicate == CmpInst::Predicate::ICMP_EQ) {
    strictMaxIVPredicate = chIVOnLeft 
      ? CmpInst::Predicate::ICMP_UGE
      : CmpInst::Predicate::ICMP_ULE;
  }

  CmpInst *chCmp;
  if (chIVOnLeft) {
    chCmp = CmpInst::Create(originCmp->getOpcode(), strictMaxIVPredicate, chIV, cloneMaxIV);
  } else {
    chCmp = CmpInst::Create(originCmp->getOpcode(), strictMaxIVPredicate, cloneMaxIV, chIV);
  }

  chHeaderB.Insert(chCmp);
  auto innerHeader = chunker->innerBBMap[LDI->header];
  if (chunker->originHeaderBr->getSuccessor(0) == LDI->loopExitBlocks[0]) {
    chHeaderB.CreateCondBr(chCmp, chunker->exitBlock, innerHeader);
  } else {
    chHeaderB.CreateCondBr(chCmp, innerHeader, chunker->exitBlock);
  }
}

void DOALL::alterInnerLoopToIterateChunks (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker
) {

  /*
   * Reset start to 0, revise latch to only inc/dec IV
   */
  auto innerIV = chunker->innerValMap[chunker->originIV];
  ((PHINode *)innerIV)->setIncomingValue(
    chunker->startValIVIndex,
    ConstantInt::get(originIV->getType(), 0)
  );
  auto innerStepIV = (User *)chunker->innerValMap[chunker->stepperIV];
  innerStepIV->setOperand(
    chunker->stepSizeIVIndex,
    ConstantInt::get(chunker->stepperIV->getType(), 1)
  );

  /*
   * Create new, composite induction variable for inner loop
   */
  auto innerHeader = chunkerInfo->innerValMap[LDI->header];
  IRBuilder<> headerBuilder(innerHeader);
  // ASSUMPTION: Monotonically increasing IV
  auto innerOuterIVSum = headerBuilder.CreateAdd(innerIV, chIV);
  for (auto &use : originIV->uses()) {
    auto cloneI = chunker->innerValMap[(Instruction *)use.getUser()];
    auto cloneU = (User *)cloneI;
    if (cloneU == innerStepIV || cloneI->getParent() == innerHeader) continue;
    cloneU->replaceUsesOfWith(innerIV, innerOuterIVSum);
  }

  // HERE(angelo)

  /*
   * Replace inner loop original condition with less than total loop size condition
   * Add a cond to check for less than chunk size
   */
  auto innerCondIV = (User *)instrArgMap[originCond];
  // Ensure the add comes before its use in the comparison
  innerCondIV->setOperand(originCondPHIIndex, innerOuterIVSum);
  ((CmpInst *)innerCondIV)->setPredicate(stricterMaxIVCondPredicate);

  auto ivSumInst = cast<Instruction>(innerOuterIVSum);
  ivSumInst->removeFromParent();
  ivSumInst->insertBefore(cast<Instruction>(innerCondIV));

  auto chunkCondBB = BasicBlock::Create(cxt, "", chunker);
  IRBuilder<> chunkCondBBBuilder(chunkCondBB);

  auto chunkSizeCast = chunkSizeVal;
  if (chunkSizeVal->getType() != innerIV->getType()) {
    chunkSizeCast = chunkCondBBBuilder.CreateIntCast(chunkSizeVal, innerIV->getType(), /*isSigned=*/false);
  }
  Value *chunkCond = chunkCondBBBuilder.CreateICmpULT(innerIV, chunkSizeCast);

  auto innerBr = cast<BranchInst>(innerHeader->getTerminator());
  assert(innerBr->getNumSuccessors() == 2);
  auto innerBodySuccIndex = -1;
  if (innerBr->getSuccessor(0) == chLatch) innerBodySuccIndex = 1;
  if (innerBr->getSuccessor(1) == chLatch) innerBodySuccIndex = 0;
  auto innerBodyBB = innerBr->getSuccessor(innerBodySuccIndex);
  innerBr->setSuccessor(innerBodySuccIndex, chunkCondBB);

  if (originHeaderBrInst->getSuccessor(0) == exitBlock) {
    chunkCondBBBuilder.CreateCondBr(chunkCond, chLatch, innerBodyBB);
  } else {
    chunkCondBBBuilder.CreateCondBr(chunkCond, innerBodyBB, chLatch);
  }
}

void DOALL::addChunkFunctionExecutionAsideOriginalLoop (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker
) {
  auto firstBB = &*LDI->function->begin();
  IRBuilder<> entryBuilder(firstBB->getTerminator());
  LDI->envArray = entryBuilder.CreateAlloca(LDI->envArrayType);

  auto &cxt = LDI->function->getContext();
  LDI->entryPointOfParallelizedLoop = BasicBlock::Create(cxt, "", LDI->function);
  LDI->exitPointOfParallelizedLoop = BasicBlock::Create(cxt, "", LDI->function);
  IRBuilder<> doallBuilder(LDI->entryPointOfParallelizedLoop);

  auto envPtr = createEnvArray(LDI, par, entryBuilder, doallBuilder);
  // TODO(angelo): Outsource num cores / chunk size values to autotuner or heuristic
  auto numCores = ConstantInt::get(par.int64, NUM_CORES);
  auto chunkSize = ConstantInt::get(par.int64, CHUNK_SIZE);

  doallBuilder.CreateCall(this->doallDispatcher, ArrayRef<Value *>({
    (Value *)chunker->f,
    envPtr,
    numCores,
    chunkSize
  }));
  doallBuilder.CreateBr(LDI->exitPointOfParallelizedLoop);

  reducePostEnvironment(LDI, par);
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

    auto producerSCC = LDI->loopSCCDAG->sccOfValue(producer);
    Instruction::BinaryOps binOp;
    for (auto nodePair : producerSCC->internalNodePairs()) {
      auto I = cast<Instruction>(nodePair.first);
      if (I->isAssociative()) {
        auto opCode = I->getOpcode();
        assert(Instruction::isBinaryOp(opCode));
        binOp = static_cast<Instruction::BinaryOps>(opCode);
      }
    }

    Value *accumVal = nullptr;
    for (auto i = 0; i < NUM_CORES; ++i) {
      auto indVal = cast<Value>(ConstantInt::get(par.int64, i));
      auto reduceArrPtr = reduceBuilder.CreateInBoundsGEP(
        reduceArr,
        ArrayRef<Value*>({
          chunker->zeroV,
          indVal
        })
      );
      auto reducePtr = reduceBuilder.CreateBitCast(
        reduceBuilder.CreateLoad(reduceArrPtr),
        PointerType::getUnqual(producer->getType())
      );
      auto reduceVal = reduceBuilder.CreateLoad(reducePtr);
      accumVal = accumVal 
        ? reduceBuilder.CreateBinOp(binOp, accumVal, reduceVal)
        : reduceVal;
    }

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
    // arr->print(errs() << "Index " << envIndex << ", Array: "); errs() << "\n";
    // alloca->print(errs() << "Alloca "); errs() << "\n";
    auto indValue = cast<Value>(ConstantInt::get(par.int64, envIndex));
    auto envPtr = entryBuilder.CreateInBoundsGEP(arr, ArrayRef<Value*>({ chunker->zeroV, indValue }));
    auto depCast = entryBuilder.CreateBitCast(envPtr, PointerType::getUnqual(alloca->getType()));
    auto store = entryBuilder.CreateStore(alloca, depCast);
  };

  /*
   * Create empty environment array for producers, exit block tracking
   */
  for (auto envIndex : LDI->environment->getPreEnvIndices()) {
    Type *envType = LDI->environment->typeOfEnv(envIndex);
    auto varAlloca = entryBuilder.CreateAlloca(envType);

    storeEnvAllocaInArray(LDI->envArray, envIndex, varAlloca);

    /*
     * Insert pre-loop producers into the environment array
     */
    parBuilder.CreateStore(LDI->environment->producerAt(envIndex), varAlloca);
  }
  for (auto envIndex : LDI->environment->getPostEnvIndices()) {
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
