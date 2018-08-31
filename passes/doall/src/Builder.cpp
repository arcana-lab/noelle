#include "DOALL.hpp"

std::unique_ptr<ChunkerInfo> DOALL::createChunkingFuncAndArgs (
  LoopDependenceInfo *LDI,
  Parallelization &par
) {

  auto chunker = std::make_unique<ChunkerInfo>();
  auto M = LDI->function->getParent();
  auto &cxt = M->getContext();

  auto voidTy = Type::getVoidTy(cxt);
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
    }

    if (auto phi = dyn_cast<PHINode>(cloneI)) {
      for (int i = 0; i < phi->getNumIncomingValues(); ++i) {
        auto cloneBB = chunker->innerBBMap[phi->getIncomingBlock(i)];
        phi->setIncomingBlock(i, cloneBB);
      }
    }

    // TODO(angelo): Add exhaustive search of types to parallelization
    // utilities for use in DSWP and here in DOALL
    for (auto &op : cloneI->operands()) {
      auto opV = op.get();
      if (chunker->preEnvMap.find(opV) != chunker->preEnvMap.end()) {
        op.set(chunker->preEnvMap[opV]);
      } else if (auto opI = dyn_cast<Instruction>(opV)) {
        if (chunker->innerValMap.find(opI) != chunker->innerValMap.end()) {
          op.set(chunker->innerValMap[opI]);
        }
      }
    }
  }
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

  auto chIV = chHeaderB.CreatePHI(par.int64, /*numReservedValues=*/2);
  chunker->chIV = chIV;

  // ASSUMPTION: Monotonically increasing IV
  auto chIVStepSize = entryB.CreateMul(chunker->numCoresArgVal, chunker->chunkSizeArgVal);
  chIVStepSize = entryB.CreateMul(chIVStepSize, chunker->originStepSize);
  auto chIVInc = chLatchB.CreateAdd(chIV, chIVStepSize);

  chIV->addIncoming(chIVStart, chunker->entryBlock);
  chIV->addIncoming(chIVInc, chunker->chLatch);

  entryB.CreateBr(chunker->chHeader);
  chLatchB.CreateBr(chunker->chHeader);

  Value *cloneMaxIV = chunker->maxIV;
  if (!isa<ConstantInt>(chunker->maxIV)) {
    if (chunker->preEnvMap.find(chunker->maxIV) != chunker->preEnvMap.end()) {
      cloneMaxIV = chunker->preEnvMap[chunker->maxIV];
    } else {
      cloneMaxIV = chunker->innerValMap[(Instruction*)chunker->maxIV];
    }
  }

  CmpInst *chCmp;
  if (chunker->originCmpPHIIndex == 0) {
    chCmp = CmpInst::Create(chunker->originCmp->getOpcode(), chunker->strictPredicate, chIV, cloneMaxIV);
  } else {
    chCmp = CmpInst::Create(chunker->originCmp->getOpcode(), chunker->strictPredicate, cloneMaxIV, chIV);
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
    ConstantInt::get(chunker->originIV->getType(), 0)
  );
  auto innerStepIV = (User *)chunker->innerValMap[(Instruction*)chunker->stepperIV];
  innerStepIV->setOperand(
    chunker->stepSizeIVIndex,
    ConstantInt::get(chunker->stepperIV->getType(), 1)
  );

  /*
   * Create new, composite induction variable for inner loop
   */
  auto innerHeader = chunker->innerBBMap[LDI->header];
  IRBuilder<> headerBuilder(innerHeader);
  // ASSUMPTION: Monotonically increasing IV
  auto innerOuterIVSum = headerBuilder.CreateAdd(innerIV, chunker->chIV);
  for (auto &use : chunker->originIV->uses()) {
    auto cloneI = chunker->innerValMap[(Instruction *)use.getUser()];
    auto cloneU = (User *)cloneI;
    if (cloneU == innerStepIV || cloneI->getParent() == innerHeader) continue;
    cloneU->replaceUsesOfWith(innerIV, innerOuterIVSum);
  }

  /*
   * Replace inner loop original condition with less than total loop size condition
   * Add a cond to check for less than chunk size
   */
  auto innerCondIV = (User *)chunker->innerValMap[chunker->originCmp];
  // Ensure the add comes before its use in the comparison
  innerCondIV->setOperand(chunker->originCmpPHIIndex, innerOuterIVSum);
  ((CmpInst *)innerCondIV)->setPredicate(chunker->strictPredicate);

  auto ivSumInst = cast<Instruction>(innerOuterIVSum);
  ivSumInst->removeFromParent();
  ivSumInst->insertBefore(cast<Instruction>(innerCondIV));

  auto chunkCondBB = chunker->createChunkerBB();
  IRBuilder<> chunkCondBBBuilder(chunkCondBB);

  Value *chunkCond = chunkCondBBBuilder.CreateICmpULT(innerIV, chunker->chunkSizeArgVal);

  auto innerBr = cast<BranchInst>(innerHeader->getTerminator());
  assert(innerBr->getNumSuccessors() == 2);
  auto innerBodySuccIndex = -1;
  if (innerBr->getSuccessor(0) == chunker->chLatch) innerBodySuccIndex = 1;
  if (innerBr->getSuccessor(1) == chunker->chLatch) innerBodySuccIndex = 0;
  auto innerBodyBB = innerBr->getSuccessor(innerBodySuccIndex);
  innerBr->setSuccessor(innerBodySuccIndex, chunkCondBB);

  if (chunker->originHeaderBr->getSuccessor(0) == chunker->exitBlock) {
    chunkCondBBBuilder.CreateCondBr(chunkCond, chunker->chLatch, innerBodyBB);
  } else {
    chunkCondBBBuilder.CreateCondBr(chunkCond, innerBodyBB, chunker->chLatch);
  }
}

