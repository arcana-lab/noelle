/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/tools/ParallelizationTechnique.hpp"
#include "noelle/core/ReductionSCC.hpp"
#include "noelle/core/BinaryReductionSCC.hpp"
#include "noelle/core/LoopCarriedUnknownSCC.hpp"

namespace arcana::noelle {

ParallelizationTechnique::ParallelizationTechnique(Noelle &n)
  : noelle{ n },
    envBuilder{ nullptr },
    tasks{},
    entryPointOfParallelizedLoop{ nullptr },
    exitPointOfParallelizedLoop{ nullptr },
    numTaskInstances{ 0 } {
  this->verbose = n.getVerbosity();
}

Value *ParallelizationTechnique::getEnvArray(void) const {
  assert(this->envBuilder != nullptr);
  return this->envBuilder->getEnvironmentArray();
}

uint32_t ParallelizationTechnique::getIndexOfEnvironmentVariable(
    uint32_t id) const {
  assert(this->envBuilder != nullptr);

  auto envVar = this->envBuilder->getIndexOfEnvironmentVariable(id);

  return envVar;
}

void ParallelizationTechnique::initializeEnvironmentBuilder(
    LoopContent *loopContent,
    std::set<uint32_t> nonReducableVars) {
  std::set<uint32_t> emptySet{};

  this->initializeEnvironmentBuilder(loopContent, nonReducableVars, emptySet);
}

void ParallelizationTechnique::initializeEnvironmentBuilder(
    LoopContent *loopContent,
    std::set<uint32_t> simpleVars,
    std::set<uint32_t> reducableVars) {
  auto isReducable = [&reducableVars](uint32_t variableID,
                                      bool isLiveOut) -> bool {
    if (reducableVars.find(variableID) != reducableVars.end()) {
      return true;
    }
    return false;
  };
  this->initializeEnvironmentBuilder(loopContent, isReducable);
}

void ParallelizationTechnique::initializeEnvironmentBuilder(
    LoopContent *loopContent,
    std::function<bool(uint32_t variableID, bool isLiveOut)>
        shouldThisVariableBeReduced) {
  auto shouldThisVariableBeSkipped =
      [](uint32_t variableID, bool isLiveOut) -> bool { return false; };
  this->initializeEnvironmentBuilder(loopContent,
                                     shouldThisVariableBeReduced,
                                     shouldThisVariableBeSkipped);
}

void ParallelizationTechnique::initializeEnvironmentBuilder(
    LoopContent *loopContent,
    std::function<bool(uint32_t variableID, bool isLiveOut)>
        shouldThisVariableBeReduced,
    std::function<bool(uint32_t variableID, bool isLiveOut)>
        shouldThisVariableBeSkipped) {
  assert(loopContent != nullptr);

  /*
   * Fetch the environment of the loop
   */
  auto environment = loopContent->getEnvironment();
  assert(environment != nullptr);

  /*
   * Check the state of the parallelization technique 'this'.
   */
  if (this->tasks.size() == 0) {
    errs()
        << "ERROR: Parallelization technique tasks haven't been created yet!\n"
        << "\tTheir environment builders can't be initialized until they are.\n";
    abort();
  }

  /*
   * Generate code to allocate and initialize the loop environment.
   */
  auto program = this->noelle.getProgram();
  this->envBuilder = new LoopEnvironmentBuilder(program->getContext(),
                                                environment,
                                                shouldThisVariableBeReduced,
                                                shouldThisVariableBeSkipped,
                                                this->numTaskInstances,
                                                this->tasks.size());

  /*
   * Create the users of the environment: one user per task.
   */
  this->initializeLoopEnvironmentUsers();
}

void ParallelizationTechnique::initializeLoopEnvironmentUsers(void) {

  /*
   * Create the users of the environment: one user per task.
   */
  for (auto i = 0u; i < this->tasks.size(); ++i) {

    /*
     * Fetch the current task and the related environment-user.
     */
    auto task = this->tasks[i];
    assert(task != nullptr);
    auto envUser = this->envBuilder->getUser(i);
    assert(envUser != nullptr);

    /*
     * Generate code within the current task to cast the generic pointer to the
     * type of the environment it points to.
     */
    auto entryBlock = task->getEntry();
    IRBuilder<> entryBuilder(entryBlock);
    auto bitcastInst = entryBuilder.CreateBitCast(
        task->getEnvironment(),
        PointerType::getUnqual(envBuilder->getEnvironmentArrayType()),
        "noelle.environment_variable.pointer");
    envUser->setEnvironmentArray(bitcastInst);
  }
}

void ParallelizationTechnique::allocateEnvironmentArray(
    LoopContent *loopContent) {

  /*
   * Fetch the loop function.
   */
  auto loopStructure = loopContent->getLoopStructure();
  auto loopFunction = loopStructure->getFunction();

  /*
   * Fetch the first instruction of the first basic block of the function that
   * includes the loop we want to parallelized.
   */
  auto firstBB = loopFunction->begin();
  auto firstI = firstBB->begin();

  /*
   * Generate the environment.
   */
  IRBuilder<> builder(&*firstI);
  envBuilder->allocateEnvironmentArray(builder);
  envBuilder->generateEnvVariables(builder);
}

void ParallelizationTechnique::populateLiveInEnvironment(
    LoopContent *loopContent) {

  /*
   * Fetch the metadata manager
   */
  auto mm = this->noelle.getMetadataManager();

  /*
   * Fetch the loop environment.
   */
  auto env = loopContent->getEnvironment();

  /*
   * Store live-in values into the environment just before jumping to the
   * parallelized loop.
   */
  IRBuilder<> builder(this->entryPointOfParallelizedLoop);
  for (auto envID : env->getEnvIDsOfLiveInVars()) {

    /*
     * Skip the environment variable if it's not included in the builder
     */
    if (!this->envBuilder->isIncludedEnvironmentVariable(envID)) {
      continue;
    }

    /*
     * Fetch the value to store.
     */
    auto producerOfLiveIn = env->getProducer(envID);

    /*
     * Fetch the memory location inside the environment dedicated to the live-in
     * value.
     */
    auto environmentVariable = this->envBuilder->getEnvironmentVariable(envID);

    /*
     * Store the value inside the environment.
     */
    auto newStore = builder.CreateStore(producerOfLiveIn, environmentVariable);

    /*
     * Attach the metadata to the new store
     */
    mm->addMetadata(newStore,
                    "noelle.environment_variable.live_in.store_pointer",
                    std::to_string(envID));
  }
}

BasicBlock *ParallelizationTechnique::
    performReductionToAllReducableLiveOutVariables(
        LoopContent *loopContent,
        Value *numberOfThreadsExecuted) {

  /*
   * Fetch the loop structure.
   */
  auto loopSummary = loopContent->getLoopStructure();

  /*
   * Fetch the SCCDAG.
   */
  auto sccManager = loopContent->getSCCManager();
  auto loopSCCDAG = sccManager->getSCCDAG();

  /*
   * Fetch the environment of the loop
   */
  auto environment = loopContent->getEnvironment();
  assert(environment != nullptr);

  /*
   * Collect reduction operation information needed to accumulate reducable
   * variables after parallelization execution
   */
  std::unordered_map<uint32_t, BinaryReductionSCC *> reductions;
  std::map<ReductionSCC *, Value *> fromReductionToProducer;
  for (auto envID : environment->getEnvIDsOfLiveOutVars()) {

    /*
     * Check if the current live-out variable was reduced.
     */
    auto isReduced = this->envBuilder->hasVariableBeenReduced(envID);
    if (!isReduced) {
      continue;
    }

    /*
     * The current live-out variable has been reduced.
     *
     * Collect information about the reduction
     */
    auto producer = environment->getProducer(envID);
    auto producerSCC = loopSCCDAG->sccOfValue(producer);
    auto producerSCCAttributes =
        cast<BinaryReductionSCC>(sccManager->getSCCAttrs(producerSCC));
    assert(producerSCCAttributes != nullptr);

    /*
     * Keep track about the reduction.
     */
    reductions[envID] = producerSCCAttributes;
    fromReductionToProducer[producerSCCAttributes] = producer;
  }

  /*
   * Generate the code to perform the reduction.
   */
  IRBuilder<> builder{ this->entryPointOfParallelizedLoop };
  auto castF =
      [this, &builder, &fromReductionToProducer](ReductionSCC *red) -> Value * {
    auto p = fromReductionToProducer.at(red);
    auto initialValue = red->getInitialValue();
    auto i =
        this->castToCorrectReducibleType(builder, initialValue, p->getType());
    return i;
  };
  auto afterReductionB = this->envBuilder->reduceLiveOutVariables(
      this->entryPointOfParallelizedLoop,
      builder,
      reductions,
      numberOfThreadsExecuted,
      castF);

  /*
   * If reduction occurred, then all environment loads to propagate live outs
   * need to be inserted after the reduction loop
   */
  auto afterReductionBuilder = new IRBuilder<>(afterReductionB);
  if (afterReductionB->getTerminator()) {
    afterReductionBuilder->SetInsertPoint(afterReductionB->getTerminator());
  }

  for (int envID : environment->getEnvIDsOfLiveOutVars()) {
    auto prod = environment->getProducer(envID);

    /*
     * If the environment variable isn't reduced, it is held in allocated memory
     * that needs to be loaded from in order to retrieve the value.
     */
    auto isReduced = envBuilder->hasVariableBeenReduced(envID);
    Value *envVar;
    if (isReduced) {
      envVar = envBuilder->getAccumulatedReducedEnvironmentVariable(envID);
    } else {
      envVar = afterReductionBuilder->CreateLoad(
          envBuilder->getEnvironmentVariable(envID),
          "noelle.environment_variable.live_out.reduction");
    }
    assert(envVar != nullptr);

    for (auto consumer : environment->consumersOf(prod)) {
      if (auto depPHI = dyn_cast<PHINode>(consumer)) {
        depPHI->addIncoming(envVar, this->exitPointOfParallelizedLoop);
        continue;
      }
      prod->print(errs() << "Producer of environment variable:\t");
      errs() << "\n";
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

void ParallelizationTechnique::addPredecessorAndSuccessorsBasicBlocksToTasks(
    LoopContent *loopContent,
    std::vector<Task *> taskStructs) {
  assert(this->tasks.size() == 0);

  /*
   * Fetch the loop structure.
   */
  auto loopStructure = loopContent->getLoopStructure();

  /*
   * Fetch the loop headers.
   */
  auto loopPreHeader = loopStructure->getPreHeader();

  /*
   * Fetch the loop function.
   */
  auto loopFunction = loopStructure->getFunction();

  /*
   * Setup original loop and task with functions and basic blocks for wiring
   */
  auto &cxt = loopFunction->getContext();
  this->entryPointOfParallelizedLoop =
      BasicBlock::Create(cxt, "", loopFunction);
  this->exitPointOfParallelizedLoop = BasicBlock::Create(cxt, "", loopFunction);

  for (auto i = 0; i < taskStructs.size(); ++i) {
    auto task = taskStructs[i];
    tasks.push_back(task);

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
     * Create one basic block per loop exit, mapping between originals and
     * clones, and branching from them to the function exit block
     */
    for (auto exitBB : loopStructure->getLoopExitBasicBlocks()) {
      auto newExitBB = task->addBasicBlockStub(exitBB);
      task->tagBasicBlockAsLastBlock(newExitBB);
      IRBuilder<> builder(newExitBB);
      builder.CreateBr(task->getExit());
    }
  }
}

void ParallelizationTechnique::cloneSequentialLoop(LoopContent *loopContent,
                                                   int taskIndex) {
  assert(loopContent != nullptr);
  assert(taskIndex < this->tasks.size());

  /*
   * Fetch the task.
   */
  auto task = this->tasks.at(taskIndex);

  /*
   * Code to filter out instructions we don't want to clone.
   */
  auto filter = [](Instruction *inst) -> bool {
    if (auto call = dyn_cast<CallInst>(inst)) {
      if (call->isLifetimeStartOrEnd()) {
        return false;
      }
    }

    return true;
  };

  /*
   * Clone all basic blocks of the original loop
   */
  auto topLoop = loopContent->getLoopStructure();
  task->cloneAndAddBasicBlocks(topLoop->getBasicBlocks(), filter);
}

void ParallelizationTechnique::cloneSequentialLoopSubset(
    LoopContent *loopContent,
    int taskIndex,
    std::set<Instruction *> subset) {

  /*
   * Fetch the task.
   */
  auto task = tasks.at(taskIndex);

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

void ParallelizationTechnique::cloneMemoryLocationsLocallyAndRewireLoop(
    LoopContent *loopContent,
    int taskIndex) {

  /*
   * Fetch the task.
   */
  auto task = this->tasks.at(taskIndex);
  assert(task != nullptr);

  /*
   * Fetch the user associated to the task.
   */
  auto userID = this->fromTaskIDToUserID.at(task->getID());
  auto envUser = this->envBuilder->getUser(userID);

  /*
   * Fetch loop-specific abstractions.
   */
  auto rootLoop = loopContent->getLoopStructure();
  auto memoryCloningAnalysis = loopContent->getMemoryCloningAnalysis();

  /*
   * Fetch the environment of the loop
   */
  auto environment = loopContent->getEnvironment();
  assert(environment != nullptr);

  /*
   * Fetch the types manager.
   */
  auto typesManager = this->noelle.getTypesManager();

  /*
   * Check every stack object that can be safely cloned.
   */
  for (auto location : memoryCloningAnalysis->getClonableMemoryObjects()) {

    /*
     * Fetch the stack object.
     */
    auto alloca = location->getAllocation();

    /*
     * Check if this is an allocation used by this task
     */
    auto loopInstructionsRequiringClonedOperands =
        location->getLoopInstructionsUsingLocation();
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
     * The stack object can be safely cloned (thanks to the object-cloning
     * analysis) and it is used by our loop.
     *
     * First, we need to remove the alloca instruction to be a live-in if the
     * stack object doesn't need to be initialized.
     */
    if (!location->doPrivateCopiesNeedToBeInitialized()) {
      task->removeLiveIn(alloca);
    }

    /*
     * Now we need to traverse operands of loop instructions to clone
     * all live-in references (casts and GEPs) of the allocation to clone
     * State all cloned instructions in the task's instruction map for data flow
     * adjustment later
     */
    auto &entryBlock = (*task->getTaskBody()->begin());
    auto firstInstruction = &*entryBlock.begin();
    IRBuilder<> entryBuilder(&entryBlock);
    std::queue<Instruction *> instructionsToConvertOperandsOf;
    for (auto I : taskInstructions) {
      instructionsToConvertOperandsOf.push(I);
    }
    auto locationOutsideUses =
        location->getInstructionsUsingLocationOutsideLoop();
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
         * FIXME: Checking task's instruction map would be mis-leading, as
         * live-in values could be listed as clones to these values. Find a way
         * to ensure that wouldn't happen
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
         * then gets set to itself. This ensures that any operand using it that
         * has already been traversed will come after
         */
        auto opCloneI = opI->clone();
        entryBuilder.Insert(opCloneI);
        entryBuilder.SetInsertPoint(opCloneI);
        instructionsToConvertOperandsOf.push(opI);

        /*
         * Swap the operand's live in mapping with this clone so the live-in
         * allocation from the caller of the dispatcher is NOT used; instead, we
         * want the cloned allocation to be used
         *
         * NOTE: Add the instruction to the loop instruction map for data flow
         * adjusting to re-wire operands correctly
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
          if (dyn_cast<Constant>(opJ)) {

            /*
             * The current operand is a constant.
             * There is no need for a live-in.
             */
            continue;
          }

          /*
           * Check if the current operand is the alloca instruction that will be
           * cloned.
           */
          if (opJ == alloca) {
            continue;
          }

          /*
           * Check if the current operand requires to become a live-in.
           */
          auto newLiveIn = true;
          for (auto envID : envUser->getEnvIDsOfLiveInVars()) {
            auto producer = environment->getProducer(envID);
            if (producer == opJ) {
              newLiveIn = false;
              break;
            }
          }
          if (!newLiveIn) {
            continue;
          }

          /*
           * The current operand must become a new live-in.
           *
           * Make space in the environment for the new live-in.
           */
          auto newLiveInEnvironmentID =
              environment->addLiveInValue(opJ, { opI });
          this->envBuilder->addVariableToEnvironment(newLiveInEnvironmentID,
                                                     opJ->getType());

          /*
           * Declare the new live-in of the loop is also a new live-in for the
           * user (i.e., task) of the environment specified bt the input (i.e.,
           * taskIndex).
           */
          envUser->addLiveIn(newLiveInEnvironmentID);

          /*
           * Add the load inside the task to load from the environment the new
           * live-in.
           */
          auto envVarPtr =
              envUser->createEnvironmentVariablePointer(entryBuilder,
                                                        newLiveInEnvironmentID,
                                                        opJ->getType());
          auto environmentLocationLoad =
              entryBuilder.CreateLoad(envVarPtr,
                                      "noelle.environment_variable.live_in");

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
    auto allocaClone = cast<AllocaInst>(alloca->clone());
    auto firstInst = &*entryBlock.begin();
    entryBuilder.SetInsertPoint(firstInst);
    entryBuilder.Insert(allocaClone);

    /*
     * Initialize the private copy
     */
    if (location->doPrivateCopiesNeedToBeInitialized()) {

      /*
       * Fetch the pointer to the stack object that is passed as live-in.
       */
      Instruction *ptrToOriginalObjectInTask = alloca;
      if (!task->isAnOriginalLiveIn(alloca)) {
        ptrToOriginalObjectInTask = nullptr;
        for (auto ptr : location->getPointersUsedToAccessObject()) {
          if (task->isAnOriginalLiveIn(ptr) && isa<BitCastInst>(ptr)) {
            ptrToOriginalObjectInTask = ptr;
            break;
          }
        }
      }
      if (ptrToOriginalObjectInTask == nullptr) {
        ptrToOriginalObjectInTask = alloca;

        /*
         * We must add a new live-in that is the pointer to the original stack
         * object.
         */
        auto newLiveInEnvironmentID = environment->addLiveInValue(alloca, {});
        this->envBuilder->addVariableToEnvironment(newLiveInEnvironmentID,
                                                   alloca->getType());

        /*
         * Declare the new live-in of the loop is also a new live-in for the
         * user (i.e., task) of the environment specified bt the input (i.e.,
         * taskIndex).
         */
        envUser->addLiveIn(newLiveInEnvironmentID);

        /*
         * Add the load inside the task to load from the environment the new
         * live-in.
         */
        IRBuilder<> entryBuilderAtTheEnd(&entryBlock);
        auto lastInstruction = &*(--entryBlock.end());
        entryBuilderAtTheEnd.SetInsertPoint(lastInstruction);
        auto envVarPtr =
            envUser->createEnvironmentVariablePointer(entryBuilderAtTheEnd,
                                                      newLiveInEnvironmentID,
                                                      alloca->getType());
        auto environmentLocationLoad = entryBuilderAtTheEnd.CreateLoad(
            envVarPtr,
            "noelle.environment_variable.live_in");

        /*
         * Make the task aware that the new load represents the live-in value.
         */
        task->addLiveIn(alloca, environmentLocationLoad);
      }
      assert(ptrToOriginalObjectInTask != nullptr);
      assert(task->isAnOriginalLiveIn(ptrToOriginalObjectInTask));

      /*
       * Fetch the original stack object.
       */
      auto ptrOfOriginalStackObject = cast<Instruction>(
          task->getCloneOfOriginalLiveIn(ptrToOriginalObjectInTask));
      assert(ptrOfOriginalStackObject != nullptr);

      /*
       * Initialize the private copy of the stack object.
       */
      auto t = allocaClone->getAllocatedType();
      auto beforePtrOfOriginalStackObject =
          ptrOfOriginalStackObject->getPrevNode();
      entryBuilder.SetInsertPoint(ptrOfOriginalStackObject);
      auto &DL = allocaClone->getFunction()->getParent()->getDataLayout();
      auto sizeInBits = alloca->getAllocationSizeInBits(DL);
      uint64_t bytes = 0;
      if (sizeInBits.hasValue()) {
        bytes = sizeInBits.getValue() / 8;
      } else {
        bytes = typesManager->getSizeOfType(t);
      }
      auto allocaCloneCasted = cast<Instruction>(
          entryBuilder.CreateBitCast(allocaClone,
                                     ptrOfOriginalStackObject->getType()));
      auto initInst = entryBuilder.CreateMemCpy(allocaCloneCasted,
                                                {},
                                                ptrOfOriginalStackObject,
                                                {},
                                                bytes);
      ptrOfOriginalStackObject->moveAfter(beforePtrOfOriginalStackObject);
      allocaCloneCasted->moveAfter(allocaClone);

      /*
       * Register the task-private copy of @alloca as the clone of the live-in
       * @alloca.
       */
      task->addLiveIn(ptrToOriginalObjectInTask, allocaCloneCasted);
    }

    /*
     * Keep track of the original-clone mapping.
     */
    task->addInstruction(alloca, allocaClone);
  }
}

void ParallelizationTechnique::generateCodeToLoadLiveInVariables(
    LoopContent *loopContent,
    int taskIndex) {

  /*
   * Fetch the task.
   */
  auto task = this->tasks.at(taskIndex);

  /*
   * Fetch the user associated to the task.
   */
  auto userID = this->fromTaskIDToUserID.at(task->getID());
  auto envUser = this->envBuilder->getUser(userID);

  /*
   * Fetch the environment of the loop.
   */
  auto env = loopContent->getEnvironment();

  /*
   * Generate the loads to load values from the live-in environment variables.
   */
  errs() << "GenerateCodeToLoadLiveInVariables: Start\n";
  IRBuilder<> builder(task->getEntry());
  for (auto envID : envUser->getEnvIDsOfLiveInVars()) {

    /*
     * Fetch the current producer of the original code that generates the
     * live-in value.
     */
    auto producer = env->getProducer(envID);

    /*
     * Create GEP access of the environment variable at the given index
     */
    auto envPointer =
        envUser->createEnvironmentVariablePointer(builder,
                                                  envID,
                                                  producer->getType());

    /*
     * Load the live-in value from the environment pointer.
     */
    errs() << "GenerateCodeToLoadLiveInVariables:   Environment ID = " << envID
           << "\n";
    errs() << "GenerateCodeToLoadLiveInVariables:     Pointer = " << *envPointer
           << "\n";
    auto metaString = std::string{ "noelle_environment_variable_" };
    metaString.append(std::to_string(envID));
    auto envLoad = builder.CreateLoad(envPointer, metaString);

    /*
     * Register the load as a "clone" of the original producer
     */
    task->addLiveIn(producer, envLoad);
  }
  errs() << "GenerateCodeToLoadLiveInVariables: Exit\n";
}

void ParallelizationTechnique::generateCodeToStoreLiveOutVariables(
    LoopContent *loopContent,
    int taskIndex) {

  /*
   * Fetch the metadata manager
   */
  auto mm = this->noelle.getMetadataManager();

  /*
   * Fetch the environment of the loop
   */
  auto env = loopContent->getEnvironment();

  /*
   * Fetch the requested task.
   */
  auto task = this->tasks.at(taskIndex);
  assert(task != nullptr);

  /*
   * Create a builder that points to the entry point of the function executed by
   * the task.
   */
  auto entryBlock = task->getEntry();
  assert(entryBlock != nullptr);
  auto entryTerminator = entryBlock->getTerminator();
  assert(entryTerminator != nullptr);
  IRBuilder<> entryBuilder(entryTerminator);

  /*
   * Fetch the function executed by the task.
   */
  auto &taskFunction = *task->getTaskBody();

  /*
   * Fetch the CFG analysis
   */
  auto cfgAnalysis = this->noelle.getCFGAnalysis();

  /*
   * Fetch the loop SCCDAG
   */
  auto sccManager = loopContent->getSCCManager();
  auto loopSCCDAG = sccManager->getSCCDAG();

  /*
   * Fetch the user associated to the task.
   */
  auto userID = this->fromTaskIDToUserID.at(task->getID());
  auto envUser = this->envBuilder->getUser(userID);

  /*
   * Iterate over live-out variables and inject stores at the end of the
   * execution of the function of the task to propagate the new live-out values
   * back to the caller of the parallelized loop.
   */
  for (auto envID : envUser->getEnvIDsOfLiveOutVars()) {

    /*
     * Fetch the producer of the current live-out variable.
     * Fetch the clones of the producer. If none are specified in the
     * one-to-many mapping, assume the direct cloning of the producer is the
     * only clone
     * TODO: Find a better place to map this single clone (perhaps when the
     * original loop's values are cloned)
     */
    auto producer = cast<Instruction>(env->getProducer(envID));
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
    auto isReduced = this->envBuilder->hasVariableBeenReduced(envID);
    if (isReduced) {
      envUser->createReducableEnvPtr(entryBuilder,
                                     envID,
                                     envType,
                                     numTaskInstances,
                                     task->getTaskInstanceID());
    } else {
      envUser->createEnvironmentVariablePointer(entryBuilder, envID, envType);
    }
    auto envPtr = envUser->getEnvPtr(envID);

    /*
     * If the variable is reducable, store the identity value as the initial
     * value before the parallelized loop starts its execution.
     */
    if (isReduced) {

      /*
       * Fetch the reduction
       */
      auto producerSCC = loopSCCDAG->sccOfValue(producer);
      auto reductionVariable =
          cast<ReductionSCC>(sccManager->getSCCAttrs(producerSCC));
      assert(reductionVariable != nullptr);

      /*
       * Fetch the operator of the accumulator instruction for this reducable
       * variable Store the identity value of the operator
       */
      auto identityV = reductionVariable->getIdentityValue();
      auto newStore = entryBuilder.CreateStore(identityV, envPtr);

      /*
       * Attach the metadata to the new store
       */
      mm->addMetadata(
          newStore,
          "noelle.environment_variable.live_out.reducable.initialize_private_copy",
          std::to_string(envID));
    }

    /*
     * Inject store instructions to propagate live-out values back to the caller
     * of the parallelized loop.
     *
     * NOTE: To support storing live outs at exit blocks and not directly where
     * the producer is executed, produce a PHI node at each store point with the
     * following incoming values: the last executed intermediate of the producer
     * that is post-dominated by that incoming block. There should only be one
     * such value assuming that store point is correctly chosen
     *
     * NOTE: This provides flexibility to parallelization schemes with modified
     * prologues or latches that have reducible live outs. Furthermore, this
     * flexibility is ONLY permitted for reducible or IV live outs as other live
     * outs can never store intermediate values of the producer.
     */
    for (auto producerClone : producerClones) {

      /*
       * Compute the dominators.
       */
      auto taskDS = this->noelle.getDominators(&taskFunction);

      /*
       * Fetch all points in the CFG where we need to insert the store
       * instruction.
       */
      auto insertBBs =
          this->determineLatestPointsToInsertLiveOutStore(loopContent,
                                                          taskIndex,
                                                          producerClone,
                                                          isReduced,
                                                          *taskDS);
      for (auto BB : insertBBs) {

        /*
         * Fetch the value we need to store.
         */
        auto producerValueToStore =
            isReduced
                ? this->fetchOrCreatePHIForIntermediateProducerValueOfReducibleLiveOutVariable(
                    loopContent,
                    taskIndex,
                    envID,
                    BB,
                    *taskDS)
                : producerClone;

        /*
         * Generate the store instruction to store the value to the live-out
         * variable, which is allocated on the stack of the caller.
         */
        IRBuilder<> liveOutBuilder(BB);
        auto store = liveOutBuilder.CreateStore(producerValueToStore, envPtr);
        store->removeFromParent();

        /*
         * Decide the condition for which the store needs to be executed.
         *
         * If the live-out variable is reduced, then the store needs to always
         * execute. This is because threads have their own private copy.
         *
         * If the live-out variable is not reduced, then the store needs to be
         * executed only by the thread that has executed the last iteration.
         */
        if (isReduced) {

          /*
           * The live-out variable is reduced.
           */
          store->insertBefore(BB->getTerminator());

          /*
           * Attach the metadata to the new store
           */
          mm->addMetadata(
              store,
              "noelle.environment_variable.live_out.reducable.update_private_copy",
              std::to_string(envID));

        } else {

          /*
           * The live-out variable is not reduced.
           *
           * Attach the metadata to the new store
           */
          mm->addMetadata(store,
                          "noelle.environment_variable.live_out.store",
                          std::to_string(envID));

          /*
           * Check if the place to inject the store is included in a cycle in
           * the CFG (hence, it can run multiple times). If that is not the
           * case, then we need to store the live-out variable only if the
           * current task has executed the last iteration of the loop.
           *
           * If instead the store is included in a cycle, then the
           * store will happen within the loop body and we assume to be
           * synchronized correctly by the parallelization technique.
           */
          if (!cfgAnalysis.isIncludedInACycle(*BB)) {

            /*
             * The place to insert the store is the last basic block executed
             * before leaving the task.
             */
            auto lastIterationBB =
                this->getBasicBlockExecutedOnlyByLastIterationBeforeExitingTask(
                    loopContent,
                    taskIndex,
                    *BB);
            assert(lastIterationBB != nullptr);
            IRBuilder<> b{ lastIterationBB };
            auto lastIterationBBTerminator = lastIterationBB->getTerminator();
            if (lastIterationBBTerminator != nullptr) {
              store->insertBefore(lastIterationBBTerminator);
            } else {
              b.Insert(store);
            }

          } else {
            store->insertBefore(BB->getTerminator());
          }
        }
      }

      /*
       * Free the memory
       */
      delete taskDS;
    }
  }
}

std::set<BasicBlock *> ParallelizationTechnique::
    determineLatestPointsToInsertLiveOutStore(LoopContent *loopContent,
                                              int taskIndex,
                                              Instruction *liveOut,
                                              bool isReduced,
                                              DominatorSummary &taskDS) {
  auto task = this->tasks[taskIndex];

  /*
   * Fetch the header.
   */
  auto loopSummary = loopContent->getLoopStructure();
  auto liveOutBlock = liveOut->getParent();

  /*
   * Insert stores in loop exit blocks
   * If the live out is reducible, it is fine that the live out value does not
   * dominate the exit as some other intermediate is guaranteed to.
   */
  std::set<BasicBlock *> insertPoints{};
  for (auto BB : loopSummary->getLoopExitBasicBlocks()) {
    auto cloneBB = task->getCloneOfOriginalBasicBlock(BB);
    auto liveOutDominatesExit = taskDS.DT.dominates(liveOutBlock, cloneBB);
    if (!isReduced && !liveOutDominatesExit)
      continue;
    insertPoints.insert(cloneBB);
  }

  /*
   * If the parallelization scheme introduced other loop exiting blocks,
   * and this live out is reducible, we must store the latest intermediate value
   * for them
   */
  if (isReduced) {
    for (auto predecessor : predecessors(task->getExit())) {
      if (predecessor == task->getEntry())
        continue;
      insertPoints.insert(predecessor);
    }
  }

  /*
   * HACK: If no exit block is dominated by the live out, the scheme is doing
   * short-circuiting logic of some sort on the loop's execution. State the live
   * out's block itself as a safe-guard.
   * TODO: Provide a way for each scheme to provide such an override instead of
   * this blanket catch
   */
  if (insertPoints.empty()) {
    insertPoints.insert(liveOut->getParent());
  }

  return insertPoints;
}

Instruction *ParallelizationTechnique::
    fetchOrCreatePHIForIntermediateProducerValueOfReducibleLiveOutVariable(
        LoopContent *loopContent,
        int taskIndex,
        int envID,
        BasicBlock *insertBasicBlock,
        DominatorSummary &taskDS) {

  /*
   * Fetch the SCC manager.
   */
  auto sccManager = loopContent->getSCCManager();

  /*
   * Fetch the task.
   */
  auto task = this->tasks[taskIndex];

  /*
   * Fetch all clones of intermediate values of the producer
   */
  auto producer =
      (Instruction *)loopContent->getEnvironment()->getProducer(envID);
  auto producerSCC = sccManager->getSCCDAG()->sccOfValue(producer);

  std::set<Instruction *> intermediateValues{};
  for (auto originalPHI : sccManager->getSCCAttrs(producerSCC)->getPHIs()) {
    intermediateValues.insert(task->getCloneOfOriginalInstruction(originalPHI));
  }
  auto f = [&intermediateValues, task](Instruction *i) -> bool {
    if (isa<LoadInst>(i)) {
      return false;
    }
    if (isa<StoreInst>(i)) {
      return false;
    }
    if (isa<CallBase>(i)) {
      return false;
    }
    intermediateValues.insert(task->getCloneOfOriginalInstruction(i));
    return false;
  };
  producerSCC->iterateOverInstructions(f);

  /*
   * If in the insert block there already exists a single intermediate,
   * return that intermediate
   */
  Instruction *lastIntermediateAtInsertBlock = nullptr;
  for (auto intermediateValue : intermediateValues) {
    if (intermediateValue->getParent() != insertBasicBlock)
      continue;
    if (lastIntermediateAtInsertBlock
        && taskDS.DT.dominates(intermediateValue,
                               lastIntermediateAtInsertBlock))
      continue;
    lastIntermediateAtInsertBlock = intermediateValue;
  }
  if (lastIntermediateAtInsertBlock)
    return lastIntermediateAtInsertBlock;

  /*
   * Produce PHI at the insert point
   */
  IRBuilder<> builder(insertBasicBlock->getFirstNonPHIOrDbgOrLifetime());
  auto producerType = producer->getType();
  auto phiNode = builder.CreatePHI(producerType, pred_size(insertBasicBlock));

  /*
   * Fetch all PHI node basic block predecessors
   * Determine all intermediate values dominating each predecessor
   * Determine the intermediate value of this set that dominates no other
   * intermediates in the set
   */
  for (auto predIter = pred_begin(insertBasicBlock);
       predIter != pred_end(insertBasicBlock);
       ++predIter) {
    auto predecessor = *predIter;

    auto dominatingValues =
        taskDS.DT.getDominatorsOf(intermediateValues, predecessor);
    assert(
        dominatingValues.size() > 0
        && "Cannot store reducible live out where no producer value dominates the point");

    auto lastDominatingValues =
        taskDS.DT.getInstructionsThatDoNotDominateAnyOther(dominatingValues);
    assert(
        lastDominatingValues.size() == 1
        && "Cannot store reducible live out where no last produced value is known");
    auto lastDominatingIntermediateValue = *lastDominatingValues.begin();

    auto predecessorTerminator = predecessor->getTerminator();
    IRBuilder<> builderAtValue(predecessorTerminator);

    auto correctlyTypedValue =
        this->castToCorrectReducibleType(builderAtValue,
                                         lastDominatingIntermediateValue,
                                         producer->getType());
    phiNode->addIncoming(correctlyTypedValue, predecessor);
  }

  return phiNode;
}

Value *ParallelizationTechnique::castToCorrectReducibleType(
    IRBuilder<> &builder,
    Value *value,
    Type *targetType) {
  auto valueTy = value->getType();
  if (valueTy == targetType)
    return value;

  if (valueTy->isIntegerTy() && targetType->isIntegerTy()) {
    return builder.CreateBitCast(value, targetType);
  } else if (valueTy->isIntegerTy() && targetType->isFloatingPointTy()) {
    return builder.CreateSIToFP(value, targetType);
  } else if (valueTy->isFloatingPointTy() && targetType->isIntegerTy()) {
    return builder.CreateFPToSI(value, targetType);
  } else if (valueTy->isFloatingPointTy() && targetType->isFloatingPointTy()) {
    return builder.CreateFPCast(value, targetType);
  } else
    assert(false && "Cannot cast to non-reducible type");

  return nullptr;
}

void ParallelizationTechnique::setReducableVariablesToBeginAtIdentityValue(
    LoopContent *loopContent,
    int taskIndex) {

  /*
   * Fetch the task.
   */
  assert(taskIndex < this->tasks.size());
  auto task = this->tasks[taskIndex];
  assert(task != nullptr);

  /*
   * Fetch task information.
   */
  auto loopStructure = loopContent->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();
  auto headerClone = task->getCloneOfOriginalBasicBlock(loopHeader);
  assert(headerClone != nullptr);
  auto loopPreHeader = loopStructure->getPreHeader();
  auto preheaderClone = task->getCloneOfOriginalBasicBlock(loopPreHeader);
  assert(preheaderClone != nullptr);

  /*
   * Fetch the environment of the loop
   */
  auto environment = loopContent->getEnvironment();
  assert(environment != nullptr);

  /*
   * Fetch the SCCDAG.
   */
  auto sccManager = loopContent->getSCCManager();
  auto sccdag = sccManager->getSCCDAG();

  /*
   * Iterate over live-out variables.
   */
  for (auto envID : environment->getEnvIDsOfLiveOutVars()) {

    /*
     * Check if the current live-out variable can be reduced.
     */
    auto isThisLiveOutVarReducable =
        this->envBuilder->hasVariableBeenReduced(envID);
    if (!isThisLiveOutVarReducable) {
      continue;
    }

    /*
     * Fetch the instruction that produces the live-out variable.
     * The reducible live out must be contained within an SCC that has a
     * PHI node in the header. The incoming value from the preheader is the
     * location of the initial value that needs to be changed
     */
    auto producer = environment->getProducer(envID);
    assert(producer != nullptr);
    auto producerSCC = sccdag->sccOfValue(producer);
    auto reductionVar =
        cast<ReductionSCC>(sccManager->getSCCAttrs(producerSCC));
    auto loopEntryProducerPHI =
        reductionVar->getPhiThatAccumulatesValuesBetweenLoopIterations();
    assert(loopEntryProducerPHI != nullptr);

    /*
     * Fetch the related instruction of the producer that has been created
     * (cloned) and stored in the parallelized version of the loop.
     */
    auto producerClone = cast<PHINode>(
        task->getCloneOfOriginalInstruction(loopEntryProducerPHI));

    /*
     * Fetch the cloned pre-header index
     */
    auto incomingIndex = producerClone->getBasicBlockIndex(preheaderClone);
    assert(incomingIndex != -1 && "Loop entry present on producer PHI node");

    /*
     * Fetch the identity constant for the operation reduced.
     * For example, if the variable reduced is an accumulator where "+" is used
     * to accumulate values, then "0" is the identity.
     */
    auto identityV = reductionVar->getIdentityValue();

    /*
     * Set the initial value for the private variable.
     */
    producerClone->setIncomingValue(incomingIndex, identityV);
  }
}

void ParallelizationTechnique::generateCodeToStoreExitBlockIndex(
    LoopContent *loopContent,
    int taskIndex) {

  /*
   * Fetch the NOELLE's managers
   */
  auto mm = this->noelle.getMetadataManager();
  auto tm = this->noelle.getTypesManager();

  /*
   * Fetch the program.
   */
  auto program = this->noelle.getProgram();

  /*
   * Check whether there are multiple exit blocks or not.
   * If there are more exit blocks, then we need to specify which one has been
   * taken.
   */
  auto task = this->tasks.at(taskIndex);
  if (task->getNumberOfLastBlocks() == 1) {
    return;
  }

  /*
   * Fetch the environment of the loop
   */
  auto environment = loopContent->getEnvironment();
  assert(environment != nullptr);

  /*
   * Fetch the user associated to the task.
   */
  auto userID = this->fromTaskIDToUserID.at(task->getID());
  auto envUser = this->envBuilder->getUser(userID);

  /*
   * There are multiple exit blocks.
   *
   * Fetch the pointer of the location where the exit block ID taken will be
   * stored.
   */
  auto exitBlockID = environment->getExitBlockID();
  assert(exitBlockID != -1);
  auto entryTerminator = task->getEntry()->getTerminator();
  IRBuilder<> entryBuilder(entryTerminator);

  auto envType = environment->typeOfEnvironmentLocation(exitBlockID);
  envUser->createEnvironmentVariablePointer(entryBuilder, exitBlockID, envType);

  /*
   * Add a store instruction to specify to the code outside the parallelized
   * loop which exit block is taken.
   */
  auto int32 = tm->getIntegerType(32);
  for (int i = 0; i < task->getNumberOfLastBlocks(); ++i) {
    auto bb = task->getLastBlock(i);
    auto term = bb->getTerminator();

    /*
     * Create the store instruction
     */
    IRBuilder<> builder(bb);
    auto envPtr = envUser->getEnvPtr(exitBlockID);
    auto newStore = builder.CreateStore(ConstantInt::get(int32, i), envPtr);

    /*
     * Attach the metadata to the new store
     */
    mm->addMetadata(newStore, "noelle.exit_block", std::to_string(i));

    term->removeFromParent();
    builder.Insert(term);
  }
}

void ParallelizationTechnique::doNestedInlineOfCalls(
    Function *function,
    std::set<CallInst *> &calls) {
  std::queue<CallInst *> callsToInline;
  for (auto call : calls)
    callsToInline.push(call);

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
            if (func == nullptr || func->empty())
              continue;
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
          if (funcToInline.find(call->getCalledFunction())
              != funcToInline.end()) {
            callsToInline.push(call);
          }
        }
      }
    }
  }
}

