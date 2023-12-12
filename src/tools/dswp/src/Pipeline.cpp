/*
 * Copyright 2016 - 2023  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/tools/DSWP.hpp"

namespace arcana::noelle {

void DSWP::generateStagesFromPartitionedSCCs(LoopContent *LDI) {
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
  auto currentUserID = 0;

  /*
   * Create the tasks.
   */
  for (auto subset : depthOrdered) {

    /*
     * Fetch the function that executes a stage.
     */
    auto taskExecuter = program->getFunction("stageExecuter");
    assert(taskExecuter != nullptr);

    /*
     * Define its signature.
     */
    auto taskArgType = taskExecuter->arg_begin()->getType();
    auto taskSignature =
        cast<FunctionType>(cast<PointerType>(taskArgType)->getElementType());

    /*
     * Create task (stage), populating its SCCs
     */
    auto task = new DSWPTask(taskSignature, *program);
    this->fromTaskIDToUserID[task->getID()] = currentUserID;
    currentUserID++;
    techniqueTasks.push_back(task);
    for (auto scc : subset->sccs) {
      task->stageSCCs.insert(scc);
      this->sccToStage[scc] = task;
    }
  }
  this->addPredecessorAndSuccessorsBasicBlocksToTasks(LDI, techniqueTasks);
  this->numTaskInstances = techniqueTasks.size();
  assert(this->numTaskInstances == this->partitioner->numberOfPartitions());

  return;
}

void DSWP::addClonableSCCsToStages(LoopContent *LDI) {
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
        auto fromSCCNode = sccEdge->getSrcNode();
        auto fromSCC = fromSCCNode->getT();
        if (visitedNodes.find(fromSCCNode) != visitedNodes.end())
          continue;
        auto fromSCCInfo = sccManager->getSCCAttrs(fromSCC);
        if (this->canBeCloned(fromSCCInfo)) {
          task->clonableSCCs.insert(fromSCC);
        }

        dependentSCCNodes.push(fromSCCNode);
        visitedNodes.insert(fromSCCNode);
      }
    }
  }
}

bool DSWP::isCompleteAndValidStageStructure(LoopContent *LDI) const {
  std::set<SCC *> allSCCs;
  for (auto techniqueTask : this->tasks) {
    auto task = (DSWPTask *)techniqueTask;
    for (auto scc : task->stageSCCs) {
      if (allSCCs.find(scc) != allSCCs.end()) {
        errs()
            << "DSWP:  ERROR! A non-clonable SCC is present in more than one DSWP stage";
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
    auto sccAttrs = sccManager->getSCCAttrs(node->getT());
    if (this->canBeCloned(sccAttrs)) {
      continue;
    }

    if (allSCCs.find(node->getT()) == allSCCs.end()) {
      errs() << "DSWP:  ERROR! A loop's SCC is not present in any DSWP stage";
      return false;
    }
  }

  return true;
}

void DSWP::createPipelineFromStages(LoopContent *LDI, Noelle &par) {

  /*
   * Fetch the managers.
   */
  auto cm = par.getConstantsManager();

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
   * Each integer represents the bitwidth of each queue that connects pipeline
   * stages.
   */
  auto queueSizesPtr = createQueueSizesArrayFromStages(LDI, builder, par);

  /*
   * Call the stage dispatcher with the environment, queues array, and stages
   * array
   */
  auto queuesCount = cm->getIntegerConstant(this->queues.size(), 64);
  auto stagesCount = cm->getIntegerConstant(this->numTaskInstances, 64);

  /*
   * Add the call to the task dispatcher
   */
  auto runtimeCall = builder.CreateCall(
      taskDispatcher,
      ArrayRef<Value *>(
          { envPtr, queueSizesPtr, stagesPtr, stagesCount, queuesCount }));
  auto numThreadsUsed = builder.CreateExtractValue(runtimeCall, (uint64_t)0);

  /*
   * Propagate live-out values to the caller of the loop.
   */
  auto latestBBAfterCall =
      this->performReductionToAllReducableLiveOutVariables(LDI, numThreadsUsed);

  IRBuilder<> afterCallBuilder{ latestBBAfterCall };
  afterCallBuilder.CreateBr(this->exitPointOfParallelizedLoop);

  return;
}

Value *DSWP::createStagesArrayFromStages(LoopContent *LDI,
                                         IRBuilder<> funcBuilder,
                                         Noelle &par) {

  /*
   * Fetch the managers.
   */
  auto cm = par.getConstantsManager();
  auto tm = par.getTypesManager();

  auto stagesAlloca =
      cast<Value>(funcBuilder.CreateAlloca(this->stageArrayType));
  auto stageCastType =
      PointerType::getUnqual(this->tasks[0]->getTaskBody()->getType());
  for (int i = 0; i < this->numTaskInstances; ++i) {
    auto stage = this->tasks[i];
    auto stageIndex = cm->getIntegerConstant(i, 64);
    auto stagePtr = funcBuilder.CreateInBoundsGEP(
        stagesAlloca,
        ArrayRef<Value *>({ this->zeroIndexForBaseArray, stageIndex }));
    auto stageCast = funcBuilder.CreateBitCast(stagePtr, stageCastType);
    funcBuilder.CreateStore(stage->getTaskBody(), stageCast);
  }

  auto int8Type = tm->getIntegerType(8);

  return cast<Value>(
      funcBuilder.CreateBitCast(stagesAlloca,
                                PointerType::getUnqual(int8Type)));
}

Value *DSWP::createQueueSizesArrayFromStages(LoopContent *LDI,
                                             IRBuilder<> funcBuilder,
                                             Noelle &par) {

  /*
   * Fetch the managers.
   */
  auto cm = par.getConstantsManager();
  auto tm = par.getTypesManager();

  auto int64Type = tm->getIntegerType(64);
  auto queuesAlloca = cast<Value>(
      funcBuilder.CreateAlloca(ArrayType::get(int64Type, this->queues.size())));
  for (int i = 0; i < this->queues.size(); ++i) {
    auto &queue = this->queues[i];
    auto queueIndex = cm->getIntegerConstant(i, 64);
    auto queuePtr = funcBuilder.CreateInBoundsGEP(
        queuesAlloca,
        ArrayRef<Value *>({ this->zeroIndexForBaseArray, queueIndex }));
    auto queueCast =
        funcBuilder.CreateBitCast(queuePtr, PointerType::getUnqual(int64Type));
    funcBuilder.CreateStore(cm->getIntegerConstant(queue->bitLength, 64),
                            queueCast);
  }

  return cast<Value>(
      funcBuilder.CreateBitCast(queuesAlloca,
                                PointerType::getUnqual(int64Type)));
}

} // namespace arcana::noelle
