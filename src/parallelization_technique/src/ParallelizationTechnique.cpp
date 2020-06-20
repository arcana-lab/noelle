/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "ParallelizationTechnique.hpp"

using namespace llvm;

ParallelizationTechnique::ParallelizationTechnique (
  Module &module, 
  Hot &p,
  Verbosity v
  )
  : module{module}, verbose{v}, tasks{}, envBuilder{0}, profile{p}
  {

  return ;
}

ParallelizationTechnique::~ParallelizationTechnique () {
  reset();

  return ;
}

void ParallelizationTechnique::reset () {
  for (auto task : tasks) delete task;
  tasks.clear();
  numTaskInstances = 0;

  if (envBuilder) {
    delete envBuilder;
    envBuilder = nullptr;
  }

  return ;
}

void ParallelizationTechnique::initializeEnvironmentBuilder (
  LoopDependenceInfo *LDI,
  std::set<int> simpleVars,
  std::set<int> reducableVars
) {
  if (tasks.size() == 0) {
    errs() << "ERROR: Parallelization technique tasks haven't been created yet!\n"
      << "\tTheir environment builders can't be initialized until they are.\n";
    abort();
  }

  /*
   * Collect the Type of each environment variable
   */
  std::vector<Type *> varTypes;
  for (auto i = 0; i < LDI->environment->envSize(); ++i) {
    varTypes.push_back(LDI->environment->typeOfEnv(i));
  }

  this->envBuilder = new EnvBuilder(module.getContext());
  this->envBuilder->createEnvVariables(varTypes, simpleVars, reducableVars, this->numTaskInstances);

  this->envBuilder->createEnvUsers(tasks.size());
  for (auto i = 0; i < tasks.size(); ++i) {
    auto task = tasks[i];
    auto envUser = envBuilder->getUser(i);
    auto entryBlock = task->getEntry();
    IRBuilder<> entryBuilder(entryBlock);
    envUser->setEnvArray(entryBuilder.CreateBitCast(
      task->getEnvironment(),
      PointerType::getUnqual(envBuilder->getEnvArrayTy())
    ));
  }

  return ;
}

void ParallelizationTechnique::initializeEnvironmentBuilder (
  LoopDependenceInfo *LDI,
  std::set<int> nonReducableVars
  ){
  std::set<int> emptySet{};

  this->initializeEnvironmentBuilder(LDI, nonReducableVars, emptySet);

  return ;
}

void ParallelizationTechnique::allocateEnvironmentArray (LoopDependenceInfo *LDI) {

  /*
   * Fetch the loop function.
   */
  auto loopSummary = LDI->getLoopSummary();
  auto loopFunction = loopSummary->getFunction();

  /*
   * Fetch the first instruction of the first basic block of the function that includes the loop we want to parallelized.
   */
  auto firstBB = loopFunction->begin();
  auto firstI = firstBB->begin();

  /*
   * Generate the environment.
   */
  IRBuilder<> builder(&*firstI);
  envBuilder->generateEnvArray(builder);
  envBuilder->generateEnvVariables(builder);

  return ;
}

void ParallelizationTechnique::populateLiveInEnvironment (LoopDependenceInfo *LDI) {
  IRBuilder<> builder(this->entryPointOfParallelizedLoop);
  for (auto envIndex : LDI->environment->getEnvIndicesOfLiveInVars()) {
    builder.CreateStore(LDI->environment->producerAt(envIndex), envBuilder->getEnvVar(envIndex));
  }
}

