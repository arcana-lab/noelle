#include "ParallelizationTechnique.hpp"

using namespace llvm;

ParallelizationTechnique::ParallelizationTechnique (Module &module, Verbosity v)
  : module{module}, verbose{v}, tasks{}, envBuilder{0} {}

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

  envBuilder = new EnvBuilder(*LDI->environment, module.getContext());
  envBuilder->createEnvVariables(simpleVars, reducableVars, numTaskInstances);

  envBuilder->createEnvUsers(tasks.size());
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
  IRBuilder<> builder(LDI->entryPointOfParallelizedLoop);
  envBuilder->generateEnvArray(builder);
  envBuilder->generateEnvVariables(builder);
}

void ParallelizationTechnique::populateLiveInEnvironment (LoopDependenceInfo *LDI) {
  IRBuilder<> builder(LDI->entryPointOfParallelizedLoop);
  for (auto envIndex : LDI->environment->getEnvIndicesOfLiveInVars()) {
    builder.CreateStore(LDI->environment->producerAt(envIndex), envBuilder->getEnvVar(envIndex));
  }
}

void ParallelizationTechnique::propagateLiveOutEnvironment (LoopDependenceInfo *LDI) {
  IRBuilder<> builder(LDI->entryPointOfParallelizedLoop);
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
        depPHI->addIncoming(envVar, LDI->exitPointOfParallelizedLoop);
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
  for (auto originBB : LDI->liSummary.topLoop->bbs) {

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

    /*
     * Create GEP access of the environment variable at the given index
     */
    envUser->createEnvPtr(builder, envIndex);

    /*
     * Load the environment pointer
     * Register the load as a "clone" of the original producer
     */
    auto envLoad = builder.CreateLoad(envUser->getEnvPtr(envIndex));
    auto producer = LDI->environment->producerAt(envIndex);
    task->liveInClones[producer] = cast<Instruction>(envLoad);
  }
}

void ParallelizationTechnique::generateCodeToStoreLiveOutVariables (
  LoopDependenceInfo *LDI, 
  int taskIndex
){
  auto task = this->tasks[taskIndex];
  IRBuilder<> entryBuilder(task->entryBlock);
  auto envUser = this->envBuilder->getUser(taskIndex);
  for (auto envIndex : envUser->getEnvIndicesOfLiveOutVars()) {

    /*
     * Create GEP access of the single, or reducable, environment variable
     */
    bool isReduced = this->envBuilder->isReduced(envIndex);
    if (isReduced) {
      envUser->createReducableEnvPtr(entryBuilder, envIndex, numTaskInstances, task->instanceIndexV);
    } else {
      envUser->createEnvPtr(entryBuilder, envIndex);
    }
    auto envPtr = envUser->getEnvPtr(envIndex);
    auto producer = (Instruction*)LDI->environment->producerAt(envIndex);

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
      auto producerSCC = LDI->loopSCCDAG->sccOfValue(cast<PHINode>(producer));
      auto firstAccumI = *(LDI->sccdagAttrs.getSCCAttrs(producerSCC)->accumulators.begin());
      auto envPtrType = envPtr->getType();
      auto identityV = LDI->sccdagAttrs.accumOpInfo.generateIdentityFor(
        firstAccumI,
        cast<PointerType>(envPtrType)->getElementType()
      );
      entryBuilder.CreateStore(identityV, envPtr);
    }

    /*
     * Store the clone of the producer at the environment pointer
     */
    auto prodClone = task->instructionClones[producer];
    auto prodCloneBB = task->basicBlockClones[producer->getParent()];
    IRBuilder<> prodBuilder(prodCloneBB->getTerminator());
    prodBuilder.CreateStore(prodClone, envPtr);
  }

  generateCodeToStoreExitBlockIndex(LDI, taskIndex);
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

void ParallelizationTechnique::generateCodeToStoreExitBlockIndex (
  LoopDependenceInfo *LDI,
  int taskIndex
){

  /*
   * Confirm whether an exit block choice is represented in the environment
   */
  auto task = this->tasks[taskIndex];
  if (task->loopExitBlocks.size() == 1) return ;
  auto exitBlockEnvIndex = LDI->environment->indexOfExitBlock();
  assert(exitBlockEnvIndex != -1);

  auto envUser = this->envBuilder->getUser(taskIndex);
  IRBuilder<> entryBuilder(task->entryBlock);
  envUser->createEnvPtr(entryBuilder, exitBlockEnvIndex);

  auto int32 = IntegerType::get(module.getContext(), 32);
  for (int i = 0; i < task->loopExitBlocks.size(); ++i) {
    IRBuilder<> builder(&*task->loopExitBlocks[i]->begin());
    auto envPtr = envUser->getEnvPtr(exitBlockEnvIndex);
    builder.CreateStore(ConstantInt::get(int32, i), envPtr);
  }
}
