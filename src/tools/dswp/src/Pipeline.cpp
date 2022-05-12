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

void DSWP::generateStagesFromPartitionedSCCs (LoopDependenceInfo *LDI) {
  assert(LDI != nullptr);

  /*
   * Fetch the program.
   */
  auto program = this->noelle.getProgram();

  /*
   * Fetch the identified stages.
   */
  std::vector<Task *> techniqueTasks;
  auto depthOrdered = this->partitioner->getDepthOrderedSets();
  auto taskID = 0;

  /*
   * Create the tasks.
   */
  for (auto subset : depthOrdered) {

    /*
     * Create task (stage), populating its SCCs
     */
    auto task = new DSWPTask(taskID, this->taskSignature, *program);
    taskID++;
    techniqueTasks.push_back(task);
    for (auto scc : subset->sccs) {
      task->stageSCCs.insert(scc);
      this->sccToStage[scc] = task;
    }
  }
  this->addPredecessorAndSuccessorsBasicBlocksToTasks(LDI, techniqueTasks);
  this->numTaskInstances = techniqueTasks.size();
  assert(this->numTaskInstances == this->partitioner->numberOfPartitions());

  return ;
}

void DSWP::addClonableSCCsToStages (LoopDependenceInfo *LDI) {
  auto sccManager = LDI->getSCCManager();
  for (auto techniqueTask : this->tasks) {
    auto task = (DSWPTask *)techniqueTask;
    std::set<DGNode<SCC> *> visitedNodes;
    std::queue<DGNode<SCC> *> dependentSCCNodes;

    for (auto scc : task->stageSCCs) {
      dependentSCCNodes.push(sccManager->getSCCDAG()->fetchNode(scc));
    }

    while (!dependentSCCNodes.empty()) {
      auto depSCCNode = dependentSCCNodes.front();
      dependentSCCNodes.pop();

      /*
       * Collect clonable SCCs with outgoing edges to SCCs in the task
       */
      for (auto sccEdge : depSCCNode->getIncomingEdges()) {
        auto fromSCCNode = sccEdge->getOutgoingNode();
        auto fromSCC = fromSCCNode->getT();
        if (visitedNodes.find(fromSCCNode) != visitedNodes.end()) continue;
        auto fromSCCInfo = sccManager->getSCCAttrs(fromSCC);
        if (fromSCCInfo->canBeCloned()) {
          task->clonableSCCs.insert(fromSCC);
        }

        dependentSCCNodes.push(fromSCCNode);
        visitedNodes.insert(fromSCCNode);
      }
    }
  }
}

bool DSWP::isCompleteAndValidStageStructure (LoopDependenceInfo *LDI) const {
  std::set<SCC *> allSCCs;
  for (auto techniqueTask : this->tasks) {
    auto task = (DSWPTask *)techniqueTask;
    for (auto scc : task->stageSCCs) {
      if (allSCCs.find(scc) != allSCCs.end()) {
        errs() << "DSWP:  ERROR! A non-clonable SCC is present in more than one DSWP stage";
        return false;
      }
      allSCCs.insert(scc);
    }

    for (auto scc : task->clonableSCCs) {
      allSCCs.insert(scc);
    }
  }

  auto sccManager = LDI->getSCCManager();
  for (auto node : sccManager->getSCCDAG()->getNodes()) {
    if (sccManager->getSCCAttrs(node->getT())->canBeCloned()) {
      continue ;
    }

    if (allSCCs.find(node->getT()) == allSCCs.end()) {
      errs() << "DSWP:  ERROR! A loop's SCC is not present in any DSWP stage";
      return false;
    }
  }

  return true;
}

