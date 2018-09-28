#include "DOALL.hpp"

DOALL::DOALL (Module &module, Verbosity v)
  :
  ParallelizationTechnique{module, v}
  {

  this->doallDispatcher = this->module.getFunction("doallDispatcher");

  return ;
}

bool DOALL::canBeAppliedToLoop (LoopDependenceInfo *LDI, Parallelization &par, Heuristics *h, ScalarEvolution &SE) const {

  errs() << "DOALL:   Checking if is a doall loop\n";

  bool isDOALL = true;
  if (LDI->loopExitBlocks.size() > 1) { 
    isDOALL = false;
    errs() << "DOALL:   More than 1 loop exit block\n";
  }

  if (!LDI->sccdagAttrs.allPostLoopEnvValuesAreReducable(LDI->environment)) {
    isDOALL = false;
    errs() << "DOALL:   Some post environment value is not reducable\n";
  }

  if (!LDI->sccdagAttrs.loopHasInductionVariable()) {
    isDOALL = false;
    errs() << "DOALL:   Loop does not have an IV\n";
  }

  auto nonDOALLSCCs = LDI->sccdagAttrs.getSCCsWithLoopCarriedDataDependencies();
  for (auto scc : nonDOALLSCCs) {
    auto &sccInfo = LDI->sccdagAttrs.getSCCAttrs(scc);
    if (scc->getType() != SCC::SCCType::COMMUTATIVE
      && !sccInfo->isClonable
      && !LDI->sccdagAttrs.isSCCContainedInSubloop(LDI->liSummary, scc)) {
      isDOALL = false;
      scc->printMinimal(errs() << "DOALL:   Non clonable, non commutative scc at top level of loop:\n", "DOALL:\t") << "\n";
    }
  }
  errs() << "DOALL:   Is it? " << isDOALL << "\n";
  return isDOALL;
}
      
bool DOALL::apply (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  Heuristics *h,
  ScalarEvolution &SE
) {
  errs() << "DOALL:   Start\n";
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
