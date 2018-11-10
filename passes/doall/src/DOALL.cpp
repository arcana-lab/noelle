#include "DOALL.hpp"

DOALL::DOALL (Module &module, Verbosity v)
  :
  ParallelizationTechnique{module, v}
  {

  /*
   * Fetch the dispatcher to use to jump to a parallelized DOALL loop.
   */
  this->workerDispatcher = this->module.getFunction("doallDispatcher");

  auto &cxt = module.getContext();
  auto int8 = IntegerType::get(cxt, 8);
  auto int64 = IntegerType::get(cxt, 64);
  auto funcArgTypes = ArrayRef<Type*>({
    PointerType::getUnqual(int8),
    int64,
    int64,
    int64
  });
  this->workerType = FunctionType::get(Type::getVoidTy(cxt), funcArgTypes, false);

  return ;
}

bool DOALL::canBeAppliedToLoop (
  LoopDependenceInfoForParallelizer *LDI,
  Parallelization &par,
  Heuristics *h,
  ScalarEvolution &SE
) const {
  errs() << "DOALL: Checking if is a doall loop\n";

  /*
   * The loop must have one single exit path.
   */
  if (LDI->numberOfExits() > 1) { 
    errs() << "DOALL:   More than 1 loop exit blocks\n";
    return false;
  }

  /*
   * The loop must have all live-out variables to be reducable.
   */
  if (!LDI->sccdagAttrs.allPostLoopEnvValuesAreReducable(LDI->environment)) {
    errs() << "DOALL:   Some post environment value is not reducable\n";
    return false;
  }

  /*
   * The loop must have at least one induction variable.
   * This is because the trip count must be controlled by an induction variable.
   */
  if (!LDI->sccdagAttrs.loopHasInductionVariable()) {
    errs() << "DOALL:   Loop does not have an IV\n";
    return false;
  }

  /*
   * The compiler must be able to remove loop-carried data dependences of all SCCs with loop-carried data dependences.
   */
  auto nonDOALLSCCs = LDI->sccdagAttrs.getSCCsWithLoopCarriedDataDependencies();
  for (auto scc : nonDOALLSCCs) {
    auto &sccInfo = LDI->sccdagAttrs.getSCCAttrs(scc);
    //TODO(SIMONE): I'm not sure the following condition is correct. For example, a loop with a commutative SCC cannot be parallelized by DOALL.
    if (scc->getType() != SCC::SCCType::COMMUTATIVE
      && !sccInfo->isClonable
      && !LDI->sccdagAttrs.isSCCContainedInSubloop(LDI->liSummary, scc)) {
      scc->printMinimal(errs() << "DOALL:   Non clonable, non commutative scc at top level of loop:\n", "DOALL:\t") << "\n";
      return false;
    }
  }

  /*
   * The loop is a DOALL one.
   */
  errs() << "DOALL:   The loop can be parallelized with DOALL\n" ;
  return true;
}
      
bool DOALL::apply (
  LoopDependenceInfoForParallelizer *LDI,
  Parallelization &par,
  Heuristics *h,
  ScalarEvolution &SE
) {
  errs() << "DOALL: Start the parallelization\n";

  /*
   * Prepare DOALL worker (chunk executing function)
   */
  DOALLTechniqueWorker *chunkerWorker = new DOALLTechniqueWorker();
  this->generateWorkers(LDI, { chunkerWorker });
  this->numWorkerInstances = NUM_CORES;

  /*
   * Allocate memory for all environment variables
   */
  auto preEnvRange = LDI->environment->getPreEnvIndices();
  auto postEnvRange = LDI->environment->getPostEnvIndices();
  std::set<int> nonReducableVars(preEnvRange.begin(), preEnvRange.end());
  std::set<int> reducableVars(postEnvRange.begin(), postEnvRange.end());
  initializeEnvironmentBuilder(LDI, nonReducableVars, reducableVars);

  /*
   * Clone loop into the single worker used by DOALL
   */
  this->cloneSequentialLoop(LDI, 0);

  /*
   * Load all loop live-in values at the entry point of the worker.
   * Store final results to loop live-out variables.
   */
  auto envUser = this->envBuilder->getUser(0);
  for (auto envIndex : LDI->environment->getPreEnvIndices()) {
    envUser->addPreEnvIndex(envIndex);
  }
  for (auto envIndex : LDI->environment->getPostEnvIndices()) {
    envUser->addPostEnvIndex(envIndex);
  }
  this->generateCodeToLoadLiveInVariables(LDI, 0);
  this->generateCodeToStoreLiveOutVariables(LDI, 0);

  /*
   * Simplify the original IV to iterate from smaller to larger bound by +1 increments
   * Create the outermost loop that iterates over chunks
   * Adjust the innermost loop to execute a single chunk
   * TODO(angelo): Re-formulate these changes to work AFTER data flows are adjusted
   */
  this->simplifyOriginalLoopIV(LDI);
  this->generateOuterLoopAndAdjustInnerLoop(LDI);

  /*
   * Fix the data flow within the parallelized loop by redirecting operands of
   * cloned instructions to refer to the other cloned instructions. Currently,
   * they still refer to the original loop's instructions.
   */
  this->adjustDataFlowToUseClones(LDI, 0);

  /*
   * Hoist PHINodes in the original loop: this propagates their value
   *  through the outer loop latch/header back into the inner loop header
   * This is done after data flow is adjusted to disambiguate adjustments
   *  from original -> clone and adjustments to their execution flow
   */
  this->propagatePHINodesThroughOuterLoop(LDI);

  /*
   * Add the final return to the single worker's exit block.
   */
  IRBuilder<> exitB(workers[0]->exitBlock);
  exitB.CreateRetVoid();

  addChunkFunctionExecutionAsideOriginalLoop(LDI, par);

  workers[0]->F->print(errs() << "DOALL:  Finalized chunker:\n"); errs() << "\n";
  // LDI->entryPointOfParallelizedLoop->print(errs() << "Finalized doall BB\n"); errs() << "\n";
  // LDI->function->print(errs() << "LDI function:\n"); errs() << "\n";

  errs() << "DOALL: Exit\n";
  return true;
}

