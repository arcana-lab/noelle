/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "HELIX.hpp"
#include "HELIXTask.hpp"

HELIX::HELIX (Module &module, Verbosity v)
  : ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences{module, v},
    loopCarriedEnvBuilder{nullptr}, loopCarriedPHIs{}
  {

  /*
   * Fetch the LLVM context.
   */
  auto &cxt = module.getContext();

  /*
   * Fetch the dispatcher to use to jump to a parallelized HELIX loop.
   */
  this->taskDispatcher = this->module.getFunction("HELIX_dispatcher");
  this->waitSSCall = this->module.getFunction("HELIX_wait");
  this->signalSSCall =  this->module.getFunction("HELIX_signal");
  if (this->taskDispatcher == nullptr){
    errs()<< "HELIX: ERROR = the function HELIX_dispatcher could not be found.\n" ;
    abort();
  }
  if (!this->waitSSCall  || !this->signalSSCall) {
    errs() << "HELIX: ERROR = sync functions HELIX_wait, HELIX_signal were not both found.\n";
    abort();
  }

  /*
   * Fetch the LLVM types of the HELIX_dispatcher arguments.
   */
  auto int8 = IntegerType::get(cxt, 8);
  auto int64 = IntegerType::get(cxt, 64);

  /*
   * Create the LLVM signature of HELIX_dispatcher.
   */
  auto funcArgTypes = ArrayRef<Type*>({
    PointerType::getUnqual(int8),
    PointerType::getUnqual(int8),
    PointerType::getUnqual(int8),
    PointerType::getUnqual(int8),
    int64,
    int64,
    PointerType::getUnqual(int64)
  });
  this->taskType = FunctionType::get(Type::getVoidTy(cxt), funcArgTypes, false);

  return ;
}

void HELIX::reset () {
  ParallelizationTechnique::reset();
  loopCarriedPHIs.clear();
  if (loopCarriedEnvBuilder) {
    delete loopCarriedEnvBuilder;
  }
}

bool HELIX::canBeAppliedToLoop (LoopDependenceInfo *LDI, Parallelization &par, Heuristics *h) const {
  return true ;
}

bool HELIX::apply (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  Heuristics *h
) {

  /*
   * If a task has not been defined, create such a task from the
   * loop dependence info of the original function's loop
   * Otherwise, add synchronization to the already defined task
   * using the loop dependence info for that task
   */
  if (this->tasks.size() == 0) {
    this->createParallelizableTask(LDI, par, h);
  } else {
    this->synchronizeTask(LDI, par, h);
  }

  return true;
}

void HELIX::createParallelizableTask (
  LoopDependenceInfo *LDI,
  Parallelization &par, 
  Heuristics *h
){

  /*
   * NOTE: Keep around the original loops' LoopDependenceInfo for later phases
   * //TODO: we need to specify why this is necessary
   */
  this->originalLDI = LDI;

  /*
   * Print the parallelization request.
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "HELIX: Start the parallelization\n";
    errs() << "HELIX:   Number of threads to extract = " << LDI->maximumNumberOfCoresForTheParallelization << "\n";
  }

  /*
   * Generate empty tasks for the HELIX execution.
   */
  auto helixTask = new HELIXTask();
  this->generateEmptyTasks(LDI, { helixTask });
  this->numTaskInstances = LDI->maximumNumberOfCoresForTheParallelization;
  assert(helixTask == this->tasks[0]);

  /*
   * Fetch the indices of live-in and live-out variables of the loop being parallelized.
   */
  auto liveInVars = LDI->environment->getEnvIndicesOfLiveInVars();
  auto liveOutVars = LDI->environment->getEnvIndicesOfLiveOutVars();

  /*
   * Add all live-in and live-out variables as variables to be included in the environment.
   */
  std::set<int> nonReducableVars(liveInVars.begin(), liveInVars.end());
  nonReducableVars.insert(liveOutVars.begin(), liveOutVars.end());
  std::set<int> reducableVars{}; //TODO: SIMONE: why we don't have reducable vars? is this because ScalarEvolutionWrapperPass cannot be used because it needs to be recomputed?

  /*
   * Add the memory location of the environment used to store the exit block taken to leave the parallelized loop.
   * This location exists only if there is more than one loop exit.
   */
  if (LDI->numberOfExits() > 1){ 
    nonReducableVars.insert(LDI->environment->indexOfExitBlock());
  }

  /*
   * Build the single environment that is shared between all instances of the HELIX task.
   */
  this->initializeEnvironmentBuilder(LDI, nonReducableVars, reducableVars);

  /*
   * Clone the sequential loop and store the cloned instructions/basic blocks within the single task of HELIX.
   */
  this->cloneSequentialLoop(LDI, 0);

  /*
   * Load all loop live-in values at the entry point of the task.
   * Store final results to loop live-out variables.
   */
  auto envUser = this->envBuilder->getUser(0);
  for (auto envIndex : LDI->environment->getEnvIndicesOfLiveInVars()) {
    envUser->addLiveInIndex(envIndex);
  }
  for (auto envIndex : LDI->environment->getEnvIndicesOfLiveOutVars()) {
    envUser->addLiveOutIndex(envIndex);
  }
  this->generateCodeToLoadLiveInVariables(LDI, 0);

  /*
   * The operands of the cloned instructions still refer to the original ones.
   *
   * Fix the data flow within the parallelized loop by redirecting operands of cloned instructions to refer to the other cloned instructions. 
   */
  this->adjustDataFlowToUseClones(LDI, 0);

  /*
   * Add the unconditional branch from the entry basic block to the header of the loop.
   */
  IRBuilder<> entryBuilder(helixTask->entryBlock);
  entryBuilder.CreateBr(helixTask->basicBlockClones[LDI->header]);

  /*
   * Add the final return instruction to the single task's exit block.
   */
  IRBuilder<> exitB(helixTask->exitBlock);
  exitB.CreateRetVoid();

  /*
   * Store final results of loop live-out variables. 
   * Note this occurs after data flow is adjusted.  TODO: is this a must? if so, let's say it explicitely
   */
  this->generateCodeToStoreLiveOutVariables(LDI, 0);

  /*
   * Generate a store to propagate information about which exit block the parallelized loop took.
   */
  this->generateCodeToStoreExitBlockIndex(LDI, 0);

  /*
   * Spill loop carried dependencies into a separate environment array
   */
  this->spillLoopCarriedDataDependencies(LDI);

  return ;
}

void HELIX::synchronizeTask (
  LoopDependenceInfo *LDI,
  Parallelization &par, 
  Heuristics *h
){
  auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);

  /*
   * Identify the sequential segments.
   */
  auto sequentialSegments = this->identifySequentialSegments(LDI);

  /*
   * Schedule the code to minimize the instructions within each sequential segment.
   */
  this->squeezeSequentialSegments(LDI, &sequentialSegments);

  /*
   * Schedule the sequential segments to overlap parallel and sequential segments.
   */
  this->scheduleSequentialSegments(LDI, &sequentialSegments);

  /*
   * Add synchronization instructions.
   */
  this->addSynchronizations(LDI, &sequentialSegments);

  /*
   * Link the parallelize code to the original one.
   */
  this->addChunkFunctionExecutionAsideOriginalLoop(this->originalLDI, par, sequentialSegments.size());

  /*
   * Inline calls to HELIX functions.
   */
  this->inlineCalls();

  /*
   * Print the HELIX task.
   */
  if (this->verbose >= Verbosity::Maximal) {
    helixTask->F->print(errs() << "HELIX:  Task code:\n"); errs() << "\n";
    errs() << "HELIX: Exit\n";
  }

  return ;
}
