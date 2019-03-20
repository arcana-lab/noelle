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

ParallelizationTechnique::ParallelizationTechnique (Module &module, Verbosity v)
  : module{module}, verbose{v}, tasks{}, envBuilder{0}
  {

  return ;
}

ParallelizationTechnique::~ParallelizationTechnique () {
  reset();
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
  this->envBuilder->createEnvVariables(varTypes, simpleVars, reducableVars, numTaskInstances);

  this->envBuilder->createEnvUsers(tasks.size());
  for (auto i = 0; i < tasks.size(); ++i) {
    auto task = tasks[i];
    auto envUser = envBuilder->getUser(i);
    IRBuilder<> entryBuilder(task->entryBlock);
    envUser->setEnvArray(entryBuilder.CreateBitCast(
      task->envArg,
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
  IRBuilder<> builder(&*LDI->function->begin()->begin());
  envBuilder->generateEnvArray(builder);
  envBuilder->generateEnvVariables(builder);
}

void ParallelizationTechnique::populateLiveInEnvironment (LoopDependenceInfo *LDI) {
  IRBuilder<> builder(this->entryPointOfParallelizedLoop);
  for (auto envIndex : LDI->environment->getEnvIndicesOfLiveInVars()) {
    builder.CreateStore(LDI->environment->producerAt(envIndex), envBuilder->getEnvVar(envIndex));
  }
}

void ParallelizationTechnique::propagateLiveOutEnvironment (LoopDependenceInfo *LDI) {
  IRBuilder<> builder(this->entryPointOfParallelizedLoop);
  for (int envInd : LDI->environment->getEnvIndicesOfLiveOutVars()) {
    auto prod = LDI->environment->producerAt(envInd);

    /*
     * NOTE(angelo): If the environment variable isn't reduced, it is held in allocated
     * memory that needs to be loaded from in order to retrieve the value
     */
    auto isReduced = envBuilder->isReduced(envInd);
    auto envVar = envBuilder->getEnvVar(envInd);
    if (!isReduced) envVar = builder.CreateLoad(envBuilder->getEnvVar(envInd));

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
   * Setup original loop and task with functions and basic blocks for wiring
   */
  auto &cxt = LDI->function->getContext();
  this->entryPointOfParallelizedLoop = BasicBlock::Create(cxt, "", LDI->function);
  this->exitPointOfParallelizedLoop = BasicBlock::Create(cxt, "", LDI->function);

  numTaskInstances = taskStructs.size();
  for (auto i = 0; i < numTaskInstances; ++i) {
    auto task = taskStructs[i];
    tasks.push_back(task);

    auto &cxt = module.getContext();
    task->order = i;
    task->F = cast<Function>(module.getOrInsertFunction("", taskType));
    task->extractFuncArgs();
    task->entryBlock = BasicBlock::Create(cxt, "", task->F);
    task->exitBlock = BasicBlock::Create(cxt, "", task->F);

    /*
     * Map original preheader to entry block
     */
    task->basicBlockClones[LDI->preHeader] = task->entryBlock;

    /*
     * Create one basic block per loop exit, mapping between originals and clones,
     * and branching from them to the function exit block
     */
    for (auto exitBB : LDI->loopExitBlocks) {
      auto newExitBB = BasicBlock::Create(cxt, "", task->F);
      task->basicBlockClones[exitBB] = newExitBB;
      task->loopExitBlocks.push_back(newExitBB);
      IRBuilder<> builder(newExitBB);
      builder.CreateBr(task->exitBlock);
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
  for (auto originBB : LDI->liSummary.topLoop->orderedBBs) {

    /*
     * Clone the basic block in the context of the original loop's function
     */
    auto cloneBB = BasicBlock::Create(cxt, "", task->F);
    task->basicBlockClones[originBB] = cloneBB;

    /*
     * Clone every instruction in the basic block, adding them in order to the clone
     */
    IRBuilder<> builder(cloneBB);
    for (auto &I : *originBB) {
      auto cloneI = builder.Insert(I.clone());
      task->instructionClones[&I] = cloneI;
    }
  }
}

void ParallelizationTechnique::cloneSequentialLoopSubset (
  LoopDependenceInfo *LDI,
  int taskIndex,
  std::set<Instruction *> subset
){
  auto &cxt = module.getContext();
  auto task = tasks[taskIndex];
  auto &iClones = task->instructionClones;

  /*
   * Clone a portion of the original loop (determined by a set of SCCs
   * Determine the set of basic blocks these instructions belong to
   */
  std::set<BasicBlock *> bbSubset;
  for (auto I : subset) {
    iClones[I] = I->clone();
    bbSubset.insert(I->getParent());
  }

  /*
   * Add cloned instructions to their respective cloned basic blocks
   */
  for (auto bb : bbSubset) {
    auto cloneBB = BasicBlock::Create(cxt, "", task->F);
    IRBuilder<> builder(cloneBB);
    for (auto &I : *bb) {
      if (iClones.find(&I) == iClones.end()) continue;
      builder.Insert(iClones[&I]);
    }
    task->basicBlockClones[bb] = cloneBB;
  }
}

void ParallelizationTechnique::generateCodeToLoadLiveInVariables (
  LoopDependenceInfo *LDI, 
  int taskIndex
){
  auto task = this->tasks[taskIndex];
  IRBuilder<> builder(task->entryBlock);
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
    task->liveInClones[producer] = cast<Instruction>(envLoad);
  }
}

void ParallelizationTechnique::generateCodeToStoreLiveOutVariables (
  LoopDependenceInfo *LDI, 
  int taskIndex
){
  auto task = this->tasks[taskIndex];
  IRBuilder<> entryBuilder(task->entryBlock);
  auto entryTerminator = task->entryBlock->getTerminator();
  auto envUser = this->envBuilder->getUser(taskIndex);
  for (auto envIndex : envUser->getEnvIndicesOfLiveOutVars()) {
    auto producer = (Instruction*)LDI->environment->producerAt(envIndex);
    auto envType = producer->getType();

    /*
     * Create GEP access of the single, or reducable, environment variable
     */
    auto isReduced = this->envBuilder->isReduced(envIndex);
    if (isReduced) {
      envUser->createReducableEnvPtr(entryBuilder, envIndex, envType, numTaskInstances, task->instanceIndexV);
    } else {
      envUser->createEnvPtr(entryBuilder, envIndex, envType);
    }
    auto envPtr = envUser->getEnvPtr(envIndex);

    /*
     * If the variable is reducable, store the identity as the initial value
     * NOTE(angelo): A limitation of our reducability analysis requires PHINode producers
     */
    if (isReduced) {
      assert(isa<PHINode>(producer));

      /*
       * Fetch the operator of the accumulator instruction for this reducable PHI node
       * Store the identity value of the operator
       */
      auto identityV = getIdentityValueForEnvironmentValue(LDI, taskIndex, envIndex);
      entryBuilder.CreateStore(identityV, envPtr);
    }

    /*
     * Store the clone of the producer at the environment pointer
     */
    auto prodClone = task->instructionClones[producer];
    auto insertBBs = determineLatestPointsToInsertLiveOutStore(LDI, taskIndex, producer);
    for (auto BB : insertBBs) {
      IRBuilder<> liveOutBuilder(BB);
      auto store = (StoreInst*)liveOutBuilder.CreateStore(prodClone, envPtr);
      store->removeFromParent();
      store->insertBefore(BB->getTerminator());
    }
  }

  if (entryTerminator->getNextNode()) {
    entryTerminator->removeFromParent();
    entryBuilder.Insert(entryTerminator);
  }
}

std::set<BasicBlock *> ParallelizationTechnique::determineLatestPointsToInsertLiveOutStore (
  LoopDependenceInfo *LDI,
  int taskIndex,
  Instruction *liveOut
){
  auto task = this->tasks[taskIndex];

  /*
   * Determine whether the producer is in the loop header. If so, return all
   * the loop's exit blocks, as the live out value must be valid at all exit points
   */
  auto liveOutClone = task->instructionClones[liveOut];
  auto isInHeader = LDI->header == liveOut->getParent();
  if (!isInHeader) return { liveOutClone->getParent() };

  std::set<BasicBlock *> insertPoints;
  for (auto BB : LDI->loopExitBlocks) {
    insertPoints.insert(task->basicBlockClones[BB]);
  }
  return insertPoints;
}

void ParallelizationTechnique::adjustDataFlowToUseClones (
  LoopDependenceInfo *LDI,
  int taskIndex
){
  auto &task = tasks[taskIndex];
  auto &bbClones = task->basicBlockClones;
  auto &iClones = task->instructionClones;
  auto &liveIns = task->liveInClones;

  for (auto pair : iClones) {
    auto cloneI = pair.second;

    /*
     * Adjust basic block references of terminators and PHI nodes
     */
    if (auto terminator = dyn_cast<TerminatorInst>(cloneI)) {
      for (int i = 0; i < terminator->getNumSuccessors(); ++i) {
        auto succBB = terminator->getSuccessor(i);
        if (succBB->getParent() == task->F) continue;
        assert(bbClones.find(succBB) != bbClones.end());
        terminator->setSuccessor(i, bbClones[succBB]);
      }
    }

    if (auto phi = dyn_cast<PHINode>(cloneI)) {
      for (int i = 0; i < phi->getNumIncomingValues(); ++i) {
        auto incomingBB = phi->getIncomingBlock(i);
        if (incomingBB->getParent() == task->F) continue;
        auto cloneBB = bbClones[incomingBB];
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
      if (liveIns.find(opV) != liveIns.end()) {
        op.set(liveIns[opV]);
        continue ;
      }

      /*
       * If the value is generated by another instruction inside the loop,
       * set it to the equivalent cloned instruction.
       */
      if (auto opI = dyn_cast<Instruction>(opV)) {
        if (iClones.find(opI) != iClones.end()) {
          op.set(iClones[opI]);
        } else {
          if (opI->getFunction() != task->F) {
            cloneI->print(errs() << "ERROR:   Instruction has op from another function: "); errs() << "\n";
            opI->print(errs() << "ERROR:   Op: "); errs() << "\n";
          }
        }
      }
    }
  }
}

void ParallelizationTechnique::setReducableVariablesToBeginAtIdentityValue (
  LoopDependenceInfo *LDI,
  int taskIndex
){
  auto task = this->tasks[taskIndex];
  for (auto envInd : LDI->environment->getEnvIndicesOfLiveOutVars()) {
    if (!envBuilder->isReduced(envInd)) continue;

    auto producer = LDI->environment->producerAt(envInd);
    assert(isa<PHINode>(producer) && "Reducable producers are assumed to be PHIs");
    auto producerPHI = cast<PHINode>(producer);
    assert(LDI->header == producerPHI->getParent()
      && "Reducable producers are assumed to be live throughout the loop");

    auto producerClone = cast<PHINode>(task->instructionClones[producerPHI]);
    auto preheaderClone = task->basicBlockClones[LDI->preHeader];
    auto incomingIndex = producerClone->getBasicBlockIndex(preheaderClone);
    assert(incomingIndex != -1 && "Loop entry present on producer PHI node");

    auto identityV = getIdentityValueForEnvironmentValue(LDI, taskIndex, envInd);
    producerClone->setIncomingValue(incomingIndex, identityV);
  }
}

Value *ParallelizationTechnique::getIdentityValueForEnvironmentValue (
  LoopDependenceInfo *LDI,
  int taskIndex,
  int environmentIndex
){

/*
      auto producerSCC = LDI->loopSCCDAG->sccOfValue(cast<PHINode>(producer));
      auto firstAccumI = *(LDI->sccdagAttrs.getSCCAttrs(producerSCC)->accumulators.begin());
      auto envPtrType = envPtr->getType();
      auto identityV = LDI->sccdagAttrs.accumOpInfo.generateIdentityFor(
        firstAccumI,
        cast<PointerType>(envPtrType)->getElementType()
      );
*/

  auto producer = LDI->environment->producerAt(environmentIndex);
  auto producerSCC = LDI->loopSCCDAG->sccOfValue(producer);
  assert(producerSCC != nullptr && "The environment value doesn't belong to a loop SCC");

  auto &sccAttrs = LDI->sccdagAttrs.getSCCAttrs(producerSCC);
  assert(sccAttrs->accumulators.size() > 0 && "The environment value isn't accumulated!");

  auto firstAccumI = *(sccAttrs->accumulators.begin());
  return LDI->sccdagAttrs.accumOpInfo.generateIdentityFor(
    firstAccumI,
    producer->getType()
  );
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
  if (task->loopExitBlocks.size() == 1) {
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
  IRBuilder<> entryBuilder(task->entryBlock);
  auto entryTerminator = task->entryBlock->getTerminator();

  auto envType = LDI->environment->typeOfEnv(exitBlockEnvIndex);
  envUser->createEnvPtr(entryBuilder, exitBlockEnvIndex, envType);
  entryTerminator->removeFromParent();
  entryBuilder.Insert(entryTerminator);

  /*
   * Add a store instruction to specify to the code outside the parallelized loop which exit block is taken.
   */
  auto int32 = IntegerType::get(module.getContext(), 32);
  for (int i = 0; i < task->loopExitBlocks.size(); ++i) {
    auto bb = &*task->loopExitBlocks[i];
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
