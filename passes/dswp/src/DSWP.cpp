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
  Verbosity v
) :
  ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences{module, v},
  forceParallelization{forceParallelization},
  enableMergingSCC{enableSCCMerging},
  queues{}, queueArrayType{nullptr},
  sccToStage{}, stageArrayType{nullptr},
  zeroIndexForBaseArray{nullptr}
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

bool DSWP::canBeAppliedToLoop (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  Heuristics *h
) const {

  /*
   * Check if there is at least one sequential stage.
   * If there isn't, then this loop is a DOALL. Hence, DSWP is not applicable.
   */
  for (auto nodePair : LDI->loopSCCDAG->internalNodePairs()) {

    /*
     * Check if the current SCC can be removed (e.g., because it is due to induction variables).
     * If it is, then this SCC has already been assigned to every dependent partition.
     */
    auto currentSCC = nodePair.first;
    if (LDI->sccdagAttrs.canBeCloned(currentSCC)) continue ;

    /*
     * We found a sequential stage.
     */
    return true ;
  }

  /*
   * No sequential stage has been found.
   */
  return false;
}

bool DSWP::apply (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  Heuristics *h
) {

  /*
   * Start.
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DSWP: Start\n";
  }

  /*
   * Partition the SCCDAG.
   */
  partitionSCCDAG(LDI, h);
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DSWP:  There are " << this->partition->numberOfPartitions() << " partitions in the SCCDAG\n";
  }

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
  auto liveInVars = LDI->environment->getEnvIndicesOfLiveInVars();
  auto liveOutVars = LDI->environment->getEnvIndicesOfLiveOutVars();
  std::set<int> nonReducableVars(liveInVars.begin(), liveInVars.end());
  nonReducableVars.insert(liveOutVars.begin(), liveOutVars.end());
  std::set<int> reducableVars;

  /*
   * Should an exit block environment variable be necessary, register one 
   */
  if (LDI->numberOfExits() > 1){ 
    nonReducableVars.insert(LDI->environment->indexOfExitBlock());
  }

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
  this->zeroIndexForBaseArray = cast<Value>(ConstantInt::get(par.int64, 0));
  this->queueArrayType = ArrayType::get(PointerType::getUnqual(par.int8), this->queues.size());
  this->stageArrayType = ArrayType::get(PointerType::getUnqual(par.int8), this->tasks.size());

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
    generateLoadsOfQueuePointers(par, i);

    /*
     * Add push/pop operations from queues between the current pipeline stage and the connected ones
     */
    popValueQueues(par, i);
    pushValueQueues(par, i);

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
    inlineQueueCalls(i);

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

  /*
   * Exit
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DSWP: Exit\n";
  }
  return true;
}