BasicBlock * ParallelizationTechnique::propagateLiveOutEnvironment (LoopDependenceInfo *LDI, Value *numberOfThreadsExecuted) {
  auto builder = new IRBuilder<>(this->entryPointOfParallelizedLoop);

  /*
   * Fetch the loop headers.
   */
  auto loopSummary = LDI->getLoopSummary();
  auto loopPreHeader = loopSummary->getPreHeader();

  /*
   * Collect reduction operation information needed to accumulate reducable variables after parallelization execution
   */
  std::unordered_map<int, int> reducableBinaryOps;
  std::unordered_map<int, Value *> initialValues;
  for (auto envInd : LDI->environment->getEnvIndicesOfLiveOutVars()) {
    auto isReduced = envBuilder->isReduced(envInd);
    if (!isReduced) continue;

    auto producer = LDI->environment->producerAt(envInd);
    auto producerSCC = LDI->sccdagAttrs.getSCCDAG()->sccOfValue(producer);
    auto producerSCCAttributes = LDI->sccdagAttrs.getSCCAttrs(producerSCC);

    /*
     * HACK: Need to get accumulator that feeds directly into producer PHI, not any intermediate one
     */
    auto firstAccumI = *(producerSCCAttributes->getAccumulators().begin());
    auto binOpCode = firstAccumI->getOpcode();
    reducableBinaryOps[envInd] = LDI->sccdagAttrs.accumOpInfo.accumOpForType(binOpCode, producer->getType());

    PHINode *headerProducerPHI = LDI->sccdagAttrs.getSCCAttrs(producerSCC)->getSingleHeaderPHI();
    assert(headerProducerPHI != nullptr &&
      "The reducible variable should be described by a single PHI in the header");
    auto initValPHIIndex = headerProducerPHI->getBasicBlockIndex(loopPreHeader);
    initialValues[envInd] = headerProducerPHI->getIncomingValue(initValPHIIndex);
  }

  auto afterReductionB = this->envBuilder->reduceLiveOutVariables(
    this->entryPointOfParallelizedLoop,
    *builder,
    reducableBinaryOps,
    initialValues,
    numberOfThreadsExecuted);

  /*
   * Free the memory.
   */
  delete builder;

  /*
   * If reduction occurred, then all environment loads to propagate live outs need to be
   * inserted after the reduction loop
   */
  IRBuilder<> *afterReductionBuilder;
  if (afterReductionB->getTerminator()) {
    afterReductionBuilder->SetInsertPoint(afterReductionB->getTerminator());
  } else {
    afterReductionBuilder = new IRBuilder<>(afterReductionB);
  }

  for (int envInd : LDI->environment->getEnvIndicesOfLiveOutVars()) {
    auto prod = LDI->environment->producerAt(envInd);

    /*
     * NOTE(angelo): If the environment variable isn't reduced, it is held in allocated
     * memory that needs to be loaded from in order to retrieve the value
     */
    auto isReduced = envBuilder->isReduced(envInd);
    Value *envVar;
    if (isReduced) {
      envVar = envBuilder->getAccumulatedReducableEnvVar(envInd);
    } else {
      envVar = afterReductionBuilder->CreateLoad(envBuilder->getEnvVar(envInd));
    }

    for (auto consumer : LDI->environment->consumersOf(prod)) {
      if (auto depPHI = dyn_cast<PHINode>(consumer)) {
        depPHI->addIncoming(envVar, this->exitPointOfParallelizedLoop);
        continue;
      }
      prod->print(errs() << "Producer of environment variable:\t"); errs() << "\n";
      errs() << "Loop not in LCSSA!\n";
      abort();
    }
  }

  delete afterReductionBuilder;
  return afterReductionB;
}

void ParallelizationTechnique::generateEmptyTasks (
  LoopDependenceInfo *LDI,
  std::vector<Task *> taskStructs
) {
  if (this->tasks.size() > 0) {
    errs() << "The technique has been re-initialized without resetting!"
       << " There are leftover tasks.\n";
    abort();
  }

  /*
   * Fetch the loop headers.
   */
  auto loopSummary = LDI->getLoopSummary();
  auto loopPreHeader = loopSummary->getPreHeader();

  /*
   * Fetch the loop function.
   */
  auto loopFunction = loopSummary->getFunction();

  /*
   * Setup original loop and task with functions and basic blocks for wiring
   */
  auto &cxt = loopFunction->getContext();
  this->entryPointOfParallelizedLoop = BasicBlock::Create(cxt, "", loopFunction);
  this->exitPointOfParallelizedLoop = BasicBlock::Create(cxt, "", loopFunction);

  this->numTaskInstances = taskStructs.size();
  for (auto i = 0; i < numTaskInstances; ++i) {
    auto task = taskStructs[i];
    tasks.push_back(task);

    /*
     * Set the formal arguments of the task.
     */
    auto &cxt = module.getContext();
    task->extractFuncArgs();

    /*
     * Map original preheader to entry block
     */
    task->addBasicBlock(loopPreHeader, task->getEntry());

    /*
     * Create one basic block per loop exit, mapping between originals and clones,
     * and branching from them to the function exit block
     */
    for (auto exitBB : LDI->getLoopSummary()->getLoopExitBasicBlocks()) {
      auto newExitBB = task->addBasicBlockStub(exitBB);
      task->tagBasicBlockAsLastBlock(newExitBB);
      IRBuilder<> builder(newExitBB);
      builder.CreateBr(task->getExit());
    }
  }
}