void DSWP::createPipelineFromStages (LoopDependenceInfo *LDI, Noelle &par) {

  /*
   * Fetch the loop function.
   */
  auto loopSummary = LDI->getLoopStructure();
  auto loopFunction = loopSummary->getFunction();

  /*
   * Fetch the module.
   */
  auto M = loopFunction->getParent();

  /*
   * Allocate the environment array and add its live-in values.
   */
  this->allocateEnvironmentArray(LDI);
  this->populateLiveInEnvironment(LDI);
  auto envPtr = envBuilder->getEnvironmentArrayVoidPtr();

  /*
   * Reference the stages in an array
   */
  IRBuilder<> builder(this->entryPointOfParallelizedLoop);
  auto stagesPtr = createStagesArrayFromStages(LDI, builder, par);

  /*
   * Allocate an array of integers.
   * Each integer represents the bitwidth of each queue that connects pipeline stages.
   */
  auto queueSizesPtr = createQueueSizesArrayFromStages(LDI, builder, par);

  /*
   * Call the stage dispatcher with the environment, queues array, and stages array
   */
  auto queuesCount = cast<Value>(ConstantInt::get(par.int64, this->queues.size()));
  auto stagesCount = cast<Value>(ConstantInt::get(par.int64, this->numTaskInstances));

  /*
   * Add the call to the task dispatcher
   */
  auto runtimeCall = builder.CreateCall(taskDispatcher, ArrayRef<Value*>({
    envPtr,
    queueSizesPtr,
    stagesPtr,
    stagesCount,
    queuesCount
  }));
  auto numThreadsUsed = builder.CreateExtractValue(runtimeCall, (uint64_t)0);

  /*
   * Propagate live-out values to the caller of the loop.
   */
  auto latestBBAfterCall = this->performReductionToAllReducableLiveOutVariables(LDI, numThreadsUsed);

  IRBuilder<> afterCallBuilder{latestBBAfterCall};
  afterCallBuilder.CreateBr(this->exitPointOfParallelizedLoop);

  return ;
}

Value * DSWP::createStagesArrayFromStages (
  LoopDependenceInfo *LDI,
  IRBuilder<> funcBuilder,
  Noelle &par
) {
  auto stagesAlloca = cast<Value>(funcBuilder.CreateAlloca(this->stageArrayType));
  auto stageCastType = PointerType::getUnqual(this->tasks[0]->getTaskBody()->getType());
  for (int i = 0; i < this->numTaskInstances; ++i) {
    auto stage = this->tasks[i];
    auto stageIndex = cast<Value>(ConstantInt::get(par.int64, i));
    auto stagePtr = funcBuilder.CreateInBoundsGEP(stagesAlloca, ArrayRef<Value*>({
      this->zeroIndexForBaseArray,
      stageIndex
    }));
    auto stageCast = funcBuilder.CreateBitCast(stagePtr, stageCastType);
    funcBuilder.CreateStore(stage->getTaskBody(), stageCast);
  }

  return cast<Value>(funcBuilder.CreateBitCast(stagesAlloca, PointerType::getUnqual(par.int8)));
}

Value * DSWP::createQueueSizesArrayFromStages (
  LoopDependenceInfo *LDI,
  IRBuilder<> funcBuilder,
  Noelle &par
) {
  auto queuesAlloca = cast<Value>(funcBuilder.CreateAlloca(ArrayType::get(par.int64, this->queues.size())));
  for (int i = 0; i < this->queues.size(); ++i) {
    auto &queue = this->queues[i];
    auto queueIndex = cast<Value>(ConstantInt::get(par.int64, i));
    auto queuePtr = funcBuilder.CreateInBoundsGEP(queuesAlloca, ArrayRef<Value*>({
      this->zeroIndexForBaseArray,
      queueIndex
    }));
    auto queueCast = funcBuilder.CreateBitCast(queuePtr, PointerType::getUnqual(par.int64));
    funcBuilder.CreateStore(ConstantInt::get(par.int64, queue->bitLength), queueCast);
  }

  return cast<Value>(funcBuilder.CreateBitCast(queuesAlloca, PointerType::getUnqual(par.int64)));
}
