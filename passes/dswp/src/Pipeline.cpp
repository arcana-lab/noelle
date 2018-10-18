#include "DSWP.hpp"

using namespace llvm;

void DSWP::createStagesFromPartitionedSCCs (DSWPLoopDependenceInfo *LDI) {
  auto topLevelSubIDs = LDI->partition.getSubsetIDsWithNoIncomingEdges();
  assert(topLevelSubIDs.size() > 0);
  std::set<int> subsFound(topLevelSubIDs.begin(), topLevelSubIDs.end());
  std::deque<int> subsToTraverse(topLevelSubIDs.begin(), topLevelSubIDs.end());

  int order = 0;
  while (!subsToTraverse.empty())
  {
    auto sub = subsToTraverse.front();
    subsToTraverse.pop_front();

    /*
     * Add all unvisited, next depth partitions to the traversal queue 
     */
    auto nextSubs = LDI->partition.nextLevelSubsetIDs(sub);
    for (auto next : nextSubs)
    {
      if (subsFound.find(next) != subsFound.end()) continue;
      subsFound.insert(next);
      subsToTraverse.push_back(next);
    }

    LDI->stages.push_back(std::move(std::make_unique<StageInfo>(order++)));
    auto stage = LDI->stages[order - 1].get();
    for (auto scc : LDI->partition.subsetOfID(sub)->SCCs) {
      stage->stageSCCs.insert(scc);
      LDI->sccToStage[scc] = stage;
    }
  }
}

void DSWP::createPipelineStageFromSCCDAGPartition (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo, Parallelization &par) {

  /*
   * Create a function where we will store all the code that will be executed for the current pipeline stage.
   */
  auto M = LDI->function->getParent();
  auto stageF = cast<Function>(M->getOrInsertFunction("", stageType));
  auto &context = M->getContext();
  stageInfo->sccStage = stageF;

  /*
   * Create the entry and exit basic blocks of the pipeline-stage function.
   */
  stageInfo->entryBlock = BasicBlock::Create(context, "", stageF);
  stageInfo->exitBlock = BasicBlock::Create(context, "", stageF);
  stageInfo->sccBBCloneMap[LDI->preHeader] = stageInfo->entryBlock;

  /*
   * Create one basic block per loop exit.
   * Also, add unconditional branches from each of these basic blocks to the unique exit block created before.
   */
  for (auto exitBB : LDI->loopExitBlocks) {
    auto newExitBB = BasicBlock::Create(context, "", stageF);
    stageInfo->loopExitBlocks.push_back(newExitBB);
    IRBuilder<> builder(newExitBB);
    builder.CreateBr(stageInfo->exitBlock);
  }

  /*
   * Add the instructions of the current pipeline stage to the related function.
   */
  createInstAndBBForSCC(LDI, stageInfo);

  /*
   * Add code at the entry point of the related function to load pointers of all queues for the current pipeline stage.
   */
  loadAllQueuePointersInEntry(LDI, stageInfo, par);

  /*
   * Add code to push values between the current pipeline stage and the connected ones.
   */
  popValueQueues(LDI, stageInfo, par);
  pushValueQueues(LDI, stageInfo, par);

  /*
   * Add the required loads and stores to satisfy dependences from the code outside the loop to the code inside it.
   */
  loadAndStoreEnv(LDI, stageInfo, par);

  /*
   * Link the cloned basic blocks by following the control flows of the original loop.
   */
  remapControlFlow(LDI, stageInfo);

  /*
   * Link the data flows through variables of the cloned instructions following the data flows of the original loop.
   */
  remapOperandsOfInstClones(LDI, stageInfo);

  /*
   * Add the unconditional branch from the entry basic block to the header of the loop.
   */
  IRBuilder<> entryBuilder(stageInfo->entryBlock);
  entryBuilder.CreateBr(stageInfo->sccBBCloneMap[LDI->header]);

  /*
   * Add the return instruction at the end of the exit basic block.
   */
  IRBuilder<> exitBuilder(stageInfo->exitBlock);
  exitBuilder.CreateRetVoid();

  /*
   * Inline recursively calls to queues.
   */
  inlineQueueCalls(LDI, stageInfo);

  if (this->verbose >= Verbosity::Pipeline) {
    stageInfo->sccStage->print(errs() << "Pipeline stage printout:\n"); errs() << "\n";
  }

  return ;
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

  this->createEnvironment(LDI);
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
  auto stagesCount = cast<Value>(ConstantInt::get(par.int64, LDI->stages.size()));

  /*
   * Add the call to "stageDispatcher"
   */
  builder->CreateCall(stageDispatcher, ArrayRef<Value*>({ envPtr, queueSizesPtr, stagesPtr, stagesCount, queuesCount }));
  delete builder;

  this->propagateLiveOutEnvironment(LDI);
}

Value * DSWP::createStagesArrayFromStages (DSWPLoopDependenceInfo *LDI, IRBuilder<> funcBuilder, Parallelization &par) {
  auto stagesAlloca = cast<Value>(funcBuilder.CreateAlloca(LDI->stageArrayType));
  auto stageCastType = PointerType::getUnqual(LDI->stages[0]->sccStage->getType());
  for (int i = 0; i < LDI->stages.size(); ++i) {
    auto &stage = LDI->stages[i];
    auto stageIndex = cast<Value>(ConstantInt::get(par.int64, i));
    auto stagePtr = funcBuilder.CreateInBoundsGEP(stagesAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, stageIndex }));
    auto stageCast = funcBuilder.CreateBitCast(stagePtr, stageCastType);
    funcBuilder.CreateStore(stage->sccStage, stageCast);
  }

  return cast<Value>(funcBuilder.CreateBitCast(stagesAlloca, PointerType::getUnqual(par.int8)));
}

Value * DSWP::createQueueSizesArrayFromStages (DSWPLoopDependenceInfo *LDI, IRBuilder<> funcBuilder, Parallelization &par) {
  auto queuesAlloca = cast<Value>(funcBuilder.CreateAlloca(ArrayType::get(par.int64, LDI->queues.size())));
  for (int i = 0; i < LDI->queues.size(); ++i) {
    auto &queue = LDI->queues[i];
    auto queueIndex = cast<Value>(ConstantInt::get(par.int64, i));
    auto queuePtr = funcBuilder.CreateInBoundsGEP(queuesAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, queueIndex }));
    auto queueCast = funcBuilder.CreateBitCast(queuePtr, PointerType::getUnqual(par.int64));
    funcBuilder.CreateStore(ConstantInt::get(par.int64, queue->bitLength), queueCast);
  }

  return cast<Value>(funcBuilder.CreateBitCast(queuesAlloca, PointerType::getUnqual(par.int64)));
}