void ParallelizationTechnique::cloneSequentialLoop (
  LoopDependenceInfo *LDI,
  int taskIndex
){
  auto &cxt = module.getContext();
  auto task = tasks[taskIndex];

  /*
   * Clone all basic blocks of the original loop
   */
  auto topLoop = LDI->getLoopSummary();
  for (auto originBB : topLoop->orderedBBs) {

    /*
     * Clone the basic block.
     */
    task->cloneAndAddBasicBlock(originBB);
  }
}

void ParallelizationTechnique::cloneSequentialLoopSubset (
  LoopDependenceInfo *LDI,
  int taskIndex,
  std::set<Instruction *> subset
){
  auto &cxt = module.getContext();
  auto task = tasks[taskIndex];

  /*
   * Clone a portion of the original loop (determined by a set of SCCs
   * Determine the set of basic blocks these instructions belong to
   */
  std::set<BasicBlock *> bbSubset;
  for (auto I : subset) {
    task->cloneAndAddInstruction(I);
    bbSubset.insert(I->getParent());
  }

  /*
   * Add cloned instructions to their respective cloned basic blocks
   */
  for (auto bb : bbSubset) {
    auto cloneBB = task->addBasicBlockStub(bb);
    IRBuilder<> builder(cloneBB);
    for (auto &I : *bb) {
      if (!task->isAnOriginalInstruction(&I)) {
        continue;
      }
      auto cloneI = task->getCloneOfOriginalInstruction(&I);
      builder.Insert(cloneI);
    }
  }
}

void ParallelizationTechnique::generateCodeToLoadLiveInVariables (
  LoopDependenceInfo *LDI, 
  int taskIndex
){
  auto task = this->tasks[taskIndex];
  IRBuilder<> builder(task->getEntry());
  auto envUser = this->envBuilder->getUser(taskIndex);
  for (auto envIndex : envUser->getEnvIndicesOfLiveInVars()) {
    auto producer = LDI->environment->producerAt(envIndex);

    /*
     * Create GEP access of the environment variable at the given index
     */
    envUser->createEnvPtr(builder, envIndex, producer->getType());

    /*
     * Load the environment pointer
     * Register the load as a "clone" of the original producer
     */
    auto envLoad = builder.CreateLoad(envUser->getEnvPtr(envIndex));
    task->addLiveIn(producer, envLoad);
  }
}

void ParallelizationTechnique::generateCodeToStoreLiveOutVariables (
  LoopDependenceInfo *LDI, 
  int taskIndex
){

  /*
   * Fetch the requested task.
   */
  auto task = this->tasks[taskIndex];

  /*
   * Create a builder that points to the entry point of the function executed by the task.
   */
  auto entryBlock = task->getEntry();
  auto entryTerminator = entryBlock->getTerminator();
  IRBuilder<> entryBuilder(entryTerminator);

  /*
   * Iterate over live-out variables and inject stores at the end of the execution of the function of the task to propagate the new live-out values back to the caller of the parallelized loop.
   */
  auto envUser = this->envBuilder->getUser(taskIndex);
  for (auto envIndex : envUser->getEnvIndicesOfLiveOutVars()) {

    /*
     * Fetch the producer of the current live-out variable.
     */
    auto producer = (Instruction*)LDI->environment->producerAt(envIndex);

    /*
     * Create GEP access of the single, or reducable, environment variable
     */
    auto envType = producer->getType();
    auto isReduced = this->envBuilder->isReduced(envIndex);
    if (isReduced) {
      envUser->createReducableEnvPtr(entryBuilder, envIndex, envType, numTaskInstances, task->getTaskInstanceID());
    } else {
      envUser->createEnvPtr(entryBuilder, envIndex, envType);
    }
    auto envPtr = envUser->getEnvPtr(envIndex);

    /*
     * If the variable is reducable, store the identity as the initial value
     */
    if (isReduced) {

      /*
       * Fetch the operator of the accumulator instruction for this reducable variable
       * Store the identity value of the operator
       */
      auto identityV = getIdentityValueForEnvironmentValue(LDI, taskIndex, envIndex);
      entryBuilder.CreateStore(identityV, envPtr);
    }

    /*
     * Inject store instructions to propagate live-out values back to the caller of the parallelized loop.
     * 
     * NOTE: To support storing live outs at exit blocks and not directly where the producer
     * is executed, produce a PHI node at each store point with the following incoming values:
     * the last executed intermediate of the producer that is post-dominated by that incoming block.
     * There should only be one such value assuming that store point is correctly chosen
     * 
     * NOTE: This provides flexibility to parallelization schemes with modified prologues or latches
     * that have reducible live outs, and this flexibility is ONLY permitted for reducible live outs
     * as non-reducible live outs can never store intermediate values of the producer.
     */
    auto prodClone = task->getCloneOfOriginalInstruction(producer);
    auto insertBBs = this->determineLatestPointsToInsertLiveOutStore(LDI, taskIndex, producer);
    for (auto BB : insertBBs) {

      auto producerValueToStore = isReduced
        ? generatePHIOfIntermediateProducerValuesForReducibleLiveOutVariable(LDI, taskIndex, envIndex, BB)
        : prodClone;

      IRBuilder<> liveOutBuilder(BB);
      auto store = (StoreInst*)liveOutBuilder.CreateStore(producerValueToStore, envPtr);
      store->removeFromParent();
      store->insertBefore(BB->getTerminator());
    }
  }

  return ;
}

