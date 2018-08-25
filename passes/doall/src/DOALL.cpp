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

  auto chunker = createChunkingFuncAndArgTypes(LDI, par);

  // TODO(angelo): Refactor rest of DOALL chunking function creation

  /*
   * Create entry and exit blocks
   * Create outer loop header and latch
   */
  auto &cxt = LDI->function->getContext();
  auto entryBlock = BasicBlock::Create(cxt, "", chunker);
  auto exitBlock = BasicBlock::Create(cxt, "", chunker);
  auto chHeader = BasicBlock::Create(cxt, "", chunker);
  auto chLatch = BasicBlock::Create(cxt, "", chunker);
  IRBuilder<> entryB(entryBlock);

  /*
   * Collect arguments of chunker function
   */
  auto chunkF = chunker;
  auto argIter = chunkF->arg_begin();
  auto envVal = (Value *) &*(argIter++);
  auto coreVal = (Value *) &*(argIter++); 
  auto numCoresVal = (Value *) &*(argIter++);
  auto chunkSizeVal = (Value *) &*(argIter++);

  /*
   * Instruction map from original to cloned/env instructions
   */
  unordered_map<Value *, Value *> instrArgMap;

  /*
   * Load environment variables in chunker entry block
   */
  LDI->envArray = entryB.CreateBitCast(envVal, PointerType::getUnqual(LDI->envArrayType));
  auto zeroV = cast<Value>(ConstantInt::get(par.int64, 0));
  for (auto envInd : LDI->environment->getPreEnvIndices()) {
    auto producer = LDI->environment->producerAt(envInd);
    auto envIndV = cast<Value>(ConstantInt::get(par.int64, envInd));
    auto envPtr = entryB.CreateInBoundsGEP(LDI->envArray, ArrayRef<Value*>({ zeroV, envIndV }));
    auto prodPtr = entryB.CreateBitCast(
      entryB.CreateLoad(envPtr),
      PointerType::getUnqual(producer->getType())
    );
    instrArgMap[producer] = cast<Value>(entryB.CreateLoad(prodPtr));
  }

  /*
   * Create inner loop
   */
  unordered_map<BasicBlock *, BasicBlock *> innerBBMap;
  for (auto originBB : LDI->liSummary.topLoop->bbs) {
    auto cloneBB = BasicBlock::Create(cxt, "", chunkF);
    IRBuilder<> builder(cloneBB);
    innerBBMap[originBB] = cloneBB;
    for (auto &I : *originBB) {
      auto cloneI = builder.Insert(I.clone());
      instrArgMap[&I] = (Value *)cloneI;
    }
  }

  // Save reference to cloned inner loop header
  auto innerHeader = innerBBMap[LDI->header];

  // Map inner loop preheader to outer loop header
  innerBBMap[LDI->preHeader] = chHeader;

  // Map single exit block of inner loop to outer loop latch
  innerBBMap[LDI->loopExitBlocks[0]] = chLatch;

  /*
   * Map original instructions uses to that of clones
   */
  for (auto &B : LDI->liSummary.topLoop->bbs) {
    for (auto &I : *B) {
      auto cloneI = cast<Instruction>(instrArgMap[(Value *)&I]);
      if (auto terminator = dyn_cast<TerminatorInst>(cloneI)) {
        for (int i = 0; i < terminator->getNumSuccessors(); ++i)
        {
          auto succBB = terminator->getSuccessor(i);
          assert(innerBBMap.find(succBB) != innerBBMap.end());
          terminator->setSuccessor(i, innerBBMap[succBB]);
        }
      } else if (auto phi = dyn_cast<PHINode>(cloneI)) {
        for (int i = 0; i < phi->getNumIncomingValues(); ++i) {
          phi->setIncomingBlock(i, innerBBMap[phi->getIncomingBlock(i)]);
        }
      }

      for (auto &op : cloneI->operands()) {
        auto opV = op.get();
        
        if (isa<Instruction>(opV) || isa<Argument>(opV)) {
          if (instrArgMap.find(opV) != instrArgMap.end()) {
            op.set(instrArgMap[opV]);
          } else {
            op->print(errs() << "DOALL: ERROR: Operand ignored:\t"); errs() << "\n";
            abort();
          }
        }
      }
    }
  }

  /*
   * Find origin loop's induction variable
   */
  auto originHeaderBr = LDI->header->getTerminator();
  assert(isa<BranchInst>(originHeaderBr));
  auto originHeaderBrInst = cast<BranchInst>(originHeaderBr);
  auto originCond = originHeaderBrInst->getCondition();
  PHINode *originIV = nullptr;
  Value *maxIV = nullptr;
  int originCondPHIIndex, originCondMaxIndex;
  int opIndex = 0;
  for (auto &condOp : cast<User>(originCond)->operands()) {
    condOp->print(errs() << "COND OP: --------- :"); errs() << "\n";
    bool isPHI = isa<PHINode>(condOp);
    if (isPHI) {
      originCondPHIIndex = opIndex;
      originIV = cast<PHINode>(condOp);
    } else {
      originCondMaxIndex = opIndex;
      maxIV = condOp;
    }
    opIndex++;
  }
  assert(originIV != nullptr && maxIV != nullptr);

  /*
   * Determine step size of induction variable
   */
  User *stepIV = nullptr;
  int stepSizeArgIndex = -1;
  ConstantInt *originStepSize = nullptr;
  for (auto user : originIV->users()) {
    user->print(errs() << "ORIGIN IV USER: "); errs() << "\n";
    auto scev = SE.getSCEV((Value *)user);
    switch (scev->getSCEVType()) {
    case scAddExpr:
    case scAddRecExpr:
      errs() << "Add inst\n";
      stepIV = user;
      Value *lhs = user->getOperand(0);
      Value *rhs = user->getOperand(1);
      // lhs->print(errs() << "LHS: "); errs() << "\n";
      // rhs->print(errs() << "RHS: "); errs() << "\n";
      if (isa<ConstantInt>(lhs)) {
        originStepSize = cast<ConstantInt>(lhs);
        stepSizeArgIndex = 0;
      } else if (isa<ConstantInt>(rhs)) {
        originStepSize = cast<ConstantInt>(rhs);
        stepSizeArgIndex = 1;
      } else continue;
      break;
    }
  }
  assert(stepIV != nullptr && originStepSize != nullptr);

  /*
   * Get start value off of original loop IV
   */
  auto startValPHIIndex = originIV->getBasicBlockIndex(LDI->preHeader);
  auto startVal = originIV->getIncomingValue(startValPHIIndex);

  /*
   * Determine start value and step size for outer chunking loop
   */
  auto chIVStepSize = entryB.CreateMul(numCoresVal, chunkSizeVal);
  chIVStepSize = entryB.CreateMul(chIVStepSize, ConstantInt::get(par.int64, originStepSize->getZExtValue()));
  auto chIVStart = entryB.CreateMul(coreVal, chunkSizeVal);
  chIVStart = entryB.CreateAdd(chIVStart, startVal); 

  /*
   * Create outer loop IV
   */
  IRBuilder<> chHeaderB(chHeader);
  IRBuilder<> chLatchB(chLatch);
  entryB.CreateBr(chHeader);

  auto chIV = chHeaderB.CreatePHI(par.int64, /*numReservedValues=*/2);
  chIV->addIncoming(chIVStart, entryBlock);

  // ASSUMPTION: Monotonically increasing IV
  auto chIVInc = chLatchB.CreateAdd(chIV, chIVStepSize);
  chLatchB.CreateBr(chHeader);

  Value *cloneMaxIV;
  if (auto constMaxIV = dyn_cast<ConstantInt>(maxIV)) {
    cloneMaxIV = ConstantInt::get(constMaxIV->getType(), constMaxIV->getValue());
  } else {
    cloneMaxIV = instrArgMap[maxIV];
  }

  chIV->addIncoming(chIVInc, chLatch);
  assert(isa<CmpInst>(originCond));
  auto originCmp = cast<CmpInst>(originCond);
  CmpInst *condIV;
  auto stricterMaxIVCondPredicate = originCmp->getPredicate();
  if (originCondPHIIndex == 0) {
    // HACK: Make the condition stronger so that chunks don't skip over the equality condition
    if (stricterMaxIVCondPredicate == CmpInst::Predicate::ICMP_EQ) {
      stricterMaxIVCondPredicate = CmpInst::Predicate::ICMP_UGE;
    }
    condIV = CmpInst::Create(originCmp->getOpcode(), stricterMaxIVCondPredicate, chIV, cloneMaxIV);
  } else {
    // HACK: Make the condition stronger so that chunks don't skip over the equality condition
    if (stricterMaxIVCondPredicate == CmpInst::Predicate::ICMP_EQ) {
      stricterMaxIVCondPredicate = CmpInst::Predicate::ICMP_ULE;
    }
    condIV = CmpInst::Create(originCmp->getOpcode(), stricterMaxIVCondPredicate, cloneMaxIV, chIV);
  }

  chHeaderB.Insert(condIV);
  if (originHeaderBrInst->getSuccessor(0) == LDI->loopExitBlocks[0]) {
    chHeaderB.CreateCondBr(condIV, exitBlock, innerHeader);
  } else {
    chHeaderB.CreateCondBr(condIV, innerHeader, exitBlock);
  }

  /*
   *** Alter inner loop to iterate single chunks:
   */

  /*
   * Reset start to 0
   * Revise latch to only inc/dec IV
   */
  auto innerIV = instrArgMap[originIV];
  ((PHINode *)innerIV)->setIncomingValue(startValPHIIndex, ConstantInt::get(originIV->getType(), 0));
  auto innerStepIV = (User *)instrArgMap[(Value *)stepIV];
  innerStepIV->setOperand(stepSizeArgIndex, ConstantInt::get(stepIV->getType(), 1));

  /*
   * Create new, composite induction variable for inner loop
   */
  IRBuilder<> headerBuilder(innerHeader);
  // ASSUMPTION: Monotonically increasing IV
  auto innerOuterIVSum = headerBuilder.CreateAdd(innerIV, chIV);
  // for (auto &use : innerIV->uses()) {
  //   auto userV = (Value *)use.getUser();
  //   userV->print(errs() << "USER V: "); errs() << "\n";
  //   if (userV == innerStepIV || ((Instruction *)userV)->getParent() == innerHeader) continue;
  //   errs() << "--- WAS SET\n";
  //   use.set(innerOuterIVSum);
  // }
  for (auto &use : originIV->uses()) {
    auto cloneV = instrArgMap[(Value *)use.getUser()];
    if (cloneV == innerStepIV || ((Instruction *)cloneV)->getParent() == innerHeader) continue;
    ((User *)cloneV)->replaceUsesOfWith(innerIV, innerOuterIVSum);
  }
  // innerIV->print(errs() << "INNER V: "); errs() << "\n";
  // for (auto user : innerIV->users()) {
  //   user->print(errs() << "INNER USER V: "); errs() << "\n";
  //   if (user == innerStepIV || ((Instruction *)user)->getParent() == innerHeader) continue;
  //   errs() << "--- WAS SET\n";
  //   user->replaceUsesOfWith(innerIV, innerOuterIVSum);
  // }

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

  Value *chunkCond = chunkCondBBBuilder.CreateICmpULT(innerIV, chunkSizeVal);

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
  // chunkF->print(errs() << "CHUNKING FUNCTION:\n"); errs() << "\n";
  // LDI->entryPointOfParallelizedLoop->print(errs() << "Finalized doall BB\n"); errs() << "\n";
  // LDI->function->print(errs() << "LDI function:\n"); errs() << "\n";

  errs() << "DOALL: Exit\n";
  return true;
}

