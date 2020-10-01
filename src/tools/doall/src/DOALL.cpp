/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "DOALL.hpp"
#include "DOALLTask.hpp"

DOALL::DOALL (
  Module &module,
  Hot &p,
  Verbosity v
) :
  ParallelizationTechnique{module, p, v}
  {

  /*
   * Fetch the dispatcher to use to jump to a parallelized DOALL loop.
   */
  this->taskDispatcher = this->module.getFunction("NOELLE_DOALLDispatcher");
  if (this->taskDispatcher == nullptr){
    errs() << "NOELLE: ERROR = function NOELLE_DOALLDispatcher couldn't be found\n";
    abort();
  }

  /*
   * Define the signature of the task, which will be invoked by the DOALL dispatcher.
   */
  auto &cxt = module.getContext();
  auto int8 = IntegerType::get(cxt, 8);
  auto int64 = IntegerType::get(cxt, 64);
  auto funcArgTypes = ArrayRef<Type*>({
    PointerType::getUnqual(int8),
    int64,
    int64,
    int64
  });
  this->taskType = FunctionType::get(Type::getVoidTy(cxt), funcArgTypes, false);

  return ;
}

bool DOALL::canBeAppliedToLoop (
  LoopDependenceInfo *LDI,
  Noelle &par,
  Heuristics *h
) const {
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DOALL: Checking if the loop is DOALL\n";
  }

  /*
   * The loop must have one single exit path.
   */
  if (LDI->numberOfExits() > 1) { 
    if (this->verbose != Verbosity::Disabled) {
      errs() << "DOALL:   More than 1 loop exit blocks\n";
    }
    return false;
  }

  /*
   * The loop must have all live-out variables to be reducable.
   */
  if (!LDI->sccdagAttrs.areAllLiveOutValuesReducable(LDI->environment)) {
    if (this->verbose != Verbosity::Disabled) {
      errs() << "DOALL:   Some post environment value is not reducable\n";
    }
    return false;
  }

  /*
   * The loop must have at least one induction variable.
   * This is because the trip count must be controlled by an induction variable.
   */
  if (!LDI->getLoopGoverningIVAttribution()) {
    if (this->verbose != Verbosity::Disabled) {
      errs() << "DOALL:   Loop does not have an IV\n";
    }
    return false;
  }

  /*
   * NOTE: Due to a limitation in our ability to chunk induction variables,
   * all induction variables must have step sizes that are loop invariant
   */
  auto IVManager = LDI->getInductionVariableManager();
  for (auto IV : IVManager->getInductionVariables(*LDI->getLoopStructure())) {
    if (IV->isStepValueLoopInvariant()) {
      continue;
    }
    if (this->verbose != Verbosity::Disabled) {
      errs() << "DOALL:  Loop has an induction variable with step size that is not loop invariant\n";
    }
    return false;
  }

  /*
   * The compiler must be able to remove loop-carried data dependences of all SCCs with loop-carried data dependences.
   */
  auto nonDOALLSCCs = LDI->sccdagAttrs.getSCCsWithLoopCarriedDataDependencies();
  for (auto scc : nonDOALLSCCs) {

    /*
     * Fetch the SCC metadata.
     */
    auto sccInfo = LDI->sccdagAttrs.getSCCAttrs(scc);

    /*
     * If the SCC is reducable, then it does not block the loop to be a DOALL.
     */
    if (sccInfo->canExecuteReducibly()){
      continue ;
    }

    /*
     * If the SCC can be cloned, then it does not block the loop to be a DOALL.
     */
    if (sccInfo->canBeCloned()){
      continue ;
    }

    /*
     * If all loop carried data dependencies within the SCC do not overlap between
     * iterations, then DOALL can ignore them
     */
    auto areAllDataLCDsFromDisjointMemoryAccesses = true;
    auto domainSpaceAnalysis = LDI->getLoopIterationDomainSpaceAnalysis();
    LDI->sccdagAttrs.iterateOverLoopCarriedDataDependences(scc, [
      &areAllDataLCDsFromDisjointMemoryAccesses, domainSpaceAnalysis
    ](DGEdge<Value> *dep) -> bool {
      if (dep->isControlDependence()) return false;

      if (!dep->isMemoryDependence()) {
        areAllDataLCDsFromDisjointMemoryAccesses = false;
        return true;
      }

      auto fromInst = dyn_cast<Instruction>(dep->getOutgoingT());
      auto toInst = dyn_cast<Instruction>(dep->getIncomingT());
      areAllDataLCDsFromDisjointMemoryAccesses &= fromInst && toInst && domainSpaceAnalysis->
        areInstructionsAccessingDisjointMemoryLocationsBetweenIterations(fromInst, toInst);
      return !areAllDataLCDsFromDisjointMemoryAccesses;
    });
    if (areAllDataLCDsFromDisjointMemoryAccesses) {
      // if (this->verbose >= Verbosity::Maximal) {
      //   scc->printMinimal(errs() << "SCC has memory LCDs that are disjoint between iterations!\n"); errs() << "\n";
      // }
      continue;
    }

    if (this->verbose != Verbosity::Disabled) {
      errs() << "DOALL:   We found an SCC of type " << sccInfo->getType() << " of the loop that is non clonable and non commutative\n" ;
      if (this->verbose >= Verbosity::Maximal) {
        // scc->printMinimal(errs(), "DOALL:     ") ;
        // DGPrinter::writeGraph<SCC, Value>("not-doall-loop-scc-" + std::to_string(LDI->getID()) + ".dot", scc);
        errs() << "DOALL:     Loop-carried data dependences\n";
        LDI->sccdagAttrs.iterateOverLoopCarriedDataDependences(scc, [](DGEdge<Value> *dep) -> bool {
          auto fromInst = dep->getOutgoingT();
          auto toInst = dep->getIncomingT();
          errs() << "DOALL:       " << *fromInst << " ---> " << *toInst ;
          if (dep->isMemoryDependence()){
            errs() << " via memory\n";
          } else {
            errs() << " via variable\n";
          }
          return false;
            });
      }
    }
    return false;
  }

  /*
   * The loop is a DOALL one.
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DOALL:   The loop can be parallelized with DOALL\n" ;
  }
  return true;
}
      
bool DOALL::apply (
  LoopDependenceInfo *LDI,
  Noelle &par,
  Heuristics *h
) {

  /*
   * Fetch the headers.
   */
  auto loopSummary = LDI->getLoopStructure();
  auto loopHeader = loopSummary->getHeader();
  auto loopPreHeader = loopSummary->getPreHeader();

  /*
   * Fetch the loop function.
   */
  auto loopFunction = loopSummary->getFunction();

  /*
   * Print the parallelization request.
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DOALL: Start the parallelization\n";
    errs() << "DOALL:   Number of threads to extract = " << LDI->getMaximumNumberOfCores() << "\n";
    errs() << "DOALL:   Chunk size = " << LDI->DOALLChunkSize << "\n";
  }

  /*
   * Generate an empty task for the parallel DOALL execution.
   */
  auto chunkerTask = new DOALLTask(this->taskType, this->module);
  this->generateEmptyTasks(LDI, { chunkerTask });
  this->numTaskInstances = LDI->getMaximumNumberOfCores();

  /*
   * Allocate memory for all environment variables
   */
  auto preEnvRange = LDI->environment->getEnvIndicesOfLiveInVars();
  auto postEnvRange = LDI->environment->getEnvIndicesOfLiveOutVars();
  std::set<int> nonReducableVars(preEnvRange.begin(), preEnvRange.end());
  std::set<int> reducableVars(postEnvRange.begin(), postEnvRange.end());
  this->initializeEnvironmentBuilder(LDI, nonReducableVars, reducableVars);

  /*
   * Clone loop into the single task used by DOALL
   */
  this->cloneSequentialLoop(LDI, 0);
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DOALL:  Cloned loop\n";
  }

  /*
   * Load all loop live-in values at the entry point of the task.
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
   * HACK: For now, this must follow loading live-ins as this re-wiring overrides
   * the live-in mapping to use locally cloned memory instructions that are live-in to the loop
   */
  if (LDI->isOptimizationEnabled(LoopDependenceInfoOptimization::MEMORY_CLONING_ID)) {
    this->cloneMemoryLocationsLocallyAndRewireLoop(LDI, 0);
  }

  /*
   * Fix the data flow within the parallelized loop by redirecting operands of
   * cloned instructions to refer to the other cloned instructions. Currently,
   * they still refer to the original loop's instructions.
   */
  this->adjustDataFlowToUseClones(LDI, 0);
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DOALL:  Adjusted data flow\n";
  }

  this->setReducableVariablesToBeginAtIdentityValue(LDI, 0);
  this->rewireLoopToIterateChunks(LDI);
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DOALL:  Rewired induction variables and reducible variables\n";
  }

  /*
   * Add the final return to the single task's exit block.
   */
  IRBuilder<> exitB(tasks[0]->getExit());
  exitB.CreateRetVoid();

  /*
   * Store final results to loop live-out variables. Note this occurs after
   * all other code is generated. Propagated PHIs through the generated
   * outer loop might affect the values stored
   */
  this->generateCodeToStoreLiveOutVariables(LDI, 0);

  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DOALL:  Stored live outs\n";
  }

  this->addChunkFunctionExecutionAsideOriginalLoop(LDI, loopFunction, par);

  /*
   * Final printing.
   */
  if (this->verbose >= Verbosity::Maximal) {
    // loopFunction->print(errs() << "DOALL:  Final outside-loop code:\n" );
    // errs() << "\n";
    tasks[0]->getTaskBody()->print(errs() << "DOALL:  Final parallelized loop:\n"); 
    errs() << "\n";
    // SubCFGs execGraph(*chunkerTask->getTaskBody());
    // DGPrinter::writeGraph<SubCFGs, BasicBlock>("doalltask-loop" + std::to_string(LDI->getID()) + ".dot", &execGraph);
    // SubCFGs execGraph2(*loopFunction);
    // DGPrinter::writeGraph<SubCFGs, BasicBlock>("doall-loop-" + std::to_string(LDI->getID()) + "-function.dot", &execGraph);
  }
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DOALL: Exit\n";
  }

  return true;
}