std::set<BasicBlock *> ParallelizationTechnique::determineLatestPointsToInsertLiveOutStore (
  LoopDependenceInfo *LDI,
  int taskIndex,
  Instruction *liveOut
){
  auto task = this->tasks[taskIndex];

  /*
   * Fetch the header.
   */
  auto loopSummary = LDI->getLoopSummary();
  auto loopHeader = loopSummary->getHeader();

  /*
   * TODO: Determine the exit block for which the live out is defined
   */
  std::set<BasicBlock *> insertPoints;
  // for (auto BB : loopSummary->getLoopExitBasicBlocks()) {
  //   insertPoints.insert(task->getCloneOfOriginalBasicBlock(BB));
  // }
  insertPoints.insert(task->getExit());
  return insertPoints;
}

/*
 * PROBLEM: We don't have domination summary of cloned task
 */
PHINode * ParallelizationTechnique::generatePHIOfIntermediateProducerValuesForReducibleLiveOutVariable (
  LoopDependenceInfo *LDI, 
  int taskIndex,
  int envIndex,
  BasicBlock *insertBasicBlock
) {

  /*
   * HACK: Compute task dominator summary
   */
  auto task = this->tasks[taskIndex];
  DominatorTree taskDT(*task->getTaskBody());
  PostDominatorTree taskPDT(*task->getTaskBody());
  DominatorSummary taskDS(taskDT, taskPDT);
  auto &DT = taskDS.DT;
  auto &PDT = taskDS.PDT;

  /*
   * Fetch all clones of intermediate values of the producer
   */
  auto producer = (Instruction*)LDI->environment->producerAt(envIndex);
  auto producerSCC = LDI->sccdagAttrs.getSCCDAG()->sccOfValue(producer);

  std::set<Instruction *> intermediateValues{};
  for (auto originalPHI : LDI->sccdagAttrs.getSCCAttrs(producerSCC)->getPHIs()) {
    intermediateValues.insert(task->getCloneOfOriginalInstruction(originalPHI));
  }
  for (auto originalI : LDI->sccdagAttrs.getSCCAttrs(producerSCC)->getAccumulators()) {
    intermediateValues.insert(task->getCloneOfOriginalInstruction(originalI));
  }

  /*
   * Produce PHI at the insert point
   */
  IRBuilder<> builder(insertBasicBlock->getFirstNonPHIOrDbgOrLifetime());
  auto producerType = producer->getType();
  auto phiNode = builder.CreatePHI(producerType, pred_size(insertBasicBlock));

  /*
   * Fetch all incoming blocks to the PHI node basic block
   * Determine all intermediate values post dominated by this block
   * Determine the intermediate value of this set that dominates all the others
   * NOTE: If this is a well-formed insert point for the live out, exactly one such intermediate value must exist
   */
  std::set<BasicBlock *> preds{pred_begin(insertBasicBlock), pred_end(insertBasicBlock)};
  for (auto pred : preds) {
    for (auto pred2 : preds) {
      if (DT.dominates(pred, pred2)) {
        pred->print(errs() << "This dominates:\n");
        pred2->print(errs() << "This is dominated\n");
      }
    }
  }

  for (auto predIter = pred_begin(insertBasicBlock); predIter != pred_end(insertBasicBlock); ++predIter) {
    auto predecessor = *predIter;
    predecessor->print(errs() << "Wiring from\n");

    std::set<Instruction *> dominatingValues{};
    for (auto intermediateValue : intermediateValues) {
      if (DT.dominates(intermediateValue->getParent(), predecessor)) {
        dominatingValues.insert(intermediateValue);
        intermediateValue->print(errs() << "Dominating value: "); errs() << "\n";
      }
    }

    assert(dominatingValues.size() > 0
      && "Cannot store reducible live out where no producer value dominates the point");

    Instruction *lastDominatingIntermediateValue = *dominatingValues.begin();
    for (auto value : dominatingValues) {
      if (!DT.dominates(lastDominatingIntermediateValue, value)) {
        if (!DT.dominates(value, lastDominatingIntermediateValue)) {
          lastDominatingIntermediateValue->print(errs() << "V1: "); errs() << "\n";
          value->print(errs() << "V1: "); errs() << "\n";
        }
        assert(DT.dominates(value, lastDominatingIntermediateValue)
          && "Cannot store reducible live out where no producer value post-dominates the others");
        continue;
      }
      lastDominatingIntermediateValue = value;
    }

    lastDominatingIntermediateValue->print(errs() << "Last intermediate: "); errs() << "\n";

    auto predecessorTerminator = predecessor->getTerminator();
    IRBuilder<> builderAtValue(predecessorTerminator);
    auto correctlyTypedValue = lastDominatingIntermediateValue->getType() == producerType
      ? lastDominatingIntermediateValue
      : builderAtValue.CreateBitCast(lastDominatingIntermediateValue, producerType);
    phiNode->addIncoming(correctlyTypedValue, predecessor);
  } 

  return phiNode;
}

