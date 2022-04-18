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
using namespace llvm::noelle;

void DSWP::registerQueue (
  Noelle &par,
  LoopDependenceInfo *LDI,
  DSWPTask *fromStage,
  DSWPTask *toStage,
  Instruction *producer,
  Instruction *consumer,
  bool isMemoryDependence
) {

  /*
   * Find/create the push queue in the producer stage
   */
  int queueIndex = this->queues.size();
  QueueInfo *queueInfo = nullptr;
  for (auto queueI : fromStage->producerToQueues[producer]) {
    if (this->queues[queueI]->toStage != toStage->getID()) continue;
    queueIndex = queueI;
    queueInfo = this->queues[queueIndex].get();
    break;
  }
  if (queueIndex == this->queues.size()) {
    this->queues.push_back(std::move(std::make_unique<QueueInfo>(producer, consumer, producer->getType(), isMemoryDependence)));
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
  queueInfo->fromStage = fromStage->getID();
  queueInfo->toStage = toStage->getID();

  return ;
}

void DSWP::collectControlQueueInfo (LoopDependenceInfo *LDI, Noelle &par) {

  /*
   * Fetch the SCCDAG.
   */
  auto sccManager = LDI->getSCCManager();
  auto sccdag = sccManager->getSCCDAG();

  std::set<DGNode<Value> *> conditionalBranchNodes;
  auto loopExitBlocks = LDI->getLoopStructure()->getLoopExitBasicBlocks();
  std::set<BasicBlock *> loopExitBlockSet(loopExitBlocks.begin(), loopExitBlocks.end());

  for (auto sccNode : sccdag->getNodes()) {
    auto scc = sccNode->getT();

    for (auto controlEdge : scc->getEdges()) {
      if (!controlEdge->isControlDependence()) continue;

      auto controlNode = controlEdge->getOutgoingNode();
      auto controlSCC = sccdag->sccOfValue(controlNode->getT());
      if (sccManager->getSCCAttrs(controlSCC)->canBeCloned()) continue;

      /*
       * Check if the controlling instruction has a data dependence requiring a queue
       */
      bool hasDataDependency = false;
      for (auto conditionOrReturnValueDependency : controlNode->getIncomingEdges()) {
        if (conditionOrReturnValueDependency->isControlDependence()) continue;
        hasDataDependency = true;
        break;
      }
      if (!hasDataDependency) continue;

      conditionalBranchNodes.insert(controlNode);
    }
  }

  for (auto conditionalBranchNode : conditionalBranchNodes) {

    /*
     * Identify the single condition for this conditional branch
     * FIXME: Figure out how to handle more complex terminators
     */
    std::set<Instruction *> conditionsOfConditionalBranch;
    for (auto conditionToBranchDependency : conditionalBranchNode->getIncomingEdges()) {
      assert(!conditionToBranchDependency->isMemoryDependence()
        && "Node producing control dependencies is expected not to consume a memory dependence");
      if (conditionToBranchDependency->isControlDependence()) continue;

      auto condition = conditionToBranchDependency->getOutgoingT();
      auto conditionSCC = sccdag->sccOfValue(condition);
      if (sccManager->getSCCAttrs(conditionSCC)->canBeCloned()) continue;

      conditionsOfConditionalBranch.insert(cast<Instruction>(condition));
    }
    assert(conditionsOfConditionalBranch.size() == 1);

    auto conditionalBranch = cast<Instruction>(conditionalBranchNode->getT());
    auto branchBB = conditionalBranch->getParent();
    bool isControllingLoopExit = false;
    for (auto succBB = succ_begin(branchBB); succBB != succ_end(branchBB); ++succBB) {
      isControllingLoopExit |= loopExitBlockSet.find(*succBB) != loopExitBlockSet.end();
    }

    /*
     * Determine which tasks are control dependent on the conditional branch
     */
    std::set<Task *> tasksControlledByCondition;
    if (isControllingLoopExit) {
      tasksControlledByCondition = std::set<Task *>(this->tasks.begin(), this->tasks.end());
    } else {
      tasksControlledByCondition = collectTransitivelyControlledTasks(LDI, conditionalBranchNode);
    }

    /*
     * For each task, add a queue from the condition to the branch
     */
    auto taskOfCondition = this->sccToStage.at(sccdag->sccOfValue(conditionalBranch));
    for (auto techniqueTask : tasksControlledByCondition) {
      auto taskControlledByCondition = (DSWPTask *)techniqueTask;
      if (taskOfCondition == taskControlledByCondition) continue;

      for (auto condition : conditionsOfConditionalBranch) {
        registerQueue(par, LDI, taskOfCondition, taskControlledByCondition, condition, conditionalBranch, false);
      }
    }
  }
}

std::set<Task *> DSWP::collectTransitivelyControlledTasks (
  LoopDependenceInfo *LDI,
  DGNode<Value> *conditionalBranchNode
) {
  std::set<Task *> tasksControlledByCondition;
  auto sccManager = LDI->getSCCManager();
  SCCDAG *sccdag = sccManager->getSCCDAG();
  auto getTaskOfNode = [this, sccManager, sccdag](DGNode<SCC> *node) -> Task * {
    if (sccManager->getSCCAttrs(node->getT())->canBeCloned()) return nullptr;
    return this->sccToStage.at(node->getT());
  };

  std::queue<DGNode<SCC> *> queuedNodes;
  std::set<DGNode<SCC> *> visitedNodes;
  DGNode<SCC> *controllingNode = sccdag->fetchNode(sccdag->sccOfValue(conditionalBranchNode->getT()));
  Task *controllingTask = getTaskOfNode(controllingNode);
  queuedNodes.push(controllingNode);

  while (!queuedNodes.empty()) {
    auto node = queuedNodes.front();
    queuedNodes.pop();
    if (visitedNodes.find(node) != visitedNodes.end()) continue;
    visitedNodes.insert(node);

    /*
     * Iterate the next set of dependent instructions and collect their tasks
     * Enqueue dependent instructions in tasks not already visited
     */
    for (auto dependencyEdge : node->getOutgoingEdges()) {
      auto dependentNode = dependencyEdge->getIncomingNode();
      queuedNodes.push(dependentNode);

      Task *dependentTask = getTaskOfNode(dependentNode);
      if (dependentTask) {
        tasksControlledByCondition.insert(dependentTask);
      }
    }
  }

  /*
   * A task containing the conditional branch does not need a control queue
   */ 
  tasksControlledByCondition.erase(controllingTask);

  return tasksControlledByCondition;
}

void DSWP::collectDataAndMemoryQueueInfo (LoopDependenceInfo *LDI, Noelle &par) {

  auto sccManager = LDI->getSCCManager();
  for (auto techniqueTask : this->tasks) {
    auto toStage = (DSWPTask *)techniqueTask;
    std::set<SCC *> allSCCs(toStage->clonableSCCs.begin(), toStage->clonableSCCs.end());
    allSCCs.insert(toStage->stageSCCs.begin(), toStage->stageSCCs.end());

    for (auto scc : allSCCs) {
      for (auto sccEdge : sccManager->getSCCDAG()->fetchNode(scc)->getIncomingEdges()) {
        auto fromSCC = sccEdge->getOutgoingT();
        auto fromSCCInfo = sccManager->getSCCAttrs(fromSCC);
        if (fromSCCInfo->canBeCloned()) {
          continue;
        }

        auto fromStage = this->sccToStage[fromSCC];
        if (fromStage == toStage) continue;

        /*
         * Create value queues for each dependency of the form: producer -> consumers
         */
        for (auto instructionEdge : sccEdge->getSubEdges()) {
          if (instructionEdge->isControlDependence()) continue;

          auto producer = cast<Instruction>(instructionEdge->getOutgoingT());
          auto consumer = cast<Instruction>(instructionEdge->getIncomingT());

          /*
           * TODO: Handle memory dependencies and enable synchronization queues
           */
          auto isMemoryDependence = instructionEdge->isMemoryDependence();
          assert(!isMemoryDependence && "FIXME: Support memory synchronization with queues");

          registerQueue(par, LDI, fromStage, toStage, producer, consumer, isMemoryDependence);
        }
      }
    }
  }
}

bool DSWP::areQueuesAcyclical () const {

  /*
   * For each of the ordered vector of tasks:
   * 1) ensure that push queues do not loop back to a previous task
   * 2) ensure that pop queues do not loop forward to a following task
   */
  for (int i = 0; i < this->tasks.size(); ++i) {
    DSWPTask *task = (DSWPTask *)this->tasks[i];

    for (auto queueIdx : task->pushValueQueues) {
      int toTaskIdx = this->queues[queueIdx]->toStage;
      if (toTaskIdx <= i) {
        errs() << "DSWP:  ERROR! Push queue " << queueIdx << " loops back from stage "
          << i << " to stage " << toTaskIdx;
        return false;
      }
    }

    for (auto queueIdx : task->popValueQueues) {
      int fromTaskIdx = this->queues[queueIdx]->fromStage;
      if (fromTaskIdx >= i) {
        errs() << "DSWP:  ERROR! Pop queue " << queueIdx << " goes from stage "
          << fromTaskIdx << " to stage " << i;
        return false;
      }
    }
  }

  return true;
}

void DSWP::generateLoadsOfQueuePointers (
  Noelle &par,
  int taskIndex
) {
  auto task = (DSWPTask *)this->tasks[taskIndex];
  IRBuilder<> entryBuilder(task->getEntry());
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

void DSWP::popValueQueues (LoopDependenceInfo *LDI, Noelle &par, int taskIndex) {
  auto task = (DSWPTask *)this->tasks[taskIndex];

  for (auto queueIndex : task->popValueQueues) {
    auto &queueInfo = this->queues[queueIndex];
    auto queueInstrs = task->queueInstrMap[queueIndex].get();
    auto queueCallArgs = ArrayRef<Value*>({ queueInstrs->queuePtr, queueInstrs->allocaCast });

    /*
     * Determine the clone of the basic block of the original producer
     * Insert load right there
     */
    auto originalB = queueInfo->producer->getParent();
    assert(task->isAnOriginalBasicBlock(originalB));
    auto clonedB = task->getCloneOfOriginalBasicBlock(originalB);
    Instruction *insertionPoint = clonedB->getFirstNonPHIOrDbgOrLifetime();
    IRBuilder<> builder(insertionPoint);
    auto queuePopFunction = par.queues.queuePops[par.queues.queueSizeToIndex[queueInfo->bitLength]];
    queueInstrs->queueCall = builder.CreateCall(queuePopFunction, queueCallArgs);
    queueInstrs->load = builder.CreateLoad(queueInstrs->alloca);

    /*
     * Map from producer to queue load 
     */
    task->addInstruction(queueInfo->producer, cast<Instruction>(queueInstrs->load));
  }
}

void DSWP::pushValueQueues (LoopDependenceInfo *LDI, Noelle &par, int taskIndex) {
  auto task = (DSWPTask *)this->tasks[taskIndex];

  for (auto queueIndex : task->pushValueQueues) {
    auto queueInstrs = task->queueInstrMap[queueIndex].get();
    auto queueInfo = this->queues[queueIndex].get();
    auto queueCallArgs = ArrayRef<Value*>({ queueInstrs->queuePtr, queueInstrs->allocaCast });
    auto queuePushFunction = par.queues.queuePushes[par.queues.queueSizeToIndex[queueInfo->bitLength]];

    /*
     * Store the produced value immediately
     * Push the value immediately
     */
    auto producerBlock = queueInfo->producer->getParent();
    auto producerClone = task->getCloneOfOriginalInstruction(queueInfo->producer);
    auto producerCloneBlock = producerClone->getParent();
    auto insertPoint = producerClone->getNextNode();
    if (isa<PHINode>(insertPoint)) {
      insertPoint = producerCloneBlock->getFirstNonPHIOrDbgOrLifetime();
    }
    IRBuilder<> builder(insertPoint);
    builder.CreateStore(producerClone, queueInstrs->alloca);
    queueInstrs->queueCall = builder.CreateCall(queuePushFunction, queueCallArgs);

  }
}
