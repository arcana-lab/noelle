#include "DSWP.hpp"

using namespace llvm;

void DSWP::popValueQueues (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo, Parallelization &par)
{
  for (auto queueIndex : stageInfo->popValueQueues)
  {
    auto &queueInfo = LDI->queues[queueIndex];
    auto queueInstrs = stageInfo->queueInstrMap[queueIndex].get();
    auto queueCallArgs = ArrayRef<Value*>({ queueInstrs->queuePtr, queueInstrs->allocaCast });

    auto bb = queueInfo->producer->getParent();
    assert(stageInfo->sccBBCloneMap.find(bb) != stageInfo->sccBBCloneMap.end());

    IRBuilder<> builder(stageInfo->sccBBCloneMap[bb]);
    queueInstrs->queueCall = builder.CreateCall(queuePops[queueSizeToIndex[queueInfo->bitLength]], queueCallArgs);
    queueInstrs->load = builder.CreateLoad(queueInstrs->alloca);

    /*
     * Position queue call and load relatively identically to where the producer is in the basic block
     */
    bool pastProducer = false;
    for (auto &I : *bb)
    {
      if (&I == queueInfo->producer) pastProducer = true;
      else if (auto phi = dyn_cast<PHINode>(&I)) continue;
      else if (pastProducer && stageInfo->iCloneMap.find(&I) != stageInfo->iCloneMap.end())
      {
        auto iClone = stageInfo->iCloneMap[&I];
        cast<Instruction>(queueInstrs->queueCall)->moveBefore(iClone);
        cast<Instruction>(queueInstrs->load)->moveBefore(iClone);
        break;
      }
    }
  }
}

void DSWP::pushValueQueues (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo, Parallelization &par)
{
  for (auto queueIndex : stageInfo->pushValueQueues)
  {
    auto queueInstrs = stageInfo->queueInstrMap[queueIndex].get();
    auto queueInfo = LDI->queues[queueIndex].get();
    auto queueCallArgs = ArrayRef<Value*>({ queueInstrs->queuePtr, queueInstrs->allocaCast });
    
    auto pClone = stageInfo->iCloneMap[queueInfo->producer];
    auto pCloneBB = pClone->getParent();
    IRBuilder<> builder(pCloneBB);
    auto store = builder.CreateStore(pClone, queueInstrs->alloca);
    queueInstrs->queueCall = builder.CreateCall(queuePushes[queueSizeToIndex[queueInfo->bitLength]], queueCallArgs);

    bool pastProducer = false;
    for (auto &I : *pCloneBB)
    {
      if (&I == pClone) pastProducer = true;
      else if (auto phi = dyn_cast<PHINode>(&I)) continue;
      else if (pastProducer)
      {
        store->moveBefore(&I);
        cast<Instruction>(queueInstrs->queueCall)->moveBefore(&I);
        break;
      }
    }
  }
}

void DSWP::registerQueue (DSWPLoopDependenceInfo *LDI, StageInfo *fromStage, StageInfo *toStage, Instruction *producer, Instruction *consumer)
{
  int queueIndex = LDI->queues.size();
  for (auto queueI : fromStage->producerToQueues[producer])
  {
    if (LDI->queues[queueI]->toStage != toStage->order) continue;
    queueIndex = queueI;
    break;
  }

  if (queueIndex == LDI->queues.size())
  {
    LDI->queues.push_back(std::move(std::make_unique<QueueInfo>(producer, consumer, producer->getType())));
    fromStage->producerToQueues[producer].insert(queueIndex);
  }

  fromStage->pushValueQueues.insert(queueIndex);
  toStage->popValueQueues.insert(queueIndex);
  toStage->producedPopQueue[producer] = queueIndex;

  auto queueInfo = LDI->queues[queueIndex].get();
  queueInfo->consumers.insert(consumer);
  queueInfo->fromStage = fromStage->order;
  queueInfo->toStage = toStage->order;

  bool byteSize = queueSizeToIndex.find(queueInfo->bitLength) != queueSizeToIndex.end();
  if (!byteSize)
  { 
    errs() << "NOT BYTE SIZE (" << queueInfo->bitLength << "): "; producer->getType()->print(errs()); errs() <<  "\n";
    producer->print(errs() << "Producer: "); errs() << "\n";
    abort();
  }
}

