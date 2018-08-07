#include "Parallelizer.hpp"
#include "ChunkInfo.hpp"

using namespace llvm;

bool Parallelizer::applyDOALL (DSWPLoopDependenceInfo *LDI, Parallelization &par, Heuristics *h) {
  //TODO
  errs() << "DSWP:   IS DO ALL LOOP: -------------|| || || || || ||---------------\n";

  /*
   * Collect environment information
   * For now, use environment variable on LDI structure
   */
  LDI->environment = std::make_unique<EnvInfo>();
  for (auto nodeI : LDI->loopDG->externalNodePairs()) {
    auto externalNode = nodeI.second;
    auto externalValue = externalNode->getT();
    auto envIndex = LDI->environment->envProducers.size();

    bool isProducer = false;
    for (auto edge : externalNode->getOutgoingEdges())
    {
      if (edge->isMemoryDependence() || edge->isControlDependence()) continue;
      isProducer = true;
      LDI->environment->prodConsumers[externalValue].insert(edge->getIncomingT());
    }
    if (isProducer) LDI->environment->addPreLoopProducer(externalValue);
  }

  for (auto pC : LDI->environment->prodConsumers) {
    pC.first->print(errs() << "Producer: "); errs() << "\n";
    for (auto C : pC.second) {
      C->print(errs() << "\tConsumer: "); errs() << "\n";
    }
  }

  /*
   * Function: void chunker(void *env, int64 coreInd, int64 numCores, int64 chunkSize)
   */
  auto M = LDI->function->getParent();
  auto &cxt = M->getContext();
  LDI->doallChunk = std::make_unique<ChunkInfo>();

  auto ptrType_int8 = PointerType::getUnqual(par.int8);
  auto funcArgTypes = ArrayRef<Type*>({ ptrType_int8, par.int64, par.int64, par.int64 });
  auto chunkerFuncType = FunctionType::get(Type::getVoidTy(cxt), funcArgTypes, false);

  /*
   * Create chunker function
   * Create entry and exit blocks
   * Create outer loop header and latch
   */
  auto chunkF = cast<Function>(M->getOrInsertFunction("", chunkerFuncType));
  LDI->doallChunk->chunker = chunkF;
  auto entryBlock = BasicBlock::Create(cxt, "", LDI->doallChunk->chunker);
  auto exitBlock = BasicBlock::Create(cxt, "", LDI->doallChunk->chunker);
  auto chHeader = BasicBlock::Create(cxt, "", LDI->doallChunk->chunker);
  auto chLatch = BasicBlock::Create(cxt, "", LDI->doallChunk->chunker);
  IRBuilder<> entryB(entryBlock);
  IRBuilder<> exitB(exitBlock);

  /*
   * Collect arguments of chunker function
   */
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
  LDI->envArrayType = ArrayType::get(ptrType_int8, LDI->environment->envSize());
  auto envAlloca = entryB.CreateBitCast(envVal, PointerType::getUnqual(LDI->envArrayType));
  int envIndex = 0;
  for (auto envProd : LDI->environment->envProducers)
  {
    auto envIndexValue = cast<Value>(ConstantInt::get(par.int64, envIndex++));
    auto envPtr = entryB.CreateInBoundsGEP(envAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndexValue }));
    auto envBitcastPtr = entryB.CreateBitCast(entryB.CreateLoad(envPtr), PointerType::getUnqual(envProd->getType()));
    instrArgMap[cast<Value>(envProd)] = cast<Value>(entryB.CreateLoad(envBitcastPtr));
  }

  /*
   * Create inner loop
   */
  unordered_map<BasicBlock *, BasicBlock *> innerBBMap;
  for (auto originBB : LDI->liSummary.topLoop->bbs) {
    innerBBMap[originBB] = BasicBlock::Create(cxt, "", chunkF);
    for (auto &I : *originBB) {
      instrArgMap[&I] = (Value *)(I.clone());
    }
  }

  // Save reference to cloned inner loop header
  auto innerHeader = innerBBMap[LDI->header];

  // Map single exit block of inner loop to outer loop latch
  innerBBMap[LDI->loopExitBlocks[0]] = chLatch;

  /*
   * Map original instructions uses to that of clones
   */
  for (auto &B : LDI->liSummary.topLoop->bbs) {
    for (auto &I : *B) {
      if (auto terminator = dyn_cast<TerminatorInst>(&I)) {
        for (int i = 0; i < terminator->getNumSuccessors(); ++i)
        {
          auto succBB = terminator->getSuccessor(i);
          assert(innerBBMap.find(succBB) != innerBBMap.end());
          terminator->setSuccessor(i, innerBBMap[succBB]);
        }
        continue;
      }

      for (auto &op : I.operands()) {
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
  auto originCond = cast<BranchInst>(originHeaderBr)->getCondition();
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
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*LDI->function).getSE();
  User *stepIV = nullptr;
  int stepSizeArgIndex = -1;
  ConstantInt *originStepSize = nullptr;
  for (auto user : originIV->users()) {
    // user->print(errs() << "ORIGIN IV USER: "); errs() << "\n";
    auto scev = SE.getSCEV((Value *)user);
    switch (scev->getSCEVType()) {
    case scAddExpr:
    case scAddRecExpr:
      // errs() << "Add inst\n";
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
  chIV->addIncoming(ConstantInt::get(par.int64, 0), entryBlock);

  // ASSUMPTION: Monotonically increasing IV
  auto chIVInc = chLatchB.CreateAdd(chIV, chIVStepSize);
  chLatchB.CreateBr(chHeader);

  chIV->addIncoming(chIVInc, chLatch);
  assert(isa<CmpInst>(originCond));
  auto originCmp = cast<CmpInst>(originCond);
  auto condIV = CmpInst::Create(originCmp->getOpcode(), originCmp->getPredicate(), chIV, maxIV);
  chHeaderB.Insert(condIV);
  chHeaderB.CreateCondBr(condIV, innerHeader, exitBlock);

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
   * Replace inner loop original condition with less than total loop size condition
   * Add a cond to check for less than chunk size
   */
  IRBuilder<> headerBuilder(innerHeader);
  // ASSUMPTION: Monotonically increasing IV
  auto innerOuterIVSum = headerBuilder.CreateAdd(innerIV, chIV);
  auto innerCondIV = (User *)instrArgMap[originCond];
  innerCondIV->setOperand(originCondPHIIndex, innerOuterIVSum);

  auto chunkCondBB = BasicBlock::Create(cxt, "", LDI->doallChunk->chunker);
  IRBuilder<> chunkCondBBBuilder(chunkCondBB);
  auto chunkCond = chunkCondBBBuilder.CreateICmpULT(innerIV, chunkSizeVal);

  auto innerBr = cast<BranchInst>(innerHeader->getTerminator());
  assert(innerBr->getNumSuccessors() == 2);
  auto innerBodySuccIndex = -1;
  if (innerBr->getSuccessor(0) == LDI->loopExitBlocks[0]) innerBodySuccIndex = 1;
  if (innerBr->getSuccessor(1) == LDI->loopExitBlocks[0]) innerBodySuccIndex = 0;
  auto innerBodyBB = innerBr->getSuccessor(innerBodySuccIndex);
  innerBr->setSuccessor(innerBodySuccIndex, chunkCondBB);

  chunkCondBBBuilder.CreateCondBr(chunkCond, innerBodyBB, chLatch);

  return false;
}
