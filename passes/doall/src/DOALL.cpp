#include "DOALL.hpp"

DOALL::DOALL (Module &module, Verbosity v)
  :
  ParallelizationTechnique{module, v}
  {

  this->doallDispatcher = this->module.getFunction("doallDispatcher");

  return ;
}

bool DOALL::canBeAppliedToLoop (LoopDependenceInfo *LDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) const {

  if (LDI->loopExitBlocks.size() > 1) return false;

  errs() << "DOALL CHECKS --------- IS DOALL (loop exit blocks == 1) \n";
  if (!LDI->sccdagAttrs.allPostLoopEnvValuesAreReducable(LDI->environment)) return false;
  errs() << "DOALL CHECKS --------- IS DOALL (post env reducable) \n";

  if (!LDI->sccdagAttrs.loopHasInductionVariable(SE)) return false;
  errs() << "DOALL CHECKS --------- IS DOALL (has IV) \n";

  auto nonDOALLSCCs = LDI->sccdagAttrs.getSCCsWithLoopCarriedDataDependencies();
  bool allSCCsDOALL = true;
  for (auto scc : nonDOALLSCCs) {
    scc->print(errs() << "Loop carried dep scc:\n") << "\n";
    auto &sccInfo = LDI->sccdagAttrs.getSCCAttrs(scc);
    allSCCsDOALL &= scc->getType() == SCC::SCCType::COMMUTATIVE
      || sccInfo->isClonable
      || LDI->sccdagAttrs.isSCCContainedInSubloop(LDI->liSummary, scc);
    // errs() << "DOALL CHECKS --------- IS DOALL (scc): " << isDOALL << "\n";
  }
  return allSCCsDOALL;
}
      
bool DOALL::apply (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  Heuristics *h,
  ScalarEvolution &SE
) {
  errs() << "DOALL: Start\n";

  auto chunker = this->createChunkingFuncAndArgs(LDI, par);

   /*
    * Create some utility values
    */
  chunker->zeroV = cast<Value>(ConstantInt::get(par.int64, 0));

  this->reproduceOriginLoop(LDI, par, chunker);
  this->reproducePreEnv(LDI, par, chunker);
  this->mapOriginLoopValueUses(LDI, par, chunker);
  // TODO(angelo): this shouldn't need scalar evolution, abstract to SCCAttrs
  this->collectOriginIVValues(LDI, par, chunker, SE);
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
  // TODO Check all of these after refactoring
  auto firstBB = &*LDI->function->begin();
  IRBuilder<> entryBuilder(firstBB->getTerminator());
  LDI->envArray = entryBuilder.CreateAlloca(LDI->envArrayType);

  auto &cxt = LDI->function->getContext();
  LDI->entryPointOfParallelizedLoop = BasicBlock::Create(cxt, "", LDI->function);
  LDI->exitPointOfParallelizedLoop = BasicBlock::Create(cxt, "", LDI->function);
  IRBuilder<> doallBuilder(LDI->entryPointOfParallelizedLoop);

  auto envPtr = createEnvArray(LDI, par, chunker, entryBuilder, doallBuilder);
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
}
