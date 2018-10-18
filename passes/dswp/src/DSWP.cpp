#include "DSWP.hpp"

using namespace llvm;

DSWP::DSWP (Module &module, bool forceParallelization, bool enableSCCMerging, Verbosity v)
  :
  ParallelizationTechnique{module, v},
  forceParallelization{forceParallelization},
  enableMergingSCC{enableSCCMerging}
  {

  /*
   * Fetch the function that dispatch the parallelized loop.
   */
  this->stageDispatcher = module.getFunction("stageDispatcher");

  /*
   * Fetch the function that executes a stage.
   */
  auto stageExecuter = module.getFunction("stageExecuter");

  /*
   * Define its signature.
   */
  auto stageArgType = stageExecuter->arg_begin()->getType();
  this->stageType = cast<FunctionType>(cast<PointerType>(stageArgType)->getElementType());

  return ;
}
      
bool DSWP::canBeAppliedToLoop (LoopDependenceInfoForParallelizer *baseLDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) const {
  return true;
}

bool DSWP::apply (LoopDependenceInfoForParallelizer *baseLDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) {
  this->initEnvBuilder(baseLDI);

  /*
   * Fetch the LDI.
   */
  auto LDI = static_cast<DSWPLoopDependenceInfo *>(baseLDI);
  if (LDI == nullptr){
    return false;
  }

  /*
   * Partition the SCCDAG.
   */
  partitionSCCDAG(LDI, h);

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

  return true;
}

bool DSWP::isWorthParallelizing (DSWPLoopDependenceInfo *LDI) {
  if (this->forceParallelization){
    return true;
  }

  return LDI->partition.subsets.size() > 1;
}

void DSWP::createEnvironment (LoopDependenceInfoForParallelizer  *LDI) {
  ParallelizationTechnique::createEnvironment(LDI);

  IRBuilder<> builder(LDI->entryPointOfParallelizedLoop);
  envBuilder->createEnvArray(builder);
  std::set<int> nonReducableVars;
  std::set<int> reducableVars;
  for (auto i = 0; i < LDI->environment->envSize(); ++i) nonReducableVars.insert(i);

  envBuilder->allocateEnvVariables(builder, nonReducableVars, reducableVars, 0);
}

void DSWP::configureDependencyStorage (DSWPLoopDependenceInfo *LDI, Parallelization &par) {
  LDI->zeroIndexForBaseArray = cast<Value>(ConstantInt::get(par.int64, 0));
  LDI->queueArrayType = ArrayType::get(PointerType::getUnqual(par.int8), LDI->queues.size());
  LDI->stageArrayType = ArrayType::get(PointerType::getUnqual(par.int8), LDI->stages.size());
}

void DSWP::collectStageAndQueueInfo (DSWPLoopDependenceInfo *LDI, Parallelization &par) {
  createStagesFromPartitionedSCCs(LDI);
  addRemovableSCCsToStages(LDI);

  collectPartitionedSCCQueueInfo(par, LDI);
  trimCFGOfStages(LDI);
  collectControlQueueInfo(par, LDI);
  collectRemovableSCCQueueInfo(par, LDI);

  envBuilder->createEnvUsers(LDI->stages.size());
  collectPreLoopEnvInfo(LDI);
  collectPostLoopEnvInfo(LDI);

  configureDependencyStorage(LDI, par);
}

void DSWP::propagateLiveOutEnvironment (LoopDependenceInfoForParallelizer *LDI) {
  ParallelizationTechnique::propagateLiveOutEnvironment(LDI);
}
