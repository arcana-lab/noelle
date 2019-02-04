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

DSWP::DSWP (
  Module &module,
  bool forceParallelization,
  bool enableSCCMerging,
  Verbosity v,
  int coresPerOverride
) :
  ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences{module, v},
  forceParallelization{forceParallelization},
  enableMergingSCC{enableSCCMerging},
  coresPerLoopOverride{coresPerOverride}
  {

  /*
   * Fetch the function that dispatch the parallelized loop.
   */
  this->taskDispatcher = module.getFunction("stageDispatcher");

  /*
   * Fetch the function that executes a stage.
   */
  auto taskExecuter = module.getFunction("stageExecuter");

  /*
   * Define its signature.
   */
  auto taskArgType = taskExecuter->arg_begin()->getType();
  this->taskType = cast<FunctionType>(cast<PointerType>(taskArgType)->getElementType());

  return ;
}

void DSWP::initialize (LoopDependenceInfo *baseLDI, Heuristics *h) {
  auto LDI = static_cast<DSWPLoopDependenceInfo *>(baseLDI);
  if (coresPerLoopOverride > 0) {
    LDI->maximumNumberOfCoresForTheParallelization = coresPerLoopOverride;
  }
  partitionSCCDAG(LDI, h);
}

bool DSWP::canBeAppliedToLoop (
  LoopDependenceInfo *baseLDI,
  Parallelization &par,
  Heuristics *h,
  ScalarEvolution &SE
) const {
  auto canApply = this->partition->numberOfPartitions() > 1;
  if (this->forceParallelization) {
    if (!canApply && this->verbose != Verbosity::Disabled) {
      errs() << "DSWP:  Forced parallelization of a disadvantageous loop\n";
    }
    return true;
  }

  /*
   * Check whether it is worth parallelizing the current loop.
   */
  if (!canApply && this->verbose != Verbosity::Disabled) {
    errs() << "DSWP:  Not enough TLP can be extracted\n";
    errs() << "DSWP: Exit\n";
  }

  return canApply;
}

bool DSWP::apply (
  LoopDependenceInfo *baseLDI,
  Parallelization &par,
  Heuristics *h,
  ScalarEvolution &SE
) {
  auto LDI = static_cast<DSWPLoopDependenceInfo *>(baseLDI);

  /*
   * Determine DSWP tasks (stages)
   */
  generateStagesFromPartitionedSCCs(LDI);
  addRemovableSCCsToStages(LDI);

  /*
   * Collect which queues need to exist between tasks
   *
   * NOTE: The trimming of the call graph for all tasks is an optimization
   *  that lessens the number of control queues necessary. However,
   *  the algorithm that pops queue values is naive, so the trimming
   *  optimization requires non-control queue information to be collected
   *  prior to its execution. Hence, its weird placement:
   */
  collectDataQueueInfo(LDI, par);
  trimCFGOfStages(LDI);
  collectControlQueueInfo(LDI, par);

  /*
   * Collect information on stages' environments
   */
  std::set<int> nonReducableVars;
  std::set<int> reducableVars;
  for (auto i = 0; i < LDI->environment->envSize(); ++i) nonReducableVars.insert(i);
  initializeEnvironmentBuilder(LDI, nonReducableVars, reducableVars);
  collectLiveInEnvInfo(LDI);
  collectLiveOutEnvInfo(LDI);

  if (this->verbose >= Verbosity::Maximal) {
    printStageSCCs(LDI);
  }
  if (this->verbose >= Verbosity::Minimal) {
    printStageQueues(LDI);
    printEnv(LDI);
  }
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DSWP:  Create " << this->tasks.size() << " pipeline stages\n";
  }

  /*
   * Helper declarations
   */
  LDI->zeroIndexForBaseArray = cast<Value>(ConstantInt::get(par.int64, 0));
  LDI->queueArrayType = ArrayType::get(PointerType::getUnqual(par.int8), LDI->queues.size());
  LDI->stageArrayType = ArrayType::get(PointerType::getUnqual(par.int8), this->tasks.size());

  /*
   * Create the pipeline stages (technique tasks)
   */
  for (auto i = 0; i < this->tasks.size(); ++i) {
    auto task = (DSWPTask *)this->tasks[i];

    /*
     * Add instructions of the current pipeline stage to the task function
     */
    generateLoopSubsetForStage(LDI, i);

    /*
     * Load pointers of all queues for the current pipeline stage at the function's entry
     */
    generateLoadsOfQueuePointers(LDI, par, i);

    /*
     * Add push/pop operations from queues between the current pipeline stage and the connected ones
     */
    popValueQueues(LDI, par, i);
    pushValueQueues(LDI, par, i);

    /*
     * Load all loop live-in values at the entry point of the task.
     */
    generateCodeToLoadLiveInVariables(LDI, i);

    /*
     * Fix the data flow within the parallelized loop by redirecting operands of
     * cloned instructions to refer to the other cloned instructions. Currently,
     * they still refer to the original loop's instructions.
     */
    adjustDataFlowToUseClones(LDI, i);

    /*
     * Add the unconditional branch from the entry basic block to the header of the loop.
     */
    IRBuilder<> entryBuilder(task->entryBlock);
    entryBuilder.CreateBr(task->basicBlockClones[LDI->header]);

    /*
     * Add the return instruction at the end of the exit basic block.
     */
    IRBuilder<> exitBuilder(task->exitBlock);
    exitBuilder.CreateRetVoid();

    /*
     * Store final results to loop live-out variables.
     * Generate a store to propagate the information about which exit block has been taken from the parallelized loop to the code outside it.
     */
    generateCodeToStoreLiveOutVariables(LDI, i);
    generateCodeToStoreExitBlockIndex(LDI, i);

    /*
     * Inline recursively calls to queues.
     */
    inlineQueueCalls(LDI, i);

    if (this->verbose >= Verbosity::Maximal) {
      task->F->print(errs() << "Pipeline stage " << i << ":\n"); errs() << "\n";
    }
  }

  /*
   * Create the whole pipeline by connecting the stages.
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DSWP:  Link pipeline stages\n";
  }
  createPipelineFromStages(LDI, par);

  return true;
}