std::unordered_map<InductionVariable *, Value *> ParallelizationTechnique::
    cloneIVStepValueComputation(LoopContent *loopContent,
                                int taskIndex,
                                IRBuilder<> &insertBlock) {

  /*
   * Fetch the task
   */
  assert(taskIndex < tasks.size());
  auto task = tasks.at(taskIndex);
  assert(task != nullptr);

  /*
   * Fetch the information about the loop
   */
  auto loopSummary = loopContent->getLoopStructure();
  auto allIVInfo = loopContent->getInductionVariableManager();

  /*
   * Clone each IV's step value described by the InductionVariable class
   */
  std::unordered_map<InductionVariable *, Value *> clonedStepSizeMap;
  for (auto ivInfo : allIVInfo->getInductionVariables(*loopSummary)) {

    /*
     * If the step value is constant or a value present in the original loop,
     * use its clone
     * TODO: Refactor this logic as a helper: tryAndFetchClone that doesn't
     * assert a clone must exist
     */
    auto singleComputedStepValue = ivInfo->getSingleComputedStepValue();
    if (singleComputedStepValue) {
      Value *clonedStepValue = nullptr;
      if (isa<ConstantData>(singleComputedStepValue)) {
        clonedStepValue = singleComputedStepValue;

      } else if (task->isAnOriginalLiveIn(singleComputedStepValue)) {
        clonedStepValue =
            task->getCloneOfOriginalLiveIn(singleComputedStepValue);

      } else if (auto singleComputedStepInst =
                     dyn_cast<Instruction>(singleComputedStepValue)) {
        clonedStepValue =
            task->getCloneOfOriginalInstruction(singleComputedStepInst);
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
      task->adjustDataAndControlFlowToUseClones(
          task->getCloneOfOriginalInstruction(expandedInst));
    }
    auto clonedStepValue =
        task->getCloneOfOriginalInstruction(expandedInsts.back());
    clonedStepSizeMap.insert(std::make_pair(ivInfo, clonedStepValue));
  }

  this->adjustStepValueOfPointerTypeIVToReflectPointerArithmetic(
      clonedStepSizeMap,
      insertBlock);

  return clonedStepSizeMap;
}

void ParallelizationTechnique::
    adjustStepValueOfPointerTypeIVToReflectPointerArithmetic(
        std::unordered_map<InductionVariable *, Value *> clonedStepValueMap,
        IRBuilder<> &insertBlock) {

  /*
   * Fetch the program.
   */
  auto program = this->noelle.getProgram();

  /*
   * If the IV's type is pointer, then the SCEV of the step value for the IV is
   * pointer arithmetic and needs to be multiplied by the bit size of pointers
   * to reflect the exact change of the value
   *
   * This occurs because GEP information is lost to ScalarEvolution analysis
   * when it computes the step value as a SCEV
   */
  auto &DL = program->getDataLayout();
  auto ptrSizeInBytes = DL.getPointerSize();
  for (auto ivAndStepValuePair : clonedStepValueMap) {
    auto iv = ivAndStepValuePair.first;
    auto value = ivAndStepValuePair.second;

    auto loopEntryPHI = iv->getLoopEntryPHI();
    if (!loopEntryPHI->getType()->isPointerTy())
      continue;

    auto ptrSizeValue =
        ConstantInt::get(value->getType(), ptrSizeInBytes, false);
    auto adjustedStepValue = insertBlock.CreateMul(value, ptrSizeValue);
    clonedStepValueMap[iv] = adjustedStepValue;
  }
}

float ParallelizationTechnique::computeSequentialFractionOfExecution(
    LoopContent *loopContent) const {
  auto f = [](GenericSCC *sccInfo) -> bool {
    auto mustBeSynchronized = isa<LoopCarriedUnknownSCC>(sccInfo);
    return mustBeSynchronized;
  };

  auto fraction = this->computeSequentialFractionOfExecution(loopContent, f);

  return fraction;
}

float ParallelizationTechnique::computeSequentialFractionOfExecution(
    LoopContent *loopContent,
    std::function<bool(GenericSCC *scc)> doesItRunSequentially) const {

  /*
   * Fetch the SCCDAG.
   */
  auto sccManager = loopContent->getSCCManager();
  auto sccdag = sccManager->getSCCDAG();

  /*
   * Compute the fraction of sequential code.
   */
  float totalInstructionCount = 0, sequentialInstructionCount = 0;
  for (auto sccNode : sccdag->getNodes()) {
    auto scc = sccNode->getT();
    auto sccInfo = sccManager->getSCCAttrs(scc);

    auto numInstructionsInSCC = scc->numInternalNodes();
    totalInstructionCount += numInstructionsInSCC;
    if (doesItRunSequentially(sccInfo)) {
      sequentialInstructionCount += numInstructionsInSCC;
    }
  }

  return sequentialInstructionCount / totalInstructionCount;
}

void ParallelizationTechnique::makePRVGsReentrant(void) {

  /*
   * Fetch the reentrant version of the known PRVGs.
   */
  std::map<Function *, Function *> prvgs;
  auto fm = this->noelle.getFunctionsManager();
  auto rand = fm->getFunction("rand");
  auto rand_r = fm->getFunction("rand_r");
  if ((rand != nullptr) && (rand_r != nullptr)) {
    prvgs[rand] = rand_r;
  }

  /*
   * Substitute PRVGs.
   */
  auto tm = this->noelle.getTypesManager();
  for (auto i = 0; i < this->tasks.size(); ++i) {

    /*
     * Fetch the task.
     */
    auto task = this->tasks[i];

    /*
     * Iterate over the instructions of the task.
     */
    auto f = task->getTaskBody();

    /*
     * Fetch the entry basic block.
     */
    auto &entryBB = f->getEntryBlock();
    IRBuilder<> entryBuilder(&entryBB);
    entryBuilder.SetInsertPoint(entryBB.getFirstNonPHIOrDbgOrLifetime());

    /*
     * Find the PRVG to substitute.
     */
    std::set<CallBase *> toPatch;
    for (auto &I : instructions(f)) {

      /*
       * Fetch the next call instruction.
       */
      auto callI = dyn_cast<CallBase>(&I);
      if (callI == nullptr) {
        continue;
      }

      /*
       * Fetch the next call instruction of a PRVG.
       */
      auto calleeF = callI->getCalledFunction();
      if (calleeF == nullptr) {
        continue;
      }
      auto reentrantPRVG = prvgs[calleeF];
      if (reentrantPRVG == nullptr) {
        continue;
      }

      /*
       * Collect the current place to patch.
       */
      toPatch.insert(callI);
    }

    /*
     * Substitute PRVGs.
     */
    for (auto callI : toPatch) {
      auto calleeF = callI->getCalledFunction();
      auto reentrantPRVG = prvgs[calleeF];
      assert(reentrantPRVG != nullptr);

      /*
       * Allocate the PRVG reentrant state.
       */
      auto seedI = entryBuilder.CreateAlloca(tm->getIntegerType(32));
      IRBuilder<> prvgBuilder(callI);
      auto newCallI = prvgBuilder.CreateCall(reentrantPRVG, { seedI });
      callI->replaceAllUsesWith(newCallI);
      callI->eraseFromParent();
    }
  }

  return;
}

Value *ParallelizationTechnique::fetchCloneInTask(Task *t, Value *original) {

  /*
   * Is it a constant?
   */
  if (isa<ConstantData>(original)) {
    return original;
  }

  /*
   * Is it a live-in?
   */
  if (t->isAnOriginalLiveIn(original)) {
    return t->getCloneOfOriginalLiveIn(original);
  }

  /*
   * This is a normal instruction.
   */
  assert(isa<Instruction>(original));
  auto iClone = t->getCloneOfOriginalInstruction(cast<Instruction>(original));
  assert(iClone != nullptr);

  return iClone;
}

BasicBlock *ParallelizationTechnique::getParLoopEntryPoint(void) const {
  return this->entryPointOfParallelizedLoop;
}

BasicBlock *ParallelizationTechnique::getParLoopExitPoint(void) const {
  return this->exitPointOfParallelizedLoop;
}

ParallelizationTechnique::~ParallelizationTechnique() {
  return;
}

} // namespace arcana::noelle
