#include "HELIX.hpp"
#include "HELIXTask.hpp"

HELIX::HELIX (Module &module, Verbosity v)
  :
  ParallelizationTechnique{module, v}
  {

  /*
   * Fetch the LLVM context.
   */
  auto &cxt = module.getContext();

  /*
   * Fetch the dispatcher to use to jump to a parallelized HELIX loop.
   */
  this->taskDispatcher = this->module.getFunction("helixDispatcher");
  if (this->taskDispatcher == nullptr){
    errs()<< "HELIX: ERROR = the function helixDispatcher could not be found.\n" ;
    abort();
  }

  /*
   * Fetch the LLVM types of the helixDispatcher arguments.
   */
  auto int8 = IntegerType::get(cxt, 8);
  auto int64 = IntegerType::get(cxt, 64);

  /*
   * Create the LLVM signature of helixDispatcher.
   */
  auto funcArgTypes = ArrayRef<Type*>({
    PointerType::getUnqual(int8),
    int64,
    int64
  });
  this->taskType = FunctionType::get(Type::getVoidTy(cxt), funcArgTypes, false);

  return ;
}

bool HELIX::canBeAppliedToLoop (LoopDependenceInfo *LDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) const {
  return false;
}

bool HELIX::apply (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  Heuristics *h,
  ScalarEvolution &SE
) {

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
  std::set<int> allEnvironementVariables{liveInVars.begin(), liveInVars.end()};
  allEnvironementVariables.insert(liveOutVars.begin(), liveOutVars.end());

  /*
   * Build the single environment that is shared between all instances of the HELIX task.
   */
  this->initializeEnvironmentBuilder(LDI, allEnvironementVariables);

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
  this->generateCodeToStoreLiveOutVariables(LDI, 0);

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
   * Link the parallelize code to the original one.
   */
  this->addChunkFunctionExecutionAsideOriginalLoop(LDI, par, 0);

  /*
   * Print the HELIX task.
   */
  if (this->verbose != Verbosity::Disabled) {
    helixTask->F->print(errs() << "HELIX:  Task code:\n"); errs() << "\n";
    errs() << "HELIX: Exit\n";
  }

  return true;
}
