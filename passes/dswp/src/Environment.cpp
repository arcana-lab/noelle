#include "DSWP.hpp"

using namespace llvm;

void DSWP::collectPreLoopEnvInfo (DSWPLoopDependenceInfo *LDI)
{
  for (auto nodeI : LDI->loopDG->externalNodePairs())
  {
    auto externalNode = nodeI.second;
    auto externalValue = externalNode->getT();
    auto envIndex = LDI->environment->envProducers.size();

    /*
     * Determine whether the external value is a producer to loop-internal values
     */
    bool isPreLoop = false;
    for (auto outgoingEdge : externalNode->getOutgoingEdges())
    {
      if (outgoingEdge->isMemoryDependence() || outgoingEdge->isControlDependence()) continue;
      isPreLoop = true;
      auto internalValue = outgoingEdge->getIncomingT();

      bool isSharedInst = false;
      for (auto scc : LDI->removableSCCs)
      {
        if (!scc->isInternal(internalValue)) continue;
        isSharedInst = true;
        for (auto &stage : LDI->stages) stage->incomingEnvs.insert(envIndex);
        break;
      }

      if (!isSharedInst)
      {
        for (auto &stage : LDI->stages)
        {
          bool isInternal = false;
          for (auto scc : stage->stageSCCs) isInternal |= scc->isInternal(internalValue);
          if (isInternal) stage->incomingEnvs.insert(envIndex);
        }
      }
    }
    if (isPreLoop) LDI->environment->addPreLoopProducer(externalValue);
  }
}

void DSWP::collectPostLoopEnvInfo (DSWPLoopDependenceInfo *LDI)
{
  for (auto nodeI : LDI->loopDG->externalNodePairs())
  {
    auto externalNode = nodeI.second;
    auto externalValue = externalNode->getT();
    auto envIndex = LDI->environment->envProducers.size();

    /*
     * Determine whether the external value is a consumer of loop-internal values
     */
    for (auto incomingEdge : externalNode->getIncomingEdges())
    {
      if (incomingEdge->isMemoryDependence() || incomingEdge->isControlDependence()) continue;
      auto internalValue = incomingEdge->getOutgoingT();
      auto internalInst = cast<Instruction>(internalValue);
      LDI->environment->prodConsumers[internalInst].insert(externalValue);

      /*
       * Determine the producer of the edge to the external value
       */
      if (LDI->environment->producerIndexMap.find(internalValue) != LDI->environment->producerIndexMap.end())
      {
        envIndex = LDI->environment->producerIndexMap[internalValue];
      }
      else
      {
        envIndex = LDI->environment->envProducers.size();
        LDI->environment->addPostLoopProducer(internalValue);
      }

      bool isSharedInst = false;
      for (auto scc : LDI->removableSCCs)
      {
        if (!scc->isInternal(internalValue)) continue;
        isSharedInst = true;
        LDI->stages[0]->outgoingEnvs[internalInst] = envIndex;
        break;
      }

      if (!isSharedInst)
      {
        for (auto &stage : LDI->stages)
        {
          bool isInternal = false;
          for (auto scc : stage->stageSCCs) isInternal |= scc->isInternal(internalValue);
          if (isInternal)
          {
            stage->outgoingEnvs[internalInst] = envIndex;
            break;
          }
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
    auto envType = LDI->environment->envProducers[envIndex]->getType();
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
  for (int i = 0; i < stageInfo->loopExitBlocks.size(); ++i)
  {
    IRBuilder<> builder(&*stageInfo->loopExitBlocks[i]->begin());
    auto envIndexValue = cast<Value>(ConstantInt::get(par.int64, LDI->environment->indexOfExitBlock()));
    auto envPtr = builder.CreateInBoundsGEP(stageInfo->envAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndexValue }));
    auto envVar = builder.CreateBitCast(builder.CreateLoad(envPtr), PointerType::getUnqual(par.int32));
    builder.CreateStore(ConstantInt::get(par.int32, i), envVar);
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