void ParallelizationTechnique::adjustDataFlowToUseClones (
  LoopDependenceInfo *LDI,
  int taskIndex
){
  auto &task = tasks[taskIndex];

  for (auto origI : task->getOriginalInstructions()) {
    auto cloneI = task->getCloneOfOriginalInstruction(origI);
    adjustDataFlowToUseClones(cloneI, taskIndex);
  }
}

void ParallelizationTechnique::adjustDataFlowToUseClones (
  Instruction *cloneI,
  int taskIndex
){
  auto &task = tasks[taskIndex];

  /*
   * Adjust basic block references of terminators and PHI nodes
   */
  if (cloneI->isTerminator()) {
    for (int i = 0; i < cloneI->getNumSuccessors(); ++i) {
      auto succBB = cloneI->getSuccessor(i);
      if (succBB->getParent() == task->getTaskBody()) continue;
      assert(task->isAnOriginalBasicBlock(succBB));
      cloneI->setSuccessor(i, task->getCloneOfOriginalBasicBlock(succBB));
    }
  }

  if (auto phi = dyn_cast<PHINode>(cloneI)) {
    for (int i = 0; i < phi->getNumIncomingValues(); ++i) {
      auto incomingBB = phi->getIncomingBlock(i);
      if (incomingBB->getParent() == task->getTaskBody()) continue;
      auto cloneBB = task->getCloneOfOriginalBasicBlock(incomingBB);
      phi->setIncomingBlock(i, cloneBB);
    }
  }

  /*
    * Adjust values (other instructions and live-in values) used by clones
    */
  for (auto &op : cloneI->operands()) {
    auto opV = op.get();

    /*
     * If the value is a loop live-in one,
     * set it to the value loaded outside the parallelized loop.
     */
    if (task->isAnOriginalLiveIn(opV)){
      auto internalValue = task->getCloneOfOriginalLiveIn(opV);
      op.set(internalValue);
      continue ;
    }

    /*
     * If the value is generated by another instruction inside the loop,
     * set it to the equivalent cloned instruction.
     */
    if (auto opI = dyn_cast<Instruction>(opV)) {
      if (task->isAnOriginalInstruction(opI)){
        auto cloneOpI = task->getCloneOfOriginalInstruction(opI);
        op.set(cloneOpI);
      } else {
        if (opI->getFunction() != task->getTaskBody()) {
          cloneI->print(errs() << "ERROR:   Instruction has op from another function: "); errs() << "\n";
          opI->print(errs() << "ERROR:   Op: "); errs() << "\n";
        }
      }
    }
  }
}

