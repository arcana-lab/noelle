#include "DSWP.hpp"

using namespace llvm;

void DSWP::collectPreLoopEnvInfo (DSWPLoopDependenceInfo *LDI) {
  for (auto envIndex : LDI->environment->getPreEnvIndices()) {
    auto producer = LDI->environment->producerAt(envIndex);

    for (auto consumer : LDI->environment->consumersOf(producer)) {
      bool isSharedInst = false;
      for (auto scc : LDI->partition.removableNodes) {
        if (!scc->isInternal(consumer)) continue;
        isSharedInst = true;
        for (auto &stage : LDI->stages) stage->incomingEnvs.insert(envIndex);
        break;
      }

      if (!isSharedInst) {
        for (auto &stage : LDI->stages) {
          bool isInternal = false;
          for (auto scc : stage->stageSCCs) isInternal |= scc->isInternal(consumer);
          if (isInternal) stage->incomingEnvs.insert(envIndex);
        }
      }
    }
  }
}

void DSWP::collectPostLoopEnvInfo (DSWPLoopDependenceInfo *LDI) {
  for (auto envIndex : LDI->environment->getPostEnvIndices()) {
    auto producer = LDI->environment->producerAt(envIndex);
    // TODO(angelo): remove outgoingEnvs structure from StageInfo, then remove producerI
    auto producerI = cast<Instruction>(producer);

    bool isSharedInst = false;
    for (auto scc : LDI->partition.removableNodes) {
      if (!scc->isInternal(producer)) continue;
      isSharedInst = true;
      LDI->stages[0]->outgoingEnvs[producerI] = envIndex;
      break;
    }

    if (!isSharedInst) {
      for (auto &stage : LDI->stages) {
        bool isInternal = false;
        for (auto scc : stage->stageSCCs) isInternal |= scc->isInternal(producer);
        if (isInternal) {
          stage->outgoingEnvs[producerI] = envIndex;
          break;
        }
      }
    }
  }
}

void DSWP::loadAndStoreEnv (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo, Parallelization &par)
{
  IRBuilder<> entryBuilder(stageInfo->entryBlock);

  auto envArg = &*(stageInfo->sccStage->arg_begin());
  stageInfo->envAlloca = entryBuilder.CreateBitCast(envArg, PointerType::getUnqual(LDI->envArrayType));

  auto accessProducerFromIndex = [&](int envIndex, IRBuilder<> builder) -> Value * {
    auto envIndexValue = cast<Value>(ConstantInt::get(par.int64, envIndex));
    auto envPtr = builder.CreateInBoundsGEP(stageInfo->envAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndexValue }));
    auto envType = LDI->environment->typeOfEnv(envIndex);
    return builder.CreateBitCast(builder.CreateLoad(envPtr), PointerType::getUnqual(envType));
  };

  /*
   * Store (SCC -> outside of loop) dependencies within the environment array
   */
  for (auto outgoingEnvPair : stageInfo->outgoingEnvs)
  {
    auto outgoingDepClone = stageInfo->iCloneMap[outgoingEnvPair.first];
    auto outgoingDepBB = outgoingDepClone->getParent();
    IRBuilder<> outgoingBuilder(outgoingDepBB->getTerminator());
    auto envVar = accessProducerFromIndex(outgoingEnvPair.second, outgoingBuilder);
    outgoingBuilder.CreateStore(outgoingDepClone, envVar);
  }

  /*
   * Store exit index in the exit environment variable
   */
  if (stageInfo->loopExitBlocks.size() > 1) {
    for (int i = 0; i < stageInfo->loopExitBlocks.size(); ++i) {
      IRBuilder<> builder(&*stageInfo->loopExitBlocks[i]->begin());
      auto envIndexValue = cast<Value>(ConstantInt::get(par.int64, LDI->environment->indexOfExitBlock()));
      auto envPtr = builder.CreateInBoundsGEP(stageInfo->envAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndexValue }));
      auto envVar = builder.CreateBitCast(builder.CreateLoad(envPtr), PointerType::getUnqual(par.int32));
      builder.CreateStore(ConstantInt::get(par.int32, i), envVar);
    }
  }

  /*
   * Load (outside of loop -> SCC) dependencies from the environment array 
   */
  for (auto envIndex : stageInfo->incomingEnvs)
  {
    auto envVar = accessProducerFromIndex(envIndex, entryBuilder);
    auto envLoad = entryBuilder.CreateLoad(envVar);
    stageInfo->envLoadMap[envIndex] = cast<Instruction>(envLoad);
  }
}

void DSWP::storeOutgoingDependentsIntoExternalValues (DSWPLoopDependenceInfo *LDI, IRBuilder<> builder, Parallelization &par) {

  /*
   * Extract the outgoing dependents for each stage
   */
  for (int envInd : LDI->environment->getPostEnvIndices()) {
    auto prod = LDI->environment->producerAt(envInd);
    auto envIndex = cast<Value>(ConstantInt::get(par.int64, envInd));
    auto depInEnvPtr = builder.CreateInBoundsGEP(LDI->envArray, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndex }));
    auto envVarCast = builder.CreateBitCast(builder.CreateLoad(depInEnvPtr), PointerType::getUnqual(prod->getType()));
    auto envVar = builder.CreateLoad(envVarCast);

    for (auto consumer : LDI->environment->consumersOf(prod)) {
      if (auto depPHI = dyn_cast<PHINode>(consumer)) {
        depPHI->addIncoming(envVar, LDI->exitPointOfParallelizedLoop);
        continue;
      }
      prod->print(errs() << "Producer of environment variable:\t"); errs() << "\n";
      errs() << "Loop not in LCSSA!\n";
      abort();
    }
  }
}