void DSWP::collectControlQueueInfo (DSWPLoopDependenceInfo *LDI)
{
  auto findContaining = [&](Value *val) -> std::pair<StageInfo *, SCC *> {
    for (auto &stage : LDI->stages)
    {
      for (auto scc : stage->stageSCCs) if (scc->isInternal(val)) return std::make_pair(stage.get(), scc);
      for (auto scc : stage->removableSCCs) if (scc->isInternal(val)) return std::make_pair(stage.get(), scc);
    }
    return std::make_pair(nullptr, nullptr);
  };

  for (auto bb : LDI->loopBBs){
    auto consumerTerm = bb->getTerminator();
    if (consumerTerm->getNumSuccessors() == 1) continue;
    auto consumerI = cast<Instruction>(bb->getTerminator());
    auto brStageSCC = findContaining(cast<Value>(consumerI));
    assert(brStageSCC.first != nullptr);

    for (auto edge : brStageSCC.second->fetchNode(cast<Value>(consumerI))->getIncomingEdges())
    {
      if (edge->isControlDependence()) continue;
      auto producer = cast<Instruction>(edge->getOutgoingT());
      auto prodStageSCC = findContaining(cast<Value>(producer));
      StageInfo *prodStage = prodStageSCC.first;
      SCC *prodSCC = prodStageSCC.second;
      assert(prodStage != nullptr);

      for (auto &otherStage : LDI->stages)
      {
        /*
         * Register a queue if the producer isn't in the stage and the consumer is used
         */
        if (otherStage.get() == prodStage) continue;
        if (otherStage->removableSCCs.find(prodSCC) != otherStage->removableSCCs.end()) continue;
        if (otherStage->usedCondBrs.find(consumerTerm) == otherStage->usedCondBrs.end()) continue;
        registerQueue(LDI, prodStage, otherStage.get(), producer, consumerI);
      }
    }
  }
}

void DSWP::loadAllQueuePointersInEntry (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo, Parallelization &par) {
  IRBuilder<> entryBuilder(stageInfo->entryBlock);
  auto argIter = stageInfo->sccStage->arg_begin();
  auto queuesArray = entryBuilder.CreateBitCast(&*(++argIter), PointerType::getUnqual(LDI->queueArrayType));

  /*
   * Load this stage's relevant queues
   */
  auto loadQueuePtrFromIndex = [&](int queueIndex) -> void {
    auto queueInfo = LDI->queues[queueIndex].get();
    auto queueIndexValue = cast<Value>(ConstantInt::get(par.int64, queueIndex));
    auto queuePtr = entryBuilder.CreateInBoundsGEP(queuesArray, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, queueIndexValue }));
    auto queueCast = entryBuilder.CreateBitCast(queuePtr, PointerType::getUnqual(queueTypes[queueSizeToIndex[queueInfo->bitLength]]));

    auto queueInstrs = std::make_unique<QueueInstrs>();
    queueInstrs->queuePtr = entryBuilder.CreateLoad(queueCast);
    queueInstrs->alloca = entryBuilder.CreateAlloca(queueInfo->dependentType);
    queueInstrs->allocaCast = entryBuilder.CreateBitCast(queueInstrs->alloca, PointerType::getUnqual(queueElementTypes[queueSizeToIndex[queueInfo->bitLength]]));
    stageInfo->queueInstrMap[queueIndex] = std::move(queueInstrs);
  };

  for (auto queueIndex : stageInfo->pushValueQueues) loadQueuePtrFromIndex(queueIndex);
  for (auto queueIndex : stageInfo->popValueQueues) loadQueuePtrFromIndex(queueIndex);
}

void DSWP::collectPartitionedSCCQueueInfo (DSWPLoopDependenceInfo *LDI)
{
  for (auto scc : LDI->loopSCCDAG->getNodes())
  {
    for (auto sccEdge : scc->getOutgoingEdges())
    {
      auto sccPair = sccEdge->getNodePair();
      auto fromSCC = sccPair.first->getT();
      auto toSCC = sccPair.second->getT();
      if (LDI->partitions.isRemovable(fromSCC)) continue;
      if (LDI->partitions.isRemovable(toSCC)) continue;

      auto fromStage = LDI->sccToStage[fromSCC];
      auto toStage = LDI->sccToStage[toSCC];
      if (fromStage == toStage) continue;

      /*
       * Create value queues for each dependency of the form: producer -> consumers
       */
      for (auto instructionEdge : sccEdge->getSubEdges())
      {
        assert(!instructionEdge->isMemoryDependence());
        if (instructionEdge->isControlDependence()) continue;
        auto producer = cast<Instruction>(instructionEdge->getOutgoingT());
        auto consumer = cast<Instruction>(instructionEdge->getIncomingT());
        registerQueue(LDI, fromStage, toStage, producer, consumer);
      }
    }
  }
}

void DSWP::collectRemovableSCCQueueInfo (DSWPLoopDependenceInfo *LDI) {
  for (auto &stage : LDI->stages)
  {
    auto toStage = stage.get();
    for (auto removableSCC : stage->removableSCCs)
    {
      for (auto sccEdge : LDI->loopSCCDAG->fetchNode(removableSCC)->getIncomingEdges())
      {
        auto fromSCC = sccEdge->getOutgoingT();
        if (LDI->partitions.isRemovable(fromSCC)) continue;
        auto fromStage = LDI->sccToStage[fromSCC];

        /*
         * Create value queues for each dependency of the form: producer -> consumers
         */
        for (auto instructionEdge : sccEdge->getSubEdges())
        {
          assert(!instructionEdge->isMemoryDependence());
          if (instructionEdge->isControlDependence()) continue;
          auto producer = cast<Instruction>(instructionEdge->getOutgoingT());
          auto consumer = cast<Instruction>(instructionEdge->getIncomingT());
          registerQueue(LDI, fromStage, toStage, producer, consumer);
        }
      }
    }
  }
}