void ParallelizationTechnique::setReducableVariablesToBeginAtIdentityValue (
  LoopDependenceInfo *LDI,
  int taskIndex
){

  /*
   * Fetch task information.
   */
  auto task = this->tasks[taskIndex];
  auto loopSummary = LDI->getLoopSummary();
  auto loopHeader = loopSummary->getHeader();
  auto headerClone = task->getCloneOfOriginalBasicBlock(loopHeader);
  auto loopPreHeader = loopSummary->getPreHeader();
  auto preheaderClone = task->getCloneOfOriginalBasicBlock(loopPreHeader);

  /*
   * Iterate over live-out variables.
   */
  for (auto envInd : LDI->environment->getEnvIndicesOfLiveOutVars()) {

    /*
     * Check if the current live-out variable can be reduced.
     */
    auto isThisLiveOutVarReducable = this->envBuilder->isReduced(envInd);
    if (!isThisLiveOutVarReducable) {
      continue;
    }

    /*
     * Fetch the instruction that produces the live-out variable.
     * The reducible live out must be contained within an SCC that has a
     * PHI node in the header. The incoming value from the preheader is the
     * location of the initial value that needs to be changed
     */
    auto producer = LDI->environment->producerAt(envInd);
    auto producerSCC = LDI->sccdagAttrs.getSCCDAG()->sccOfValue(producer);
    PHINode *headerProducerPHI = LDI->sccdagAttrs.getSCCAttrs(producerSCC)->getSingleHeaderPHI();
    assert(headerProducerPHI != nullptr &&
      "The reducible variable should be described by a single PHI in the header");

    /*
     * Fetch the related instruction of the producer that has been created (cloned) and stored in the parallelized version of the loop.
     */
    auto producerClone = cast<PHINode>(task->getCloneOfOriginalInstruction(headerProducerPHI));

    /*
     * Fetch the cloned pre-header index
     */
    auto incomingIndex = producerClone->getBasicBlockIndex(preheaderClone);
    assert(incomingIndex != -1 && "Loop entry present on producer PHI node");

    /*
     * Fetch the identity constant for the operation reduced.
     * For example, if the variable reduced is an accumulator where "+" is used to accumulate values, then "0" is the identity.
     */
    auto identityV = this->getIdentityValueForEnvironmentValue(LDI, taskIndex, envInd);

    /*
     * Set the initial value for the private variable.
     */
    producerClone->setIncomingValue(incomingIndex, identityV);
  }

  return ;
}

Value * ParallelizationTechnique::getIdentityValueForEnvironmentValue (
  LoopDependenceInfo *LDI,
  int taskIndex,
  int environmentIndex
){

  /*
   * Fetch the producer of new values of the current environment variable.
   */
  auto producer = LDI->environment->producerAt(environmentIndex);

  /*
   * Fetch the SCC that this producer belongs to.
   */
  auto producerSCC = LDI->sccdagAttrs.getSCCDAG()->sccOfValue(producer);
  assert(producerSCC != nullptr && "The environment value doesn't belong to a loop SCC");

  /*
   * Fetch the attributes about the producer SCC.
   */
  auto sccAttrs = LDI->sccdagAttrs.getSCCAttrs(producerSCC);
  assert(sccAttrs->numberOfAccumulators() > 0 && "The environment value isn't accumulated!");

  /*
   * Fetch the accumulator.
   */
  auto firstAccumI = *(sccAttrs->getAccumulators().begin());

  /*
   * Fetch the identity.
   */
  auto identityValue = LDI->sccdagAttrs.accumOpInfo.generateIdentityFor(
    firstAccumI,
    producer->getType()
  );

  return identityValue;
}

