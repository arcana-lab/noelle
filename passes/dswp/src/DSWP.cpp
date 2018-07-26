#include "DSWP.hpp"

using namespace llvm;

bool DSWP::applyDSWP (DSWPLoopDependenceInfo *LDI, Parallelization &par, Heuristics *h) {

  /*
   * Partition the SCCDAG.
   */
  partitionSCCDAG(LDI);

  /*
   * Check whether it is worth parallelizing the current loop.
   */
  if (!isWorthParallelizing(LDI)) {
    if (this->verbose > Verbosity::Disabled) {
      errs() << "DSWP:  Not enough TLP can be extracted\n";
      errs() << "DSWP: Exit\n";
    }
    return false;
  }

  /*
   * Collect require information to parallelize the current loop.
   */
  collectStageAndQueueInfo(LDI, par);
  if (this->verbose >= Verbosity::Maximal) {
    printStageSCCs(LDI);
    printStageQueues(LDI);
    printEnv(LDI);
  }

  /*
   * Create the pipeline stages.
   */
  if (this->verbose > Verbosity::Disabled) {
    errs() << "DSWP:  Create " << LDI->stages.size() << " pipeline stages\n";
  }
  for (auto &stage : LDI->stages) {
    createPipelineStageFromSCCDAGPartition(LDI, stage, par);
  }

  /*
   * Create the whole pipeline by connecting the stages.
   */
  if (this->verbose > Verbosity::Disabled) {
    errs() << "DSWP:  Link pipeline stages\n";
  }
  createPipelineFromStages(LDI, par);
  assert(LDI->pipelineBB != nullptr);

  return true;
}

bool DSWP::isWorthParallelizing (DSWPLoopDependenceInfo *LDI) {
  if (this->forceParallelization){
    return true;
  }

  return LDI->partitions.partitions.size() > 1;
}

void DSWP::configureDependencyStorage (DSWPLoopDependenceInfo *LDI, Parallelization &par) {
  LDI->zeroIndexForBaseArray = cast<Value>(ConstantInt::get(par.int64, 0));
  LDI->envArrayType = ArrayType::get(PointerType::getUnqual(par.int8), LDI->environment->envSize());
  LDI->queueArrayType = ArrayType::get(PointerType::getUnqual(par.int8), LDI->queues.size());
  LDI->stageArrayType = ArrayType::get(PointerType::getUnqual(par.int8), LDI->stages.size());

  return ;
}

void DSWP::collectStageAndQueueInfo (DSWPLoopDependenceInfo *LDI, Parallelization &par) {
  createStagesfromPartitionedSCCs(LDI);
  addRemovableSCCsToStages(LDI);

  collectPartitionedSCCQueueInfo(LDI);
  trimCFGOfStages(LDI);
  collectControlQueueInfo(LDI);
  collectRemovableSCCQueueInfo(LDI);

  LDI->environment = std::make_unique<EnvInfo>();
  LDI->environment->exitBlockType = par.int32;
  collectPreLoopEnvInfo(LDI);
  collectPostLoopEnvInfo(LDI);

  configureDependencyStorage(LDI, par);

  return ;
}

Value * DSWP::createEnvArrayFromStages (DSWPLoopDependenceInfo *LDI, IRBuilder<> funcBuilder, IRBuilder<> builder, Parallelization &par) {

  /*
   * Create empty environment array for producers, exit block tracking
   */
  std::vector<Value*> envPtrs;
  for (int i = 0; i < LDI->environment->envSize(); ++i) {
    Type *envType = LDI->environment->typeOfEnv(i);
    auto varAlloca = funcBuilder.CreateAlloca(envType);
    envPtrs.push_back(varAlloca);
    auto envIndex = cast<Value>(ConstantInt::get(par.int64, i));
    auto envPtr = funcBuilder.CreateInBoundsGEP(LDI->envArray, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndex }));
    auto depCast = funcBuilder.CreateBitCast(envPtr, PointerType::getUnqual(PointerType::getUnqual(envType)));
    funcBuilder.CreateStore(varAlloca, depCast);
  }

  /*
   * Insert pre-loop producers into the environment array
   */
  for (int envIndex : LDI->environment->preLoopEnv) {
    builder.CreateStore(LDI->environment->envProducers[envIndex], envPtrs[envIndex]);
  }
  
  return cast<Value>(builder.CreateBitCast(LDI->envArray, PointerType::getUnqual(par.int8)));
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