Function * DOALL::createChunkingFuncAndArgTypes (LoopDependenceInfo *LDI, Parallelization &par) {

  /*
   * Function: void chunker(void *env, int64 coreInd, int64 numCores, int64 chunkSize)
   */
  auto M = LDI->function->getParent();
  auto &cxt = M->getContext();

  auto ptrType_int8 = PointerType::getUnqual(par.int8);
  auto funcArgTypes = ArrayRef<Type*>({ ptrType_int8, par.int64, par.int64, par.int64 });
  auto chunkerFuncType = FunctionType::get(Type::getVoidTy(cxt), funcArgTypes, false);
  auto chunker = cast<Function>(M->getOrInsertFunction("", chunkerFuncType));

  LDI->envArrayType = ArrayType::get(ptrType_int8, LDI->environment->envSize());

  return chunker;
}

void DOALL::addChunkFunctionExecutionAsideOriginalLoop (LoopDependenceInfo *LDI, Parallelization &par, Function *chunker) {
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
    (Value *)chunker,
    envPtr,
    numCores,
    chunkSize
  }));
  doallBuilder.CreateBr(LDI->exitPointOfParallelizedLoop);

  reducePostEnvironment(LDI, par);
}

void DOALL::reducePostEnvironment (LoopDependenceInfo *LDI, Parallelization &par) {
  auto &cxt = LDI->function->getContext();
  IRBuilder<> reduceBuilder(LDI->exitPointOfParallelizedLoop);

  auto zeroV = cast<Value>(ConstantInt::get(par.int64, 0));
  for (auto envInd : LDI->environment->getPostEnvIndices()) {
    auto producer = LDI->environment->producerAt(envInd);
    producer->print(errs() << "Producer: "); errs() << "\n";
    auto envIndV = cast<Value>(ConstantInt::get(par.int64, envInd));
    auto envPtr = reduceBuilder.CreateInBoundsGEP(LDI->envArray, ArrayRef<Value*>({ zeroV, envIndV }));
    auto reduceArr = reduceBuilder.CreateBitCast(
      reduceBuilder.CreateLoad(envPtr),
      PointerType::getUnqual(ArrayType::get(PointerType::getUnqual(par.int8), NUM_CORES))
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
      auto reduceArrPtr = reduceBuilder.CreateInBoundsGEP(reduceArr, ArrayRef<Value*>({ zeroV, indVal }));
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

Value *DOALL::createEnvArray (LoopDependenceInfo *LDI, Parallelization &par, IRBuilder<> entryBuilder, IRBuilder<> parBuilder) {

  auto zeroV = ConstantInt::get(par.int64, 0);
  auto storeEnvAllocaInArray = [&](Value *arr, int envIndex, AllocaInst *alloca) -> void {
    arr->print(errs() << "Index " << envIndex << ", Array: "); errs() << "\n";
    alloca->print(errs() << "Alloca "); errs() << "\n";
    auto indValue = cast<Value>(ConstantInt::get(par.int64, envIndex));
    auto envPtr = entryBuilder.CreateInBoundsGEP(arr, ArrayRef<Value*>({ zeroV, indValue }));
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