void ParallelizationTechnique::generateCodeToStoreExitBlockIndex (
  LoopDependenceInfo *LDI,
  int taskIndex
){

  /*
   * Check whether there are multiple exit blocks or not.
   * If there are more exit blocks, then we need to specify which one has been taken.
   */
  auto task = this->tasks[taskIndex];
  if (task->getNumberOfLastBlocks() == 1) {
    return ;
  }

  /*
   * There are multiple exit blocks.
   *
   * Fetch the pointer of the location where the exit block ID taken will be stored.
   */
  auto exitBlockEnvIndex = LDI->environment->indexOfExitBlock();
  assert(exitBlockEnvIndex != -1);
  auto envUser = this->envBuilder->getUser(taskIndex);
  auto entryTerminator = task->getEntry()->getTerminator();
  IRBuilder<> entryBuilder(entryTerminator);

  auto envType = LDI->environment->typeOfEnv(exitBlockEnvIndex);
  envUser->createEnvPtr(entryBuilder, exitBlockEnvIndex, envType);

  /*
   * Add a store instruction to specify to the code outside the parallelized loop which exit block is taken.
   */
  auto int32 = IntegerType::get(module.getContext(), 32);
  for (int i = 0; i < task->getNumberOfLastBlocks(); ++i) {
    auto bb = task->getLastBlock(i);
    auto term = bb->getTerminator();
    IRBuilder<> builder(bb);
    auto envPtr = envUser->getEnvPtr(exitBlockEnvIndex);
    builder.CreateStore(ConstantInt::get(int32, i), envPtr);
    term->removeFromParent();
    builder.Insert(term);
  }

  return ;
}

void ParallelizationTechnique::doNestedInlineOfCalls (
  Function *function,
  std::set<CallInst *> &calls
){
  std::queue<CallInst *> callsToInline;
  for (auto call : calls) callsToInline.push(call);

  while (!callsToInline.empty()) {

    /*
     * Empty the queue, inlining each site
     */
    std::set<Function *> funcToInline;
    while (!callsToInline.empty()) {
      auto callToInline = callsToInline.front();
      callsToInline.pop();

      auto F = callToInline->getCalledFunction();
      for (auto &B : *F) {
        for (auto &I : B) {
          if (auto call = dyn_cast<CallInst>(&I)) {
            auto func = call->getCalledFunction();
            if (func == nullptr || func->empty()) continue;
            funcToInline.insert(func);
          }
        }
      }

      InlineFunctionInfo IFI;
      InlineFunction(callToInline, IFI);
    }

    /*
     * Collect next level of queue push/pop calls to inline
     */
    for (auto &B : *function) {
      for (auto &I : B) {
        if (auto call = dyn_cast<CallInst>(&I)) {
          if (funcToInline.find(call->getCalledFunction()) != funcToInline.end()) {
            callsToInline.push(call);
          }
        }
      }
    }
  }
}

void ParallelizationTechnique::dumpToFile (LoopDependenceInfo &LDI) {
  std::error_code EC;
  raw_fd_ostream File("technique-dump-loop-" + std::to_string(LDI.getID()) + ".txt", EC, sys::fs::F_Text);

  if (EC) {
    errs() << "ERROR: Could not dump debug logs to file!";
    return ;
  }

  /*
   * Fetch the loop summary.
   */
  auto loopSummary = LDI.getLoopSummary();

  std::set<BasicBlock *> bbs(loopSummary->orderedBBs.begin(), loopSummary->orderedBBs.end());
  DGPrinter::writeGraph<SubCFGs>("technique-original-loop-" + std::to_string(LDI.getID()) + ".dot", new SubCFGs(bbs));
  DGPrinter::writeGraph<SCCDAG>("technique-sccdag-loop-" + std::to_string(LDI.getID()) + ".dot", LDI.sccdagAttrs.getSCCDAG());

  for (int i = 0; i < tasks.size(); ++i) {
    auto task = tasks[i];
    File << "===========\n";
    std::string taskName = "Task " + std::to_string(i) + ": ";
    task->getTaskBody()->print(File << taskName << "function" << "\n");
    File << "\n";

    File << taskName << "instruction clones" << "\n";
    for (auto origI : task->getOriginalInstructions()){
      origI->print(File << "Original: "); File << "\n\t";
      auto cloneI = task->getCloneOfOriginalInstruction(origI);
      cloneI->print(File << "Cloned: "); File << "\n";
    }
    File << "\n";

    File << taskName << "basic block clones" << "\n";
    for (auto origBB : task->getOriginalBasicBlocks()){
      origBB->printAsOperand(File << "Original: "); File << "\n\t";
      auto cloneBB = task->getCloneOfOriginalBasicBlock(origBB);
      cloneBB->printAsOperand(File << "Cloned: "); File << "\n";
    }
    File << "\n";

    File << taskName << "live in clones" << "\n";
    for (auto origLiveIn : task->getOriginalLiveIns()) {
      origLiveIn->print(File << "Original: "); File << "\n\t";
      auto cloneLiveIn = task->getCloneOfOriginalLiveIn(origLiveIn);
      cloneLiveIn->print(File << "Cloned: "); File << "\n";
    }
    File << "\n";
  }

  File.close();
}
