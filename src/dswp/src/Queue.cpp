/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "DSWP.hpp"

using namespace llvm;

void DSWP::registerQueue (
  Parallelization &par,
  LoopDependenceInfo *LDI,
  DSWPTask *fromStage,
  DSWPTask *toStage,
  Instruction *producer,
  Instruction *consumer
) {

  /*
   * Find/create the push queue in the producer stage
   */
  int queueIndex = this->queues.size();
  QueueInfo *queueInfo = nullptr;
  for (auto queueI : fromStage->producerToQueues[producer]) {
    if (this->queues[queueI]->toStage != toStage->order) continue;
    queueIndex = queueI;
    queueInfo = this->queues[queueIndex].get();
    break;
  }
  if (queueIndex == this->queues.size()) {
    this->queues.push_back(std::move(std::make_unique<QueueInfo>(producer, consumer, producer->getType())));
    fromStage->producerToQueues[producer].insert(queueIndex);
    queueInfo = this->queues[queueIndex].get();

    /*
     * Confirm a new queue is of a size handled by the parallelizer
     */
    auto& queueTypes = par.queues.queueSizeToIndex;
    bool byteSize = queueTypes.find(queueInfo->bitLength) != queueTypes.end();
    if (!byteSize) {
      errs() << "NOT SUPPORTED BYTE SIZE (" << queueInfo->bitLength << "): "; producer->getType()->print(errs()); errs() <<  "\n";
      producer->print(errs() << "Producer: "); errs() << "\n";
      abort();
    }
  }

  /*
   * Track queue indices in stages
   */
  fromStage->pushValueQueues.insert(queueIndex);
  toStage->popValueQueues.insert(queueIndex);
  toStage->producedPopQueue[producer] = queueIndex;

  /*
   * Track the stages this queue communicates between
   */
  queueInfo->consumers.insert(consumer);
  queueInfo->fromStage = fromStage->order;
  queueInfo->toStage = toStage->order;
}

void DSWP::collectControlQueueInfo (LoopDependenceInfo *LDI, Parallelization &par) {
  auto findContaining = [&](Value *val) -> std::pair<DSWPTask *, SCC *> {
    for (auto techniqueTask : this->tasks) {
      auto task = (DSWPTask *)techniqueTask;
      for (auto scc : task->stageSCCs) if (scc->isInternal(val)) return std::make_pair(task, scc);
      for (auto scc : task->removableSCCs) if (scc->isInternal(val)) return std::make_pair(task, scc);
    }
    return std::make_pair(nullptr, nullptr);
  };

  for (auto bb : LDI->loopBBs){
    auto consumerTerm = bb->getTerminator();
    if (consumerTerm->getNumSuccessors() == 1) continue;
    auto consumerI = cast<Instruction>(bb->getTerminator());
    auto brStageSCC = findContaining(cast<Value>(consumerI));
    assert(brStageSCC.first != nullptr);

    for (auto edge : brStageSCC.second->fetchNode(cast<Value>(consumerI))->getIncomingEdges()) {
      if (edge->isControlDependence()) continue;
      auto producer = cast<Instruction>(edge->getOutgoingT());
      auto prodStageSCC = findContaining(cast<Value>(producer));
      DSWPTask *prodStage = prodStageSCC.first;
      SCC *prodSCC = prodStageSCC.second;
      assert(prodStage != nullptr);

      for (auto techniqueTask : this->tasks) {
        auto otherStage = (DSWPTask *)techniqueTask;

        /*
         * Register a queue if the producer isn't in the stage and the consumer is used
         */
        if (otherStage == prodStage) continue;
        if (otherStage->removableSCCs.find(prodSCC) != otherStage->removableSCCs.end()) continue;
        if (otherStage->usedCondBrs.find(consumerTerm) == otherStage->usedCondBrs.end()) continue;
        registerQueue(par, LDI, prodStage, otherStage, producer, consumerI);
      }
    }
  }
}

void DSWP::collectDataQueueInfo (LoopDependenceInfo *LDI, Parallelization &par) {
  for (auto techniqueTask : this->tasks) {
    auto toStage = (DSWPTask *)techniqueTask;
    std::set<SCC *> allSCCs(toStage->removableSCCs.begin(), toStage->removableSCCs.end());
    allSCCs.insert(toStage->stageSCCs.begin(), toStage->stageSCCs.end());
    for (auto scc : allSCCs) {
      for (auto sccEdge : LDI->sccdagAttrs.getSCCDAG()->fetchNode(scc)->getIncomingEdges()) {
        auto fromSCC = sccEdge->getOutgoingT();
        auto fromSCCInfo = LDI->sccdagAttrs.getSCCAttrs(fromSCC);
        if (fromSCCInfo->canBeCloned()) {
          continue;
        }
        auto fromStage = this->sccToStage[fromSCC];
        if (fromStage == toStage) continue;

        /*
         * Create value queues for each dependency of the form: producer -> consumers
         */
        for (auto instructionEdge : sccEdge->getSubEdges()) {
          assert(!instructionEdge->isMemoryDependence());
          if (instructionEdge->isControlDependence()) continue;
          auto producer = cast<Instruction>(instructionEdge->getOutgoingT());
          auto consumer = cast<Instruction>(instructionEdge->getIncomingT());
          registerQueue(par, LDI, fromStage, toStage, producer, consumer);
        }
      }
    }
  }
}

