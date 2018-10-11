#include "DSWP.hpp"

using namespace llvm;

void DSWP::collectPreLoopEnvInfo (DSWPLoopDependenceInfo *LDI) {
  for (auto envIndex : LDI->environment->getPreEnvIndices()) {
    auto producer = LDI->environment->producerAt(envIndex);

    for (auto consumer : LDI->environment->consumersOf(producer)) {
      bool isSharedInst = false;
      for (auto scc : LDI->sccdagAttrs.clonableSCCs) {
        if (!scc->isInternal(consumer)) continue;
        isSharedInst = true;
        for (auto i = 0; i < LDI->stages.size(); ++i) {
          LDI->envBuilder->getUser(i)->addPreEnvIndex(envIndex);
        }
        break;
      }

      if (!isSharedInst) {
        for (auto i = 0; i < LDI->stages.size(); ++i) {
          auto &stage = LDI->stages[i];
          bool isInternal = false;
          for (auto scc : stage->stageSCCs) isInternal |= scc->isInternal(consumer);
          if (isInternal) LDI->envBuilder->getUser(i)->addPreEnvIndex(envIndex);
        }
      }
    }
  }
}

void DSWP::collectPostLoopEnvInfo (DSWPLoopDependenceInfo *LDI) {
  for (auto envIndex : LDI->environment->getPostEnvIndices()) {
    auto producer = LDI->environment->producerAt(envIndex);

    bool isSharedInst = false;
    for (auto scc : LDI->sccdagAttrs.clonableSCCs) {
      if (!scc->isInternal(producer)) continue;
      isSharedInst = true;
      LDI->envBuilder->getUser(0)->addPostEnvIndex(envIndex);
      break;
    }

    if (!isSharedInst) {
      for (auto i = 0; i < LDI->stages.size(); ++i) {
        auto &stage = LDI->stages[i];
        bool isInternal = false;
        for (auto scc : stage->stageSCCs) isInternal |= scc->isInternal(producer);
        if (isInternal) {
          LDI->envBuilder->getUser(i)->addPostEnvIndex(envIndex);
          break;
        }
      }
    }
  }
}

void DSWP::loadAndStoreEnv (
  DSWPLoopDependenceInfo *LDI,
  std::unique_ptr<StageInfo> &stageInfo,
  Parallelization &par
) {
  auto envUser = LDI->envBuilder->getUser(stageInfo->order);
  IRBuilder<> entryBuilder(stageInfo->entryBlock);
  auto envArg = &*(stageInfo->sccStage->arg_begin());
  envUser->setEnvArray(entryBuilder.CreateBitCast(
    envArg,
    PointerType::getUnqual(LDI->envBuilder->getEnvArrayTy())
  ));

  /*
   * Access environment variables for loading and storing
   */
  for (auto envIndex : envUser->getPreEnvIndices()) {
    envUser->createEnvPtr(entryBuilder, envIndex);
  }
  for (auto envIndex : envUser->getPostEnvIndices()) {
    envUser->createEnvPtr(entryBuilder, envIndex);
  }

  /*
   * Load (outside of loop -> SCC) dependencies
   */
  for (auto envIndex : envUser->getPreEnvIndices())
  {
    auto envLoad = entryBuilder.CreateLoad(envUser->getEnvPtr(envIndex));
    stageInfo->envLoadMap[envIndex] = cast<Instruction>(envLoad);
  }

  /*
   * Store (SCC -> outside of loop) dependencies
   */
  for (auto envIndex : envUser->getPostEnvIndices())
  {
    auto producer = LDI->environment->producerAt(envIndex);
    auto outgoingDepI = stageInfo->iCloneMap[cast<Instruction>(producer)];
    auto outgoingDepBB = outgoingDepI->getParent();
    IRBuilder<> outgoingBuilder(outgoingDepBB->getTerminator());
    outgoingBuilder.CreateStore(outgoingDepI, envUser->getEnvPtr(envIndex));
  }

  /*
   * Store the basic block index of the exiting block
   */
  if (stageInfo->loopExitBlocks.size() > 1) {
    auto exitBlockEnvIndex = LDI->environment->indexOfExitBlock();
    envUser->createEnvPtr(entryBuilder, exitBlockEnvIndex);

    for (int i = 0; i < stageInfo->loopExitBlocks.size(); ++i) {
      IRBuilder<> builder(&*stageInfo->loopExitBlocks[i]->begin());
      auto envPtr = envUser->getEnvPtr(exitBlockEnvIndex);
      builder.CreateStore(ConstantInt::get(par.int32, i), envPtr);
    }
  }
}

void DSWP::storeOutgoingDependentsIntoExternalValues (DSWPLoopDependenceInfo *LDI, IRBuilder<> builder, Parallelization &par) {

  /*
   * Extract the outgoing dependents for each stage
   */
  for (int envInd : LDI->environment->getPostEnvIndices()) {
    auto prod = LDI->environment->producerAt(envInd);
    auto envVar = builder.CreateLoad(LDI->envBuilder->getEnvVar(envInd));

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