void DOALL::propagateLiveOutEnvironment (LoopDependenceInfoForParallelizer *LDI) {
  std::unordered_map<int, int> reducableBinaryOps;
  std::unordered_map<int, Value *> initialValues;

  /*
   * Assertions.
   */
  assert(LDI != nullptr);

  for (auto envInd : LDI->environment->getPostEnvIndices()) {
    auto producer = LDI->environment->producerAt(envInd);
    auto producerSCC = LDI->loopSCCDAG->sccOfValue(producer);
    auto firstAccumI = *(LDI->sccdagAttrs.getSCCAttrs(producerSCC)->PHIAccumulators.begin());
    auto binOpCode = firstAccumI->getOpcode();
    reducableBinaryOps[envInd] = LDI->sccdagAttrs.accumOpInfo.accumOpForType(binOpCode, producer->getType());

    auto prodPHI = cast<PHINode>(producer);
    auto initValPHIIndex = prodPHI->getBasicBlockIndex(LDI->preHeader);
    initialValues[envInd] = prodPHI->getIncomingValue(initValPHIIndex);
  }

  auto builder = new IRBuilder<>(LDI->entryPointOfParallelizedLoop);
  this->envBuilder->reduceLiveOutVariables(*builder, reducableBinaryOps, initialValues);

  /*
   * Free the memory.
   */
  delete builder;

  ParallelizationTechnique::propagateLiveOutEnvironment(LDI);

  return ;
}

void DOALL::addChunkFunctionExecutionAsideOriginalLoop (
  LoopDependenceInfoForParallelizer *LDI,
  Parallelization &par
) {

  /*
   * Create the entry and exit points of the function that will include the parallelized loop.
   */
  auto &cxt = LDI->function->getContext();
  LDI->entryPointOfParallelizedLoop = BasicBlock::Create(cxt, "", LDI->function);
  LDI->exitPointOfParallelizedLoop = BasicBlock::Create(cxt, "", LDI->function);

  /*
   * Create the environment.
   */
  this->allocateEnvironmentArray(LDI);
  this->populateLiveInEnvironment(LDI);

  /*
   * Fetch the pointer to the environment.
   */
  auto envPtr = envBuilder->getEnvArrayInt8Ptr();

  // TODO(angelo): Outsource num cores / chunk size values to autotuner or heuristic
  auto numCores = ConstantInt::get(par.int64, NUM_CORES);
  auto chunkSize = ConstantInt::get(par.int64, CHUNK_SIZE);

  /*
   * Call the function that incudes the parallelized loop.
   */
  IRBuilder<> doallBuilder(LDI->entryPointOfParallelizedLoop);
  doallBuilder.CreateCall(this->workerDispatcher, ArrayRef<Value *>({
    (Value *)workers[0]->F,
    envPtr,
    numCores,
    chunkSize
  }));

  /*
   * Propagate the last value of live-out variables to the code outside the parallelized loop.
   */
  this->propagateLiveOutEnvironment(LDI);

  /*
   * Jump to the unique successor of the loop.
   */
  doallBuilder.CreateBr(LDI->exitPointOfParallelizedLoop);

  return ;
}