void DSWP::generateLoadsOfQueuePointers (
  Parallelization &par,
  int taskIndex
) {
  auto task = (DSWPTask *)this->tasks[taskIndex];
  IRBuilder<> entryBuilder(task->entryBlock);
  auto queuesArray = entryBuilder.CreateBitCast(
    task->queueArg,
    PointerType::getUnqual(this->queueArrayType)
  );

  /*
   * Load this stage's relevant queues
   */
  auto loadQueuePtrFromIndex = [&](int queueIndex) -> void {
    auto queueInfo = this->queues[queueIndex].get();
    auto queueIndexValue = cast<Value>(ConstantInt::get(par.int64, queueIndex));
    auto queuePtr = entryBuilder.CreateInBoundsGEP(queuesArray, ArrayRef<Value*>({
      this->zeroIndexForBaseArray,
      queueIndexValue
    }));
    auto parQueueIndex = par.queues.queueSizeToIndex[queueInfo->bitLength];
    auto queueType = par.queues.queueTypes[parQueueIndex];
    auto queueElemType = par.queues.queueElementTypes[parQueueIndex];
    auto queueCast = entryBuilder.CreateBitCast(queuePtr, PointerType::getUnqual(queueType));

    auto queueInstrs = std::make_unique<QueueInstrs>();
    queueInstrs->queuePtr = entryBuilder.CreateLoad(queueCast);
    queueInstrs->alloca = entryBuilder.CreateAlloca(queueInfo->dependentType);
    queueInstrs->allocaCast = entryBuilder.CreateBitCast(
      queueInstrs->alloca,
      PointerType::getUnqual(queueElemType)
    );
    task->queueInstrMap[queueIndex] = std::move(queueInstrs);
  };

  for (auto queueIndex : task->pushValueQueues) loadQueuePtrFromIndex(queueIndex);
  for (auto queueIndex : task->popValueQueues) loadQueuePtrFromIndex(queueIndex);
}

void DSWP::popValueQueues (Parallelization &par, int taskIndex) {
  auto task = (DSWPTask *)this->tasks[taskIndex];
  auto &bbClones = task->basicBlockClones;
  auto &iClones = task->instructionClones;

  for (auto queueIndex : task->popValueQueues) {
    auto &queueInfo = this->queues[queueIndex];
    auto queueInstrs = task->queueInstrMap[queueIndex].get();
    auto queueCallArgs = ArrayRef<Value*>({ queueInstrs->queuePtr, queueInstrs->allocaCast });

    auto bb = queueInfo->producer->getParent();
    assert(bbClones.find(bb) != bbClones.end());

    IRBuilder<> builder(bbClones[bb]);
    auto queuePopFunction = par.queues.queuePops[par.queues.queueSizeToIndex[queueInfo->bitLength]];
    queueInstrs->queueCall = builder.CreateCall(queuePopFunction, queueCallArgs);
    queueInstrs->load = builder.CreateLoad(queueInstrs->alloca);

    /*
     * Map from producer to queue load 
     */
    task->instructionClones[queueInfo->producer] = (Instruction *)queueInstrs->load;

    /*
     * Position queue call and load relatively identically to where the producer is in the basic block
     */
    bool pastProducer = false;
    bool moved = false;
    for (auto &I : *bb) {
      if (&I == queueInfo->producer) pastProducer = true;
      else if (auto phi = dyn_cast<PHINode>(&I)) continue;
      else if (pastProducer && iClones.find(&I) != iClones.end()) {
        auto iClone = iClones[&I];
        cast<Instruction>(queueInstrs->queueCall)->moveBefore(iClone);
        cast<Instruction>(queueInstrs->load)->moveBefore(iClone);
        moved = true;
        break;
      }
    }
    assert(moved);
  }
}

void DSWP::pushValueQueues (Parallelization &par, int taskIndex) {
  auto task = (DSWPTask *)this->tasks[taskIndex];
  auto &iClones = task->instructionClones;

  for (auto queueIndex : task->pushValueQueues) {
    auto queueInstrs = task->queueInstrMap[queueIndex].get();
    auto queueInfo = this->queues[queueIndex].get();
    auto queueCallArgs = ArrayRef<Value*>({ queueInstrs->queuePtr, queueInstrs->allocaCast });
    
    auto pClone = iClones[queueInfo->producer];
    auto pCloneBB = pClone->getParent();
    IRBuilder<> builder(pCloneBB);
    auto store = builder.CreateStore(pClone, queueInstrs->alloca);
    auto queuePushFunction = par.queues.queuePushes[par.queues.queueSizeToIndex[queueInfo->bitLength]];
    queueInstrs->queueCall = builder.CreateCall(queuePushFunction, queueCallArgs);

    bool pastProducer = false;
    for (auto &I : *pCloneBB) {
      if (&I == pClone) pastProducer = true;
      else if (auto phi = dyn_cast<PHINode>(&I)) continue;
      else if (pastProducer) {
        store->moveBefore(&I);
        cast<Instruction>(queueInstrs->queueCall)->moveBefore(&I);
        break;
      }
    }
  }
}
