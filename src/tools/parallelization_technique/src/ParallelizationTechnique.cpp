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
using namespace llvm::noelle;

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
  auto loopSummary = LDI->getLoopStructure();
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
  auto loopSummary = LDI->getLoopStructure();
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

    PHINode *loopEntryProducerPHI = fetchLoopEntryPHIOfProducer(LDI, producer);
    auto initValPHIIndex = loopEntryProducerPHI->getBasicBlockIndex(loopPreHeader);
    auto initialValue = loopEntryProducerPHI->getIncomingValue(initValPHIIndex);
    initialValues[envInd] = castToCorrectReducibleType(*builder, initialValue, producer->getType());
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

  /*
   * Free the memory.
   */
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
  auto loopSummary = LDI->getLoopStructure();
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
    for (auto exitBB : LDI->getLoopStructure()->getLoopExitBasicBlocks()) {
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
  auto topLoop = LDI->getLoopStructure();
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

void ParallelizationTechnique::cloneMemoryLocationsLocallyAndRewireLoop (
  LoopDependenceInfo *LDI,
  int taskIndex
){

  auto task = this->tasks[taskIndex];
  auto rootLoop = LDI->getLoopStructure();
  auto memoryCloningAnalysis = LDI->getMemoryCloningAnalysis();

  for (auto location : memoryCloningAnalysis->getClonableMemoryLocations()) {

    /*
     * Check if this is an allocation used by this task
     */
    auto loopInstructionsRequiringClonedOperands = location->getLoopInstructionsUsingLocation();
    std::unordered_set<Instruction *> taskInstructions;
    for (auto I : loopInstructionsRequiringClonedOperands) {
      if (!task->isAnOriginalInstruction(I)) continue;
      taskInstructions.insert(I);
    }
    if (taskInstructions.size() == 0) continue;

    /*
     * If so, traverse operands of loop instructions to clone
     * all live-in references (casts and GEPs) of the allocation to clone
     * State all cloned instructions in the task's instruction map for data flow adjustment later
     */
    auto alloca = location->getAllocation();
    auto &entryBlock = (*task->getTaskBody()->begin());
    IRBuilder<> entryBuilder(&entryBlock);
    std::queue<Instruction *> instructionsToConvertOperandsOf;
    for (auto I : taskInstructions) {
      instructionsToConvertOperandsOf.push(I);
    }
    while (!instructionsToConvertOperandsOf.empty()) {
      auto I = instructionsToConvertOperandsOf.front();
      instructionsToConvertOperandsOf.pop();

      for (auto i = 0; i < I->getNumOperands(); ++i) {
        auto op = I->getOperand(i);

        /*
         * Ensure the instruction is outside the loop and not already cloned
         * FIXME: Checking task's instruction map would be mis-leading, as live-in values
         * could be listed as clones to these values. Find a way to ensure that wouldn't happen
         */
        auto opI = dyn_cast<Instruction>(op);
        if (!opI || rootLoop->isIncluded(opI)) continue;

        /*
         * Ensure the operand is a reference of the allocation
         * NOTE: Ignore checking for the allocation. That is cloned separately
         */
        if (!location->isInstructionCastOrGEPOfLocation(opI)) continue;

        /*
         * Ensure the instruction hasn't been cloned yet
         */
        if (task->isAnOriginalInstruction(opI)) continue;

        /*
         * Clone operand and then add to queue
         * NOTE: The operand clone is inserted before the insert point which
         * then gets set to itself. This ensures that any operand using it that has
         * already been traversed will come after
         */
        auto opCloneI = opI->clone();
        entryBuilder.Insert(opCloneI);
        entryBuilder.SetInsertPoint(opCloneI);
        instructionsToConvertOperandsOf.push(opI);

        /*
         * Swap the operand's live in mapping with this clone so the live-in allocation from
         * the caller of the dispatcher is NOT used; instead, we want the cloned allocation to be used
         *
         * NOTE: Add the instruction to the loop instruction map for data flow adjusting
         * to re-wire operands correctly
         */
        task->addLiveIn(opI, opCloneI);
        task->addInstruction(opI, opCloneI);
      }
    }

    /*
     * Clone the allocation and all other necessary instructions
     */
    auto allocaClone = alloca->clone();
    auto firstInst = &*entryBlock.begin();
    entryBuilder.SetInsertPoint(firstInst);
    entryBuilder.Insert(allocaClone);
    task->addInstruction(alloca, allocaClone);
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

  auto &taskFunction = *task->getTaskBody();
  DominatorTree taskDT(taskFunction);
  PostDominatorTree taskPDT(taskFunction);
  DominatorSummary taskDS(taskDT, taskPDT);

  /*
   * Iterate over live-out variables and inject stores at the end of the execution of the function of the task to propagate the new live-out values back to the caller of the parallelized loop.
   */
  auto envUser = this->envBuilder->getUser(taskIndex);
  for (auto envIndex : envUser->getEnvIndicesOfLiveOutVars()) {

    /*
     * Fetch the producer of the current live-out variable.
     * Fetch the clones of the producer. If none are specified in the one-to-many mapping,
     * assume the direct cloning of the producer is the only clone
     * TODO: Find a better place to map this single clone (perhaps when the original loop's values are cloned)
     */
    auto producer = (Instruction*)LDI->environment->producerAt(envIndex);
    if (!task->doesOriginalLiveOutHaveManyClones(producer)) {
      auto singleProducerClone = task->getCloneOfOriginalInstruction(producer);
      task->addLiveOut(producer, singleProducerClone);
    }
    auto producerClones = task->getClonesOfOriginalLiveOut(producer);

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
      auto identityV = getIdentityValueForEnvironmentValue(LDI, envIndex, envType);
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
    for (auto producerClone : producerClones) {

      auto insertBBs = this->determineLatestPointsToInsertLiveOutStore(LDI, taskIndex, producerClone, isReduced, taskDS);
      for (auto BB : insertBBs) {

        auto producerValueToStore = isReduced
          ? fetchOrCreatePHIForIntermediateProducerValueOfReducibleLiveOutVariable(LDI, taskIndex, envIndex, BB, taskDS)
          : producerClone;

        IRBuilder<> liveOutBuilder(BB);
        auto store = (StoreInst*)liveOutBuilder.CreateStore(producerValueToStore, envPtr);
        store->removeFromParent();
        store->insertBefore(BB->getTerminator());
      }
    }
  }

  return ;
}

std::set<BasicBlock *> ParallelizationTechnique::determineLatestPointsToInsertLiveOutStore (
  LoopDependenceInfo *LDI,
  int taskIndex,
  Instruction *liveOut,
  bool isReduced,
  DominatorSummary &taskDS
){
  auto task = this->tasks[taskIndex];

  /*
   * Fetch the header.
   */
  auto loopSummary = LDI->getLoopStructure();
  auto liveOutBlock = liveOut->getParent();

  /*
   * Insert stores in loop exit blocks
   * If the live out is reducible, it is fine that the live out value does not dominate the exit
   * as some other intermediate is guaranteed to
   */
  std::set<BasicBlock *> insertPoints;
  for (auto BB : loopSummary->getLoopExitBasicBlocks()) {
    auto cloneBB = task->getCloneOfOriginalBasicBlock(BB);
    auto liveOutDominatesExit = taskDS.DT.dominates(liveOutBlock, cloneBB);
    if (!isReduced && !liveOutDominatesExit) continue;
    insertPoints.insert(cloneBB);
  }

  /*
   * If the parallelization scheme introduced other loop exiting blocks,
   * and this live out is reducible, we must store the latest intermediate value for them
   */
  if (isReduced) {
    for (auto predecessor : predecessors(task->getExit())) {
      if (predecessor == task->getEntry()) continue;
      insertPoints.insert(predecessor);
    }
  }

  /*
   * HACK: If no exit block is dominated by the live out, the scheme is doing
   * short-circuiting logic of some sort on the loop's execution. State the live out's
   * block itself as a safe-guard.
   * TODO: Provide a way for each scheme to provide such an override instead of this blanket catch
   */
  if (insertPoints.empty()) {
    insertPoints.insert(liveOut->getParent());
  }

  return insertPoints;
}

Instruction * ParallelizationTechnique::fetchOrCreatePHIForIntermediateProducerValueOfReducibleLiveOutVariable (
  LoopDependenceInfo *LDI, 
  int taskIndex,
  int envIndex,
  BasicBlock *insertBasicBlock,
  DominatorSummary &taskDS
) {

  auto task = this->tasks[taskIndex];
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
   * If in the insert block there already exists a single intermediate,
   * return that intermediate
   */
  Instruction *lastIntermediateAtInsertBlock = nullptr;
  for (auto intermediateValue : intermediateValues) {
    if (intermediateValue->getParent() != insertBasicBlock) continue;
    if (lastIntermediateAtInsertBlock &&
      DT.dominates(intermediateValue, lastIntermediateAtInsertBlock)) continue;
    lastIntermediateAtInsertBlock = intermediateValue;
  }
  if (lastIntermediateAtInsertBlock) return lastIntermediateAtInsertBlock;

  /*
   * Produce PHI at the insert point
   */
  IRBuilder<> builder(insertBasicBlock->getFirstNonPHIOrDbgOrLifetime());
  auto producerType = producer->getType();
  auto phiNode = builder.CreatePHI(producerType, pred_size(insertBasicBlock));

  /*
   * Fetch all PHI node basic block predecessors
   * Determine all intermediate values dominating each predecessor
   * Determine the intermediate value of this set that dominates no other intermediates in the set
   */
  for (auto predIter = pred_begin(insertBasicBlock); predIter != pred_end(insertBasicBlock); ++predIter) {
    auto predecessor = *predIter;

    std::unordered_set<Instruction *> dominatingValues{};
    for (auto intermediateValue : intermediateValues) {
      auto intermediateBlock = intermediateValue->getParent();
      if (DT.dominates(intermediateBlock, predecessor)) {
        dominatingValues.insert(intermediateValue);
      }
    }

    assert(dominatingValues.size() > 0
      && "Cannot store reducible live out where no producer value dominates the point");

    std::unordered_set<Instruction *> lastDominatingValues{};
    for (auto value : dominatingValues) {
      bool isDominatingOthers = false;
      for (auto otherValue : dominatingValues) {
        if (value == otherValue) continue;
        if (!DT.dominates(value, otherValue)) continue;
        isDominatingOthers = true;
        break;
      }

      if (isDominatingOthers) continue;
      lastDominatingValues.insert(value);
    }

    assert(lastDominatingValues.size() == 1
      && "Cannot store reducible live out where no last produced value is known");
    auto lastDominatingIntermediateValue = *lastDominatingValues.begin();

    auto predecessorTerminator = predecessor->getTerminator();
    IRBuilder<> builderAtValue(predecessorTerminator);

    auto correctlyTypedValue = castToCorrectReducibleType(
      builderAtValue, lastDominatingIntermediateValue, producer->getType());
    phiNode->addIncoming(correctlyTypedValue, predecessor);
  } 

  return phiNode;
}

Value *ParallelizationTechnique::castToCorrectReducibleType (IRBuilder<> &builder, Value *value, Type *targetType) {
  auto valueTy = value->getType();
  if (valueTy == targetType) return value;

  if (valueTy->isIntegerTy() && targetType->isIntegerTy()) {
    return builder.CreateBitCast(value, targetType);
  } else if (valueTy->isIntegerTy() && targetType->isFloatingPointTy()) {
    return builder.CreateSIToFP(value, targetType);
  } else if (valueTy->isFloatingPointTy() && targetType->isIntegerTy()) {
    return builder.CreateFPToSI(value, targetType);
  } else if (valueTy->isFloatingPointTy() && targetType->isFloatingPointTy()) {
    return builder.CreateFPCast(value, targetType);
  } else assert(false && "Cannot cast to non-reducible type");

  return nullptr;
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
      assert(task->isAnOriginalBasicBlock(incomingBB));
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
  auto loopSummary = LDI->getLoopStructure();
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
    PHINode *loopEntryProducerPHI = fetchLoopEntryPHIOfProducer(LDI, producer);

    /*
     * Fetch the related instruction of the producer that has been created (cloned) and stored in the parallelized version of the loop.
     */
    auto producerClone = cast<PHINode>(task->getCloneOfOriginalInstruction(loopEntryProducerPHI));

    /*
     * Fetch the cloned pre-header index
     */
    auto incomingIndex = producerClone->getBasicBlockIndex(preheaderClone);
    assert(incomingIndex != -1 && "Loop entry present on producer PHI node");

    /*
     * Fetch the identity constant for the operation reduced.
     * For example, if the variable reduced is an accumulator where "+" is used to accumulate values, then "0" is the identity.
     */
    auto identityV = this->getIdentityValueForEnvironmentValue(LDI, envInd, loopEntryProducerPHI->getType());

    /*
     * Set the initial value for the private variable.
     */
    producerClone->setIncomingValue(incomingIndex, identityV);
  }

  return ;
}

PHINode * ParallelizationTechnique::fetchLoopEntryPHIOfProducer (
  LoopDependenceInfo *LDI,
  Value *producer
){

  auto sccdag = LDI->sccdagAttrs.getSCCDAG();
  auto producerSCC = sccdag->sccOfValue(producer);

  auto sccInfo = LDI->sccdagAttrs.getSCCAttrs(producerSCC);
  auto reducibleVariable = sccInfo->getSingleLoopCarriedVariable();
  assert(reducibleVariable != nullptr);

  PHINode *headerProducerPHI = reducibleVariable->getLoopEntryPHIForValueOfVariable(producer);
  assert(headerProducerPHI != nullptr &&
    "The reducible variable should be described by a single PHI in the header");
  return headerProducerPHI;
}

Value * ParallelizationTechnique::getIdentityValueForEnvironmentValue (
  LoopDependenceInfo *LDI,
  int environmentIndex,
  Type *typeForValue
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
    typeForValue
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

std::unordered_map<InductionVariable *, Value *> ParallelizationTechnique::cloneIVStepValueComputation (
  LoopDependenceInfo *LDI,
  int taskIndex,
  IRBuilder<> &insertBlock
) {

  auto task = tasks[taskIndex];
  auto loopSummary = LDI->getLoopStructure();
  auto allIVInfo = LDI->getInductionVariableManager();
  std::unordered_map<InductionVariable *, Value *> clonedStepSizeMap;

  /*
   * Clone each IV's step value described by the InductionVariable class
   */
  for (auto ivInfo : allIVInfo->getInductionVariables(*loopSummary)) {

    /*
     * If the step value is constant or a value present in the original loop, use its clone
     * TODO: Refactor this logic as a helper: tryAndFetchClone that doesn't assert a clone must exist
     */
    auto singleComputedStepValue = ivInfo->getSingleComputedStepValue();
    if (singleComputedStepValue) {
      Value *clonedStepValue = nullptr;
      if (isa<ConstantData>(singleComputedStepValue)
        || task->isAnOriginalLiveIn(singleComputedStepValue)) {
        clonedStepValue = singleComputedStepValue;
      } else if (auto singleComputedStepInst = dyn_cast<Instruction>(singleComputedStepValue)) {
        clonedStepValue = task->getCloneOfOriginalInstruction(singleComputedStepInst);
      }

      if (clonedStepValue) {
        clonedStepSizeMap.insert(std::make_pair(ivInfo, clonedStepValue));
        continue;
      }
    }

    /*
     * The step size is a composite SCEV. Fetch its instruction expansion,
     * cloning into the entry block of the function
     * 
     * NOTE: The step size is expected to be loop invariant
     */
    auto expandedInsts = ivInfo->getComputationOfStepValue();
    assert(expandedInsts.size() > 0);
    for (auto expandedInst : expandedInsts) {
      auto clonedInst = expandedInst->clone();
      task->addInstruction(expandedInst, clonedInst);
      insertBlock.Insert(clonedInst);
    }

    /*
     * Wire the instructions in the expansion to use the cloned values
     */
    for (auto expandedInst : expandedInsts) {
      adjustDataFlowToUseClones(task->getCloneOfOriginalInstruction(expandedInst), 0);
    }
    auto clonedStepValue = task->getCloneOfOriginalInstruction(expandedInsts.back());
    clonedStepSizeMap.insert(std::make_pair(ivInfo, clonedStepValue));
  }

  this->adjustStepValueOfPointerTypeIVToReflectPointerArithmetic(clonedStepSizeMap, insertBlock);

  return clonedStepSizeMap;
}

void ParallelizationTechnique::adjustStepValueOfPointerTypeIVToReflectPointerArithmetic (
  std::unordered_map<InductionVariable *, Value *> clonedStepValueMap,
  IRBuilder<> &insertBlock
) {

  /*
   * If the IV's type is pointer, then the SCEV of the step value for the IV is
   * pointer arithmetic and needs to be multiplied by the bit size of pointers to
   * reflect the exact change of the value
   * 
   * This occurs because GEP information is lost to ScalarEvolution analysis when it
   * computes the step value as a SCEV
   */
  auto &DL = this->module.getDataLayout();
  auto ptrSizeInBytes = DL.getPointerSize();
  for (auto ivAndStepValuePair : clonedStepValueMap) {
    auto iv = ivAndStepValuePair.first;
    auto value = ivAndStepValuePair.second;

    auto loopEntryPHI = iv->getLoopEntryPHI();
    if (!loopEntryPHI->getType()->isPointerTy()) continue;

    auto ptrSizeValue = ConstantInt::get(value->getType(), ptrSizeInBytes, false);
    auto adjustedStepValue = insertBlock.CreateMul(value, ptrSizeValue);
    clonedStepValueMap[iv] = adjustedStepValue;
  }

  return ;
}

float ParallelizationTechnique::computeSequentialFractionOfExecution (
  LoopDependenceInfo *LDI,
  Noelle &par
) const {

  auto sccManager = LDI->getSCCManager();
  auto sccdag = sccManager->getSCCDAG();
  float totalInstructionCount = 0, sequentialInstructionCount = 0;
  for (auto sccNode : sccdag->getNodes()) {
    auto scc = sccNode->getT();
    auto sccInfo = sccManager->getSCCAttrs(scc);
    auto sccType = sccInfo->getType();

    auto numInstructionsInSCC = scc->numInternalNodes();
    totalInstructionCount += numInstructionsInSCC;
    bool mustBeSynchronized = sccType == SCCAttrs::SCCType::SEQUENTIAL
      && !sccInfo->canBeCloned();
    if (mustBeSynchronized) {
      sequentialInstructionCount += numInstructionsInSCC;
    }
  }

  return sequentialInstructionCount / totalInstructionCount;
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
  auto loopSummary = LDI.getLoopStructure();

  std::set<BasicBlock *> bbs(loopSummary->orderedBBs.begin(), loopSummary->orderedBBs.end());
  DGPrinter::writeGraph<SubCFGs, BasicBlock>("technique-original-loop-" + std::to_string(LDI.getID()) + ".dot", new SubCFGs(bbs));
  DGPrinter::writeGraph<SCCDAG, SCC>("technique-sccdag-loop-" + std::to_string(LDI.getID()) + ".dot", LDI.sccdagAttrs.getSCCDAG());

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
