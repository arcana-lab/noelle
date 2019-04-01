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
  Verbosity v
) :
  ParallelizationTechnique{module, v}
  {

  /*
   * Fetch the dispatcher to use to jump to a parallelized DOALL loop.
   */
  this->taskDispatcher = this->module.getFunction("doallDispatcher");

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
  Parallelization &par,
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
  if (!LDI->sccdagAttrs.isLoopGovernedByIV()) {
    if (this->verbose != Verbosity::Disabled) {
      errs() << "DOALL:   Loop does not have an IV\n";
    }
    return false;
  }

  /*
   * The loop's IV does not have bounds that have been successfuly analyzed
   */
  auto headerBr = LDI->header->getTerminator();
  auto headerSCC = LDI->loopSCCDAG->sccOfValue(headerBr);
  if (LDI->sccdagAttrs.sccIVBounds.find(headerSCC) == LDI->sccdagAttrs.sccIVBounds.end()) {
    if (this->verbose != Verbosity::Disabled) {
      errs() << "DOALL:   It wasn't possible to determine how to compute the loop trip count just before executing the loop\n" ;
    }
    return false;
  }

  /*
   * The compiler must be able to remove loop-carried data dependences of all SCCs with loop-carried data dependences.
   */
  auto nonDOALLSCCs = LDI->sccdagAttrs.getSCCsWithLoopCarriedDataDependencies();
  for (auto scc : nonDOALLSCCs) {

    /*
     * If the SCC is reducable, then it does not block the loop to be a DOALL.
     */
    if (scc->getType() == SCC::SCCType::REDUCIBLE){
      continue ;
    }

    /*
     * If the SCC can be cloned, then it does not block the loop to be a DOALL.
     */
    if (LDI->sccdagAttrs.canBeCloned(scc)){
      continue ;
    }
    
    /*
     * If the SCC is of the sub-loop, then it does not block the loop to be a DOALL.
     */
    if (LDI->sccdagAttrs.isSCCContainedInSubloop(LDI->liSummary, scc)) {
      continue ;
    }

    if (this->verbose != Verbosity::Disabled) {
      errs() << "DOALL:   We found an SCC of type " << scc->getType() << " of the loop that is non clonable and non commutative\n" ;
      if (this->verbose >= Verbosity::Maximal) {
        scc->printMinimal(errs(), "DOALL:\t") << "\n";
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
  Parallelization &par,
  Heuristics *h
) {

  /*
   * Print the parallelization request.
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DOALL: Start the parallelization\n";
    errs() << "DOALL:   Number of threads to extract = " << LDI->maximumNumberOfCoresForTheParallelization << "\n";
    errs() << "DOALL:   Chunk size = " << LDI->DOALLChunkSize << "\n";
  }

  /*
   * Generate empty tasks for DOALL execution.
   */
  auto chunkerTask = new DOALLTask();
  this->generateEmptyTasks(LDI, { chunkerTask });
  this->numTaskInstances = LDI->maximumNumberOfCoresForTheParallelization;

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
  this->setReducableVariablesToBeginAtIdentityValue(LDI, 0);

  /*
   * Add the final return to the single task's exit block.
   */
  IRBuilder<> exitB(tasks[0]->exitBlock);
  exitB.CreateRetVoid();

  /*
   * Hoist PHINodes in the original loop: this propagates their value
   *  through the outer loop latch/header back into the inner loop header
   * This is done after data flow is adjusted to disambiguate mapping
   *  from original -> clone instructions and adjusting flow of execution
   */
  this->propagatePHINodesThroughOuterLoop(LDI);

  /*
   * Store final results to loop live-out variables. Note this occurs after
   * all other code is generated. Propagated PHIs through the generated
   * outer loop might affect the values stored
   */
  this->generateCodeToStoreLiveOutVariables(LDI, 0);

  addChunkFunctionExecutionAsideOriginalLoop(LDI, par);

  if (this->verbose >= Verbosity::Maximal) {
    tasks[0]->F->print(errs() << "DOALL:  Finalized chunker:\n"); errs() << "\n";
    errs() << "DOALL: Exit\n";
  }

  return true;
}

void DOALL::propagateLiveOutEnvironment (LoopDependenceInfo *LDI) {
  std::unordered_map<int, int> reducableBinaryOps;
  std::unordered_map<int, Value *> initialValues;

  /*
   * Assertions.
   */
  assert(LDI != nullptr);

  for (auto envInd : LDI->environment->getEnvIndicesOfLiveOutVars()) {
    auto producer = LDI->environment->producerAt(envInd);
    auto producerSCC = LDI->loopSCCDAG->sccOfValue(producer);
    auto firstAccumI = *(LDI->sccdagAttrs.getSCCAttrs(producerSCC)->accumulators.begin());
    auto binOpCode = firstAccumI->getOpcode();
    reducableBinaryOps[envInd] = LDI->sccdagAttrs.accumOpInfo.accumOpForType(binOpCode, producer->getType());

    auto prodPHI = cast<PHINode>(producer);
    auto initValPHIIndex = prodPHI->getBasicBlockIndex(LDI->preHeader);
    initialValues[envInd] = prodPHI->getIncomingValue(initValPHIIndex);
  }

  auto builder = new IRBuilder<>(this->entryPointOfParallelizedLoop);
  this->envBuilder->reduceLiveOutVariables(*builder, reducableBinaryOps, initialValues);

  /*
   * Free the memory.
   */
  delete builder;

  ParallelizationTechnique::propagateLiveOutEnvironment(LDI);

  return ;
}

void DOALL::addChunkFunctionExecutionAsideOriginalLoop (
  LoopDependenceInfo *LDI,
  Parallelization &par
) {

  /*
   * Create the entry and exit points of the function that will include the parallelized loop.
   */
  auto &cxt = LDI->function->getContext();

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
  auto numCores = ConstantInt::get(par.int64, LDI->maximumNumberOfCoresForTheParallelization);

  /*
   * Fetch the chunk size.
   */
  auto chunkSize = ConstantInt::get(par.int64, LDI->DOALLChunkSize);

  /*
   * Call the function that incudes the parallelized loop.
   */
  IRBuilder<> doallBuilder(this->entryPointOfParallelizedLoop);
  doallBuilder.CreateCall(this->taskDispatcher, ArrayRef<Value *>({
    (Value *)tasks[0]->F,
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
  doallBuilder.CreateBr(this->exitPointOfParallelizedLoop);

  return ;
}
