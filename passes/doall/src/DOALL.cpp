#include "DOALL.hpp"

DOALL::DOALL (Module &module, Verbosity v)
  :
  ParallelizationTechnique{module, v}
  {

  /*
   * Fetch the dispatcher to use to jump to a parallelized DOALL loop.
   */
  this->dispatcher = this->module.getFunction("doallDispatcher");

  return ;
}

bool DOALL::canBeAppliedToLoop (LoopDependenceInfoForParallelizer *LDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) const {
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
   * Initialize the environment.
   */
  this->initEnvBuilder(LDI);

  /*
   * Create a new function, which we are going to call it the DOALL function, where we will store the parallelized loop.
   */
  auto chunker = this->createFunctionThatWillIncludeTheParallelizedLoop(LDI, par);

  /*
   * Clone the sequential loop and store the clone to the DOALL function.
   */
  this->cloneSequentialLoop(LDI, chunker);

  /*
   * Load all loop live-in values at the entry point of the DOALL function, before the parallelized loop starts.
   */
  this->generateCodeToLoadLiveInVariables(LDI, chunker);

  /*
   * Fix the data flow within the parallelized loop.
   *
   * At this point, all operands of an instruction of the parallelize loop still point to the instructions within the sequential loop.
   * We have to redirect these operands to point to the new operands that are generated within the parallelized loop.
   */
  this->adjustDataFlowToUseClonedInstructions(LDI, chunker);

  this->reduceOriginIV(LDI, par, chunker, SE);

  /*
   * Create the outermost loop that iterate over chunks.
   */
  this->createOuterLoop(LDI, par, chunker);

  /*
   * Adjust the innermost loop, which iterates over elements within a single chunk.
   */
  this->alterInnerLoopToIterateChunks(LDI, par, chunker);

  /*
   * Storing the final results to loop live-out variables.
   */
  this->generateCodeToStoreLiveOutVariables(LDI, chunker);

  /*
   * Add the final return to the DOALL function.
   */
  IRBuilder<> exitB(chunker->exitBlock);
  exitB.CreateRetVoid();

  addChunkFunctionExecutionAsideOriginalLoop(LDI, par, chunker);

  //chunker->f->print(errs() << "DOALL:  Finalized chunker:\n"); errs() << "\n";
  // LDI->entryPointOfParallelizedLoop->print(errs() << "Finalized doall BB\n"); errs() << "\n";
  // LDI->function->print(errs() << "LDI function:\n"); errs() << "\n";

  errs() << "DOALL: Exit\n";
  return true;
}

void DOALL::createEnvironment (LoopDependenceInfoForParallelizer *LDI) {
  ParallelizationTechnique::createEnvironment(LDI);

  IRBuilder<> builder(LDI->entryPointOfParallelizedLoop);
  envBuilder->createEnvArray(builder);

  /*
   * Allocate memory for all environment variables
   */
  auto preEnvRange = LDI->environment->getPreEnvIndices();
  auto postEnvRange = LDI->environment->getPostEnvIndices();
  std::set<int> nonReducableVars(preEnvRange.begin(), preEnvRange.end());
  std::set<int> reducableVars(postEnvRange.begin(), postEnvRange.end());

  envBuilder->allocateEnvVariables(builder, nonReducableVars, reducableVars, NUM_CORES);
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
  this->envBuilder->reduceLiveOutVariables(*builder, reducableBinaryOps, initialValues, NUM_CORES);

  /*
   * Free the memory.
   */
  delete builder;

  ParallelizationTechnique::propagateLiveOutEnvironment(LDI);

  return ;
}

void DOALL::addChunkFunctionExecutionAsideOriginalLoop (
  LoopDependenceInfoForParallelizer *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker
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
  this->createEnvironment(LDI);
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
  doallBuilder.CreateCall(this->dispatcher, ArrayRef<Value *>({
    (Value *)chunker->f,
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
