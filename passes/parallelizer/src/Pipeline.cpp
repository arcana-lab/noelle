#include "DSWP.hpp"

using namespace llvm;

void Parallelizer::createStagesfromPartitionedSCCs (DSWPLoopDependenceInfo *LDI) {
  auto topLevelParts = LDI->partition.topLevelSubsets();
  std::set<SCCDAGSubset *> partsFound(topLevelParts.begin(), topLevelParts.end());
  std::deque<SCCDAGSubset *> partsToTraverse(topLevelParts.begin(), topLevelParts.end());

  int order = 0;
  while (!partsToTraverse.empty())
  {
    auto part = partsToTraverse.front();
    partsToTraverse.pop_front();

    /*
     * Add all unvisited, next depth partitions to the traversal queue 
     */
    auto nextParts = LDI->partition.nextLevelSubsets(part);
    for (auto next : nextParts)
    {
      if (partsFound.find(next) != partsFound.end()) continue;
      partsFound.insert(next);
      partsToTraverse.push_back(next);
    }

    LDI->stages.push_back(std::move(std::make_unique<StageInfo>(order++)));
    auto stage = LDI->stages[order - 1].get();
    for (auto scc : part->SCCs) {
      stage->stageSCCs.insert(scc);
      LDI->sccToStage[scc] = stage;
    }
  }
}

void Parallelizer::createPipelineStageFromSCCDAGPartition (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo, Parallelization &par) {

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

void Parallelizer::createPipelineFromStages (DSWPLoopDependenceInfo *LDI, Parallelization &par) {

  /*
   * Fetch the module.
   */
  auto M = LDI->function->getParent();

  /*
   * Allocate the memory where pointers to variables will be stored.
   * These variables are those involved in dependences from code outside the loop to inside it.
   * Such variables are read by code inside the loop and updated just after the execution of the parallelized loop and before jumping to the code outside the loop.
   */
  auto firstBB = &*LDI->function->begin();
  IRBuilder<> funcBuilder(firstBB->getTerminator());
  LDI->envArray = funcBuilder.CreateAlloca(LDI->envArrayType);

  /*
   * Create a basic block in the original function where the parallelized loop exists.
   * This basic block will include code needed to execute the parallelized loop.
   */
  LDI->pipelineBB = BasicBlock::Create(M->getContext(), "", LDI->function);
  IRBuilder<> builder(LDI->pipelineBB);
  auto envPtr = createEnvArrayFromStages(LDI, funcBuilder, builder, par);
  auto stagesPtr = createStagesArrayFromStages(LDI, funcBuilder, par);

  /*
   * Allocate an array of integers.
   * Each integer represents the bitwidth of each queue that connects pipeline stages.
   */
  auto queueSizesPtr = createQueueSizesArrayFromStages(LDI, funcBuilder, par);

  /*
   * Call the stage dispatcher with the environment, queues array, and stages array
   */
  auto queuesCount = cast<Value>(ConstantInt::get(par.int64, LDI->queues.size()));
  auto stagesCount = cast<Value>(ConstantInt::get(par.int64, LDI->stages.size()));

  /*
   * Add the call to "stageDispatcher"
   */
  builder.CreateCall(stageDispatcher, ArrayRef<Value*>({ envPtr, queueSizesPtr, stagesPtr, stagesCount, queuesCount }));

  /*
   * Satisfy dependences from the code inside the loop to the code outside it.
   */
  storeOutgoingDependentsIntoExternalValues(LDI, builder, par);

  return ;
}