void DOALL::addChunkFunctionExecutionAsideOriginalLoop (
  LoopDependenceInfo *LDI,
  Function *loopFunction,
  Noelle &par
) {

  /*
   * Create the environment.
   */
  this->allocateEnvironmentArray(LDI);
  this->populateLiveInEnvironment(LDI);

  /*
   * Fetch the pointer to the environment.
   */
  auto envPtr = envBuilder->getEnvArrayInt8Ptr();

  /*
   * Fetch the number of cores
   */
  auto numCores = ConstantInt::get(par.int64, LDI->getMaximumNumberOfCores());

  /*
   * Fetch the chunk size.
   */
  auto chunkSize = ConstantInt::get(par.int64, LDI->DOALLChunkSize);

  /*
   * Call the function that incudes the parallelized loop.
   */
  IRBuilder<> doallBuilder(this->entryPointOfParallelizedLoop);
  auto doallCallInst = doallBuilder.CreateCall(this->taskDispatcher, ArrayRef<Value *>({
    tasks[0]->getTaskBody(),
    envPtr,
    numCores,
    chunkSize
  }));
  auto numThreadsUsed = doallBuilder.CreateExtractValue(doallCallInst, (uint64_t)0);

  /*
   * Propagate the last value of live-out variables to the code outside the parallelized loop.
   */
  auto latestBBAfterDOALLCall = this->propagateLiveOutEnvironment(LDI, numThreadsUsed);

  /*
   * Jump to the unique successor of the loop.
   */
  IRBuilder<> afterDOALLBuilder{latestBBAfterDOALLCall};
  afterDOALLBuilder.CreateBr(this->exitPointOfParallelizedLoop);

  return ;
}

Value * DOALL::fetchClone (Value *original) const {
  auto task = (DOALLTask *)this->tasks[0];
  if (isa<ConstantData>(original)) return original;

  if (task->isAnOriginalLiveIn(original)){
    return task->getCloneOfOriginalLiveIn(original);
  }

  assert(isa<Instruction>(original));
  auto iClone = task->getCloneOfOriginalInstruction(cast<Instruction>(original));
  assert(iClone != nullptr);
  return iClone;
}
