#include "DOALL.hpp"

DOALL::DOALL (Module &module, Verbosity v)
  :
  ParallelizationTechnique{module, v}
  {

  /*
   * Fetch the dispatcher to use to jump to a parallelized DOALL loop.
   */
  this->doallDispatcher = this->module.getFunction("doallDispatcher");

  return ;
}

bool DOALL::canBeAppliedToLoop (LoopDependenceInfo *LDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) const {
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
  LoopDependenceInfo *LDI,
  Parallelization &par,
  Heuristics *h,
  ScalarEvolution &SE
) {
  errs() << "DOALL: Start the parallelization\n";
  auto chunker = this->createChunkingFuncAndArgs(LDI, par);

  this->reproduceOriginLoop(LDI, par, chunker);
  this->reproducePreEnv(LDI, par, chunker);
  this->mapOriginLoopValueUses(LDI, par, chunker);
  this->reduceOriginIV(LDI, par, chunker, SE);
  this->createOuterLoop(LDI, par, chunker);
  this->alterInnerLoopToIterateChunks(LDI, par, chunker);
  this->storePostEnvironment(LDI, par, chunker);

  IRBuilder<> exitB(chunker->exitBlock);
  exitB.CreateRetVoid();

  addChunkFunctionExecutionAsideOriginalLoop(LDI, par, chunker);

  chunker->f->print(errs() << "DOALL:  Finalized chunker:\n"); errs() << "\n";
  // LDI->entryPointOfParallelizedLoop->print(errs() << "Finalized doall BB\n"); errs() << "\n";
  // LDI->function->print(errs() << "LDI function:\n"); errs() << "\n";

  errs() << "DOALL: Exit\n";
  return true;
}

void DOALL::addChunkFunctionExecutionAsideOriginalLoop (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker
) {
  auto &cxt = LDI->function->getContext();
  LDI->entryPointOfParallelizedLoop = BasicBlock::Create(cxt, "", LDI->function);
  LDI->exitPointOfParallelizedLoop = BasicBlock::Create(cxt, "", LDI->function);
  IRBuilder<> doallBuilder(LDI->entryPointOfParallelizedLoop);

  LDI->envBuilder->createEnvArray(doallBuilder);
  auto preEnvRange = LDI->environment->getPreEnvIndices();
  auto postEnvRange = LDI->environment->getPostEnvIndices();
  std::set<int> nonReducableVars(preEnvRange.begin(), preEnvRange.end());
  std::set<int> reducableVars(postEnvRange.begin(), postEnvRange.end());
  LDI->envBuilder->allocateEnvVariables(doallBuilder, nonReducableVars, reducableVars, NUM_CORES);
  auto envPtr = LDI->envBuilder->getEnvArrayInt8Ptr();

  /*
   * Insert pre-loop producers into the environment array
   */
  for (auto envIndex : LDI->environment->getPreEnvIndices()) {
    doallBuilder.CreateStore(LDI->environment->producerAt(envIndex), LDI->envBuilder->getEnvVar(envIndex));
  }

  // TODO(angelo): Outsource num cores / chunk size values to autotuner or heuristic
  auto numCores = ConstantInt::get(par.int64, NUM_CORES);
  auto chunkSize = ConstantInt::get(par.int64, CHUNK_SIZE);

  doallBuilder.CreateCall(this->doallDispatcher, ArrayRef<Value *>({
    (Value *)chunker->f,
    envPtr,
    numCores,
    chunkSize
  }));
  doallBuilder.CreateBr(LDI->exitPointOfParallelizedLoop);

  reducePostEnvironment(LDI, par, chunker);

  return ;
}
