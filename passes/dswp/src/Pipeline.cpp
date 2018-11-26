#include "DSWP.hpp"

using namespace llvm;

void DSWP::generateStagesFromPartitionedSCCs (DSWPLoopDependenceInfo *LDI) {
  std::vector<TaskExecution *> techniqueTasks;
  auto &depthOrdered = partitioner->getDepthOrderedSubsets();
  for (auto subset : depthOrdered) {

    /*
     * Create task (stage), populating its SCCs
     */
    auto task = new DSWPTaskExecution();
    techniqueTasks.push_back(task);
    for (auto scc : *subset) {
      task->stageSCCs.insert(scc);
      LDI->sccToStage[scc] = task;
    }
  }

  this->generateTasks(LDI, techniqueTasks);
  this->numTaskInstances = techniqueTasks.size();
  assert(this->numTaskInstances == subsets->size());
}

void DSWP::addRemovableSCCsToStages (DSWPLoopDependenceInfo *LDI) {
  for (auto techniqueTask : this->tasks) {
    auto task = (DSWPTaskExecution *)techniqueTask;
    std::set<DGNode<SCC> *> visitedNodes;
    std::queue<DGNode<SCC> *> dependentSCCNodes;

    for (auto scc : task->stageSCCs) {
      dependentSCCNodes.push(LDI->loopSCCDAG->fetchNode(scc));
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
        if (!LDI->sccdagAttrs.canBeCloned(fromSCC)) continue;

        task->removableSCCs.insert(fromSCC);
        dependentSCCNodes.push(fromSCCNode);
        visitedNodes.insert(fromSCCNode);
      }
    }
  }
}

void DSWP::createPipelineFromStages (DSWPLoopDependenceInfo *LDI, Parallelization &par) {

  /*
   * Fetch the module.
   */
  auto M = LDI->function->getParent();

  /*
   * Create a basic block in the original function where the parallelized loop exists.
   * This basic block will include code needed to execute the parallelized loop.
   */
  LDI->entryPointOfParallelizedLoop = BasicBlock::Create(M->getContext(), "", LDI->function);
  LDI->exitPointOfParallelizedLoop = LDI->entryPointOfParallelizedLoop;

  this->allocateEnvironmentArray(LDI);
  this->populateLiveInEnvironment(LDI);
  auto envPtr = envBuilder->getEnvArrayInt8Ptr();

  /*
   * Reference the stages in an array
   */
  IRBuilder<> *builder = new IRBuilder<>(LDI->entryPointOfParallelizedLoop);
  auto stagesPtr = createStagesArrayFromStages(LDI, *builder, par);

  /*
   * Allocate an array of integers.
   * Each integer represents the bitwidth of each queue that connects pipeline stages.
   */
  auto queueSizesPtr = createQueueSizesArrayFromStages(LDI, *builder, par);

  /*
   * Call the stage dispatcher with the environment, queues array, and stages array
   */
  auto queuesCount = cast<Value>(ConstantInt::get(par.int64, LDI->queues.size()));
  auto stagesCount = cast<Value>(ConstantInt::get(par.int64, this->numTaskInstances));

  /*
   * Add the call to the task dispatcher: "stageDispatcher" (see DSWP constructor)
   */
  builder->CreateCall(taskDispatcher, ArrayRef<Value*>({
    envPtr,
    queueSizesPtr,
    stagesPtr,
    stagesCount,
    queuesCount
  }));
  delete builder;

  this->propagateLiveOutEnvironment(LDI);
}

Value * DSWP::createStagesArrayFromStages (
  DSWPLoopDependenceInfo *LDI,
  IRBuilder<> funcBuilder,
  Parallelization &par
) {
  auto stagesAlloca = cast<Value>(funcBuilder.CreateAlloca(LDI->stageArrayType));
  auto stageCastType = PointerType::getUnqual(this->tasks[0]->F->getType());
  for (int i = 0; i < this->numTaskInstances; ++i) {
    auto stage = this->tasks[i];
    auto stageIndex = cast<Value>(ConstantInt::get(par.int64, i));
    auto stagePtr = funcBuilder.CreateInBoundsGEP(stagesAlloca, ArrayRef<Value*>({
      LDI->zeroIndexForBaseArray,
      stageIndex
    }));
    auto stageCast = funcBuilder.CreateBitCast(stagePtr, stageCastType);
    funcBuilder.CreateStore(stage->F, stageCast);
  }

  return cast<Value>(funcBuilder.CreateBitCast(stagesAlloca, PointerType::getUnqual(par.int8)));
}

Value * DSWP::createQueueSizesArrayFromStages (
  DSWPLoopDependenceInfo *LDI,
  IRBuilder<> funcBuilder,
  Parallelization &par
) {
  auto queuesAlloca = cast<Value>(funcBuilder.CreateAlloca(ArrayType::get(par.int64, LDI->queues.size())));
  for (int i = 0; i < LDI->queues.size(); ++i) {
    auto &queue = LDI->queues[i];
    auto queueIndex = cast<Value>(ConstantInt::get(par.int64, i));
    auto queuePtr = funcBuilder.CreateInBoundsGEP(queuesAlloca, ArrayRef<Value*>({
      LDI->zeroIndexForBaseArray,
      queueIndex
    }));
    auto queueCast = funcBuilder.CreateBitCast(queuePtr, PointerType::getUnqual(par.int64));
    funcBuilder.CreateStore(ConstantInt::get(par.int64, queue->bitLength), queueCast);
  }

  return cast<Value>(funcBuilder.CreateBitCast(queuesAlloca, PointerType::getUnqual(par.int64)));
}
