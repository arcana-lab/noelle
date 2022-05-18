/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/tools/ParallelizationTechnique.hpp"

namespace llvm::noelle { 

ParallelizationTechnique::ParallelizationTechnique (
  Noelle &n
  )
  : noelle{n}, tasks{}, envBuilder{nullptr}
  {
  this->verbose = n.getVerbosity();

  return ;
}

Value * ParallelizationTechnique::getEnvArray (void) const { 
  return envBuilder->getEnvironmentArray(); 
}

void ParallelizationTechnique::initializeEnvironmentBuilder (
  LoopDependenceInfo *LDI,
  std::set<uint32_t> simpleVars,
  std::set<uint32_t> reducableVars
) {

  /*
   * Fetch the program.
   */
  auto program = this->noelle.getProgram();

  /*
   * Check the state of the parallelization technique 'this'.
   */
  if (this->tasks.size() == 0) {
    errs() << "ERROR: Parallelization technique tasks haven't been created yet!\n"
      << "\tTheir environment builders can't be initialized until they are.\n";
    abort();
  }

  /*
   * Fetch the environment of the loop
   */
  auto environment = LDI->getEnvironment();
  assert(environment != nullptr);

  /*
   * Collect the Type of each environment variable
   */
  std::vector<Type *> varTypes;
  for (int64_t i = 0; i < environment->size(); ++i) {
    varTypes.push_back(environment->typeOfEnvironmentLocation(i));
  }

  /*
   * Create the environment builder
   */
  this->envBuilder = new LoopEnvironmentBuilder(program->getContext(), varTypes, simpleVars, reducableVars, this->numTaskInstances, tasks.size());

  /*
   * Create the users of the environment: one user per task.
   */
  for (auto i = 0; i < this->tasks.size(); ++i) {

    /*
     * Fetch the current task and the related environment-user.
     */
    auto task = this->tasks[i];
    assert(task != nullptr);
    auto envUser = envBuilder->getUser(i);
    assert(envUser != nullptr);

    /*
     * Generate code within the current task to cast the generic pointer to the type of the environment it points to.
     */
    auto entryBlock = task->getEntry();
    IRBuilder<> entryBuilder(entryBlock);
    envUser->setEnvArray(entryBuilder.CreateBitCast(
      task->getEnvironment(),
      PointerType::getUnqual(envBuilder->getEnvironmentArrayType())
    ));
  }

  return ;
}

void ParallelizationTechnique::initializeEnvironmentBuilder (
  LoopDependenceInfo *LDI,
  std::set<uint32_t> nonReducableVars
  ){
  std::set<uint32_t> emptySet{};

  this->initializeEnvironmentBuilder(LDI, nonReducableVars, emptySet);

  return ;
}

void ParallelizationTechnique::allocateEnvironmentArray (LoopDependenceInfo *LDI) {

  /*
   * Fetch the loop function.
   */
  auto loopStructure = LDI->getLoopStructure();
  auto loopFunction = loopStructure->getFunction();

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

  /*
   * Fetch the loop environment.
   */
  auto env = LDI->getEnvironment();

  /*
   * Store live-in values into the environment just before jumping to the parallelized loop.
   */
  IRBuilder<> builder(this->entryPointOfParallelizedLoop);
  for (auto envIndex : env->getEnvIndicesOfLiveInVars()) {

    /*
     * Fetch the value to store.
     */
    auto producerOfLiveIn = env->producerAt(envIndex);

    /*
     * Fetch the memory location inside the environment dedicated to the live-in value.
     */
    auto environmentVariable = this->envBuilder->getEnvironmentVariable(envIndex);

    /*
     * Store the value inside the environment.
     */
    builder.CreateStore(producerOfLiveIn, environmentVariable);
  }

  return ;
}

BasicBlock * ParallelizationTechnique::performReductionToAllReducableLiveOutVariables (LoopDependenceInfo *LDI, Value *numberOfThreadsExecuted) {
  auto builder = new IRBuilder<>(this->entryPointOfParallelizedLoop);

  /*
   * Fetch the loop headers.
   */
  auto loopSummary = LDI->getLoopStructure();
  auto loopPreHeader = loopSummary->getPreHeader();

  /*
   * Fetch the SCC manager.
   */
  auto sccManager = LDI->getSCCManager();

  /*
   * Fetch the environment of the loop
   */
  auto environment = LDI->getEnvironment();
  assert(environment != nullptr);

  /*
   * Collect reduction operation information needed to accumulate reducable variables after parallelization execution
   */
  std::unordered_map<int, int> reducableBinaryOps;
  std::unordered_map<int, Value *> initialValues;
  for (auto envInd : environment->getEnvIndicesOfLiveOutVars()) {
    auto isReduced = envBuilder->isVariableReducable(envInd);
    if (!isReduced) continue;

    auto producer = environment->producerAt(envInd);
    auto producerSCC = sccManager->getSCCDAG()->sccOfValue(producer);
    auto producerSCCAttributes = sccManager->getSCCAttrs(producerSCC);

    /*
     * HACK: Need to get accumulator that feeds directly into producer PHI, not any intermediate one
     */
    auto firstAccumI = *(producerSCCAttributes->getAccumulators().begin());
    auto binOpCode = firstAccumI->getOpcode();
    reducableBinaryOps[envInd] = sccManager->accumOpInfo.accumOpForType(binOpCode, producer->getType());

    PHINode *loopEntryProducerPHI = this->fetchLoopEntryPHIOfProducer(LDI, producer);
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

  for (int envInd : environment->getEnvIndicesOfLiveOutVars()) {
    auto prod = environment->producerAt(envInd);

    /*
     * If the environment variable isn't reduced, it is held in allocated memory that needs to be loaded from in order to retrieve the value.
     */
    auto isReduced = envBuilder->isVariableReducable(envInd);
    Value *envVar;
    if (isReduced) {
      envVar = envBuilder->getAccumulatedReducableEnvironmentVariable(envInd);
    } else {
      envVar = afterReductionBuilder->CreateLoad(envBuilder->getEnvironmentVariable(envInd));
    }
    assert(envVar != nullptr);

    for (auto consumer : environment->consumersOf(prod)) {
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

void ParallelizationTechnique::addPredecessorAndSuccessorsBasicBlocksToTasks (
  LoopDependenceInfo *LDI,
  std::vector<Task *> taskStructs
) {
  assert(this->tasks.size() == 0);

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
   * Fetch the loop structure.
   */
  auto loopStructure = LDI->getLoopStructure();

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
    task->extractFuncArgs();

    /*
     * Fetch the entry and exit basic blocks of the current task.
     */
    auto entryBB = task->getEntry();
    auto exitBB = task->getExit();
    assert(entryBB != nullptr);
    assert(exitBB != nullptr);

    /*
     * Map original preheader to entry block
     */
    task->addBasicBlock(loopPreHeader, task->getEntry());

    /*
     * Create one basic block per loop exit, mapping between originals and clones,
     * and branching from them to the function exit block
     */
    for (auto exitBB : loopStructure->getLoopExitBasicBlocks()) {
      auto newExitBB = task->addBasicBlockStub(exitBB);
      task->tagBasicBlockAsLastBlock(newExitBB);
      IRBuilder<> builder(newExitBB);
      builder.CreateBr(task->getExit());
    }
  }

  return ;
}

void ParallelizationTechnique::cloneSequentialLoop (
  LoopDependenceInfo *LDI,
  int taskIndex
){
  assert(LDI != nullptr);
  assert(taskIndex < this->tasks.size());


  /*
   * Fetch the program.
   */
  auto program = this->noelle.getProgram();

  /*
   * Fetch the task.
   */
  auto &cxt = program->getContext();
  auto task = this->tasks[taskIndex];

  /*
   * Code to filter out instructions we don't want to clone.
   */
  auto filter = [](Instruction *inst) -> bool{
    if (auto call = dyn_cast<CallInst>(inst)){
      if (call->isLifetimeStartOrEnd()){
        return false;
      }
    }

    return true;
  };
 
  /*
   * Clone all basic blocks of the original loop
   */
  auto topLoop = LDI->getLoopStructure();
  for (auto originBB : topLoop->getBasicBlocks()) {

    /*
     * Clone the basic block.
     */
    task->cloneAndAddBasicBlock(originBB, filter);
  }

  return ;
}

void ParallelizationTechnique::cloneSequentialLoopSubset (
  LoopDependenceInfo *LDI,
  int taskIndex,
  std::set<Instruction *> subset
){

  /*
   * Fetch the program.
   */
  auto program = this->noelle.getProgram();

  auto &cxt = program->getContext();
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

  /*
   * Fetch the task and other loop-specific abstractions.
   */
  auto task = this->tasks[taskIndex];
  assert(task != nullptr);
  auto rootLoop = LDI->getLoopStructure();
  auto memoryCloningAnalysis = LDI->getMemoryCloningAnalysis();
  auto envUser = this->envBuilder->getUser(taskIndex);

  task->getTaskBody()->print(errs());
  rootLoop->getFunction()->print(errs());

  /*
   * Fetch the environment of the loop
   */
  auto environment = LDI->getEnvironment();
  assert(environment != nullptr);

  /*
   * Check every stack object that can be safely cloned.
   */
  for (auto location : memoryCloningAnalysis->getClonableMemoryLocations()) {

    /*
     * Fetch the stack object.
     */
    auto alloca = location->getAllocation();

    /*
     * Check if this is an allocation used by this task
     */
    auto loopInstructionsRequiringClonedOperands = location->getLoopInstructionsUsingLocation();
    std::unordered_set<Instruction *> taskInstructions;
    for (auto I : loopInstructionsRequiringClonedOperands) {
      if (!task->isAnOriginalInstruction(I)) {
        continue;
      }
      taskInstructions.insert(I);
    }
    if (taskInstructions.size() == 0) {

      /*
       * The current stack object is not used by the task/loop.
       */
      continue;
    }

    /*
     *
     * The stack object can be safely cloned (thanks to the object-cloning analysis) and it is used by our loop.
     *
     * First, we need to remove the alloca instruction to be a live-in.
     */
    task->removeLiveIn(alloca);

    /*
     * Now we need to traverse operands of loop instructions to clone
     * all live-in references (casts and GEPs) of the allocation to clone
     * State all cloned instructions in the task's instruction map for data flow adjustment later
     */
    auto &entryBlock = (*task->getTaskBody()->begin());
    auto firstInstruction = &*entryBlock.begin();
    IRBuilder<> entryBuilder(&entryBlock);
    std::queue<Instruction *> instructionsToConvertOperandsOf;
    for (auto I : taskInstructions) {
      instructionsToConvertOperandsOf.push(I);
    }
    auto locationOutsideUses = location->getInstructionsUsingLocationOutsideLoop();
    for (auto I : locationOutsideUses) {
      instructionsToConvertOperandsOf.push(I);
    }
    while (!instructionsToConvertOperandsOf.empty()) {

      /*
       * Fetch the current instruction that needs patching and/or cloning.
       */
      auto I = instructionsToConvertOperandsOf.front();
      instructionsToConvertOperandsOf.pop();

      for (auto i = 0; i < I->getNumOperands(); ++i) {
        auto op = I->getOperand(i);

        /*
         * Ensure the instruction is outside the loop and not already cloned
         *
         * FIXME: Checking task's instruction map would be mis-leading, as live-in values
         * could be listed as clones to these values. Find a way to ensure that wouldn't happen
         */
        auto opI = dyn_cast<Instruction>(op);
        if (!opI || rootLoop->isIncluded(opI)) {
          continue;
        }

        /*
         * Ensure the operand is a reference of the allocation
         * NOTE: Ignore checking for the allocation. That is cloned separately
         */
        if (!location->isInstructionCastOrGEPOfLocation(opI)) {
          continue;
        }

        /*
         * Ensure the instruction hasn't been cloned yet
         */
        if (task->isAnOriginalInstruction(opI)) {
          continue;
        }

        /*
         * Clone operand and then add to queue
         *
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

        /*
         * Check if there are new live-in values we need to pass to the task.
         */
        for (auto j = 0; j < opI->getNumOperands(); ++j) {

          /*
           * Fetch the current operand.
           */
          auto opJ = opI->getOperand(j);
          if (dyn_cast<Constant>(opJ)){

            /*
             * The current operand is a constant.
             * There is no need for a live-in.
             */
            continue ;
          }

          /*
           * Check if the current operand is the alloca instruction that will be cloned.
           */
          if (opJ == alloca){
            assert(!task->isAnOriginalLiveIn(opJ));
            continue ;
          }

          /*
           * Check if the current operand requires to become a live-in.
           */
          auto newLiveIn = true;
          for (auto envIndex : envUser->getEnvIndicesOfLiveInVars()) {
            auto producer = environment->producerAt(envIndex);
            if (producer == opJ){
              newLiveIn = false;
              break;
            }
          }
          if (!newLiveIn){
            continue ;
          }

          /*
           * The current operand must become a new live-in.
           *
           * Make space in the environment for the new live-in.
           */
          auto newLiveInEnvironmentIndex = environment->addLiveInValue(opJ, {opI});
          this->envBuilder->addVariableToEnvironment(newLiveInEnvironmentIndex, opJ->getType());

          /*
           * Declare the new live-in of the loop is also a new live-in for the user (i.e., task) of the environment specified bt the input (i.e., taskIndex).
           */
          envUser->addLiveInIndex(newLiveInEnvironmentIndex);

          /*
           * Add the load inside the task to load from the environment the new live-in.
           */
          envUser->createEnvPtr(entryBuilder, newLiveInEnvironmentIndex, opJ->getType());
          auto environmentLocationLoad = entryBuilder.CreateLoad(envUser->getEnvPtr(newLiveInEnvironmentIndex));

          /*
           * Make the task aware that the new load represents the live-in value.
           */
          task->addLiveIn(opJ, environmentLocationLoad);
        }
      }
    }

    /*
     * Clone the stack object at the beginning of the task.
     */
    auto allocaClone = alloca->clone();
    auto firstInst = &*entryBlock.begin();
    entryBuilder.SetInsertPoint(firstInst);
    entryBuilder.Insert(allocaClone);

    /*
     * Keep track of the original-clone mapping.
     */
    task->addInstruction(alloca, allocaClone);
  }
  task->getTaskBody()->print(errs());
  rootLoop->getFunction()->print(errs());

  return ;
}

void ParallelizationTechnique::generateCodeToLoadLiveInVariables (
  LoopDependenceInfo *LDI, 
  int taskIndex
){

  /*
   * Fetch the task.
   */
  auto task = this->tasks[taskIndex];

  /*
   * Fetch the user of the environment attached to the task.
   */
  auto envUser = this->envBuilder->getUser(taskIndex);

  /*
   * Generate the loads to load values from the live-in environment variables.
   */
  IRBuilder<> builder(task->getEntry());
  for (auto envIndex : envUser->getEnvIndicesOfLiveInVars()) {

    /*
     * Fetch the current producer of the original code that generates the live-in value.
     */
    auto producer = LDI->getEnvironment()->producerAt(envIndex);

    /*
     * Create GEP access of the environment variable at the given index
     */
    envUser->createEnvPtr(builder, envIndex, producer->getType());
    auto envPointer = envUser->getEnvPtr(envIndex);

    /*
     * Load the live-in value from the environment pointer.
     */
    auto envLoad = builder.CreateLoad(envPointer);

    /*
     * Register the load as a "clone" of the original producer
     */
    task->addLiveIn(producer, envLoad);
  }

  return ;
}

void ParallelizationTechnique::generateCodeToStoreLiveOutVariables (
  LoopDependenceInfo *LDI, 
  int taskIndex
){

  /*
   * Fetch the requested task.
   */
  auto task = this->tasks[taskIndex];
  assert(task != nullptr);

  /*
   * Create a builder that points to the entry point of the function executed by the task.
   */
  auto entryBlock = task->getEntry();
  assert(entryBlock != nullptr);
  auto entryTerminator = entryBlock->getTerminator();
  assert(entryTerminator != nullptr);
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
    auto producer = (Instruction*)LDI->getEnvironment()->producerAt(envIndex);
    assert(producer != nullptr);
    if (!task->doesOriginalLiveOutHaveManyClones(producer)) {
      auto singleProducerClone = task->getCloneOfOriginalInstruction(producer);
      task->addLiveOut(producer, singleProducerClone);
    }

    auto producerClones = task->getClonesOfOriginalLiveOut(producer);

    /*
     * Create GEP access of the single, or reducable, environment variable
     */
    auto envType = producer->getType();
    auto isReduced = this->envBuilder->isVariableReducable(envIndex);
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
      auto identityV = this->getIdentityValueForEnvironmentValue(LDI, envIndex, envType);
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

  /*
   * Fetch the SCC manager.
   */
  auto sccManager = LDI->getSCCManager();

  auto task = this->tasks[taskIndex];
  auto &DT = taskDS.DT;
  auto &PDT = taskDS.PDT;

  /*
   * Fetch all clones of intermediate values of the producer
   */
  auto producer = (Instruction*)LDI->getEnvironment()->producerAt(envIndex);
  auto producerSCC = sccManager->getSCCDAG()->sccOfValue(producer);

  std::set<Instruction *> intermediateValues{};
  for (auto originalPHI : sccManager->getSCCAttrs(producerSCC)->getPHIs()) {
    intermediateValues.insert(task->getCloneOfOriginalInstruction(originalPHI));
  }
  for (auto originalI : sccManager->getSCCAttrs(producerSCC)->getAccumulators()) {
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

  /*
   * Fetch the task.
   */
  auto &task = tasks[taskIndex];

  /*
   * Rewire the data flows.
   */
  for (auto origI : task->getOriginalInstructions()) {
    auto cloneI = task->getCloneOfOriginalInstruction(origI);
    this->adjustDataFlowToUseClones(cloneI, taskIndex);
  }

  return ;
}

void ParallelizationTechnique::adjustDataFlowToUseClones (
  Instruction *cloneI,
  int taskIndex
){

  /*
   * Fetch the task.
   */
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

  /*
   * Handle PHI instructions.
   */
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

    /*
     * Fetch the current operand of @cloneI
     */
    auto opV = op.get();

    /*
     * If the value is a constant, then there is nothing we need to do.
     */
    if (dyn_cast<Constant>(opV)){
      continue ;
    }

    /*
     * If the value is a loop live-in one, set it to the value loaded from the loop environment passed to the task.
     */
    if (task->isAnOriginalLiveIn(opV)){
      auto internalValue = task->getCloneOfOriginalLiveIn(opV);
      op.set(internalValue);
      continue ;
    }

    /*
     * The value is not a live-in.
     *
     * If the value is generated by another instruction within the task, then set it to the equivalent cloned instruction.
     */
    if (auto opI = dyn_cast<Instruction>(opV)) {
      if (task->isAnOriginalInstruction(opI)){
        auto cloneOpI = task->getCloneOfOriginalInstruction(opI);
        op.set(cloneOpI);

      } else {
        if (opI->getFunction() != task->getTaskBody()) {
          cloneI->print(errs() << "ERROR:   Instruction has op from another function: "); errs() << "\n";
          opI->print(errs() << "ERROR:   Op: "); errs() << "\n";
          task->getTaskBody()->print(errs() << "ERROR: Task body ");
          opI->getFunction()->print(errs());
          abort();
        }
      }
    }
  }

  return ;
}

void ParallelizationTechnique::setReducableVariablesToBeginAtIdentityValue (
  LoopDependenceInfo *LDI,
  int taskIndex
){

  /*
   * Fetch the task.
   */
  assert(taskIndex < this->tasks.size());
  auto task = this->tasks[taskIndex];
  assert(task != nullptr);

  /*
   * Fetch task information.
   */
  auto loopStructure = LDI->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();
  auto headerClone = task->getCloneOfOriginalBasicBlock(loopHeader);
  assert(headerClone != nullptr);
  auto loopPreHeader = loopStructure->getPreHeader();
  auto preheaderClone = task->getCloneOfOriginalBasicBlock(loopPreHeader);
  assert(preheaderClone != nullptr);

  /*
   * Fetch the environment of the loop
   */
  auto environment = LDI->getEnvironment();
  assert(environment != nullptr);

  /*
   * Iterate over live-out variables.
   */
  for (auto envInd : environment->getEnvIndicesOfLiveOutVars()) {

    /*
     * Check if the current live-out variable can be reduced.
     */
    auto isThisLiveOutVarReducable = this->envBuilder->isVariableReducable(envInd);
    if (!isThisLiveOutVarReducable) {
      continue;
    }

    /*
     * Fetch the instruction that produces the live-out variable.
     * The reducible live out must be contained within an SCC that has a
     * PHI node in the header. The incoming value from the preheader is the
     * location of the initial value that needs to be changed
     */
    auto producer = environment->producerAt(envInd);
    assert(producer != nullptr);
    auto loopEntryProducerPHI = this->fetchLoopEntryPHIOfProducer(LDI, producer);
    assert(loopEntryProducerPHI != nullptr);

    /*
     * Fetch the related instruction of the producer that has been created (cloned) and stored in the parallelized version of the loop.
     */
    auto producerClone = cast<PHINode>(task->getCloneOfOriginalInstruction(loopEntryProducerPHI));
    assert(producerClone != nullptr);

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

  /*
   * Fetch the SCC manager.
   */
  auto sccManager = LDI->getSCCManager();

  auto sccdag = sccManager->getSCCDAG();
  auto producerSCC = sccdag->sccOfValue(producer);

  auto sccInfo = sccManager->getSCCAttrs(producerSCC);
  auto reducibleVariable = sccInfo->getSingleLoopCarriedVariable();
  assert(reducibleVariable != nullptr);

  auto headerProducerPHI = reducibleVariable->getLoopEntryPHIForValueOfVariable(producer);
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
   * Fetch the SCC manager.
   */
  auto sccManager = LDI->getSCCManager();

  /*
   * Fetch the environment of the loop
   */
  auto environment = LDI->getEnvironment();
  assert(environment != nullptr);

  /*
   * Fetch the producer of new values of the current environment variable.
   */
  auto producer = environment->producerAt(environmentIndex);

  /*
   * Fetch the SCC that this producer belongs to.
   */
  auto producerSCC = sccManager->getSCCDAG()->sccOfValue(producer);
  assert(producerSCC != nullptr && "The environment value doesn't belong to a loop SCC");

  /*
   * Fetch the attributes about the producer SCC.
   */
  auto sccAttrs = sccManager->getSCCAttrs(producerSCC);
  assert(sccAttrs->numberOfAccumulators() > 0 && "The environment value isn't accumulated!");

  /*
   * Fetch the accumulator.
   */
  auto firstAccumI = *(sccAttrs->getAccumulators().begin());

  /*
   * Fetch the identity.
   */
  auto identityValue = sccManager->accumOpInfo.generateIdentityFor(
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
   * Fetch the program.
   */
  auto program = this->noelle.getProgram();

  /*
   * Check whether there are multiple exit blocks or not.
   * If there are more exit blocks, then we need to specify which one has been taken.
   */
  auto task = this->tasks[taskIndex];
  if (task->getNumberOfLastBlocks() == 1) {
    return ;
  }

  /*
   * Fetch the environment of the loop
   */
  auto environment = LDI->getEnvironment();
  assert(environment != nullptr);

  /*
   * There are multiple exit blocks.
   *
   * Fetch the pointer of the location where the exit block ID taken will be stored.
   */
  auto exitBlockEnvIndex = environment->indexOfExitBlockTaken();
  assert(exitBlockEnvIndex != -1);
  auto envUser = this->envBuilder->getUser(taskIndex);
  auto entryTerminator = task->getEntry()->getTerminator();
  IRBuilder<> entryBuilder(entryTerminator);

  auto envType = environment->typeOfEnvironmentLocation(exitBlockEnvIndex);
  envUser->createEnvPtr(entryBuilder, exitBlockEnvIndex, envType);

  /*
   * Add a store instruction to specify to the code outside the parallelized loop which exit block is taken.
   */
  auto int32 = IntegerType::get(program->getContext(), 32);
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
      if (isa<ConstantData>(singleComputedStepValue)){
        clonedStepValue = singleComputedStepValue;

      } else if (task->isAnOriginalLiveIn(singleComputedStepValue)){
        clonedStepValue = task->getCloneOfOriginalLiveIn(singleComputedStepValue);

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
   * Fetch the program.
   */
  auto program = this->noelle.getProgram();

  /*
   * If the IV's type is pointer, then the SCEV of the step value for the IV is
   * pointer arithmetic and needs to be multiplied by the bit size of pointers to
   * reflect the exact change of the value
   * 
   * This occurs because GEP information is lost to ScalarEvolution analysis when it
   * computes the step value as a SCEV
   */
  auto &DL = program->getDataLayout();
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
   * Fetch the loop structure.
   */
  auto loopSummary = LDI.getLoopStructure();

  /*
   * Fetch the SCC manager.
   */
  auto sccManager = LDI.getSCCManager();

  auto allBBs = loopSummary->getBasicBlocks();
  std::set<BasicBlock *> bbs(allBBs.begin(), allBBs.end());
  DGPrinter::writeGraph<SubCFGs, BasicBlock>("technique-original-loop-" + std::to_string(LDI.getID()) + ".dot", new SubCFGs(bbs));
  DGPrinter::writeGraph<SCCDAG, SCC>("technique-sccdag-loop-" + std::to_string(LDI.getID()) + ".dot", sccManager->getSCCDAG());

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

ParallelizationTechnique::~ParallelizationTechnique () {
  return ;
}
      
BasicBlock * ParallelizationTechnique::getParLoopEntryPoint (void) const { 
  return entryPointOfParallelizedLoop; 
}

BasicBlock * ParallelizationTechnique::getParLoopExitPoint (void) const { 
  return exitPointOfParallelizedLoop; 
}

}
