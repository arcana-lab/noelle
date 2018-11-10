#include "ParallelizationTechnique.hpp"

using namespace llvm;

ParallelizationTechnique::ParallelizationTechnique (Module &module, Verbosity v)
  : module{module}, verbose{v}, workers{}, envBuilder{0} {}

ParallelizationTechnique::~ParallelizationTechnique () {
  reset();
}

void ParallelizationTechnique::reset () {
  for (auto worker : workers) delete worker;
  workers.clear();
  numWorkerInstances = 0;
  if (envBuilder) {
    delete envBuilder;
    envBuilder = nullptr;
  }
}

void ParallelizationTechnique::initializeEnvironmentBuilder (
  LoopDependenceInfoForParallelizer *LDI,
  std::set<int> simpleVars,
  std::set<int> reducableVars
) {
  if (workers.size() == 0) {
    errs() << "ERROR: Parallelization technique workers haven't been created yet!\n"
      << "\tTheir environment builders can't be initialized until they are.\n";
    abort();
  }

  envBuilder = new EnvBuilder(*LDI->environment, module.getContext());
  envBuilder->createEnvVariables(simpleVars, reducableVars, numWorkerInstances);

  envBuilder->createEnvUsers(workers.size());
  for (auto i = 0; i < workers.size(); ++i) {
    auto worker = workers[i];
    auto envUser = envBuilder->getUser(i);
    IRBuilder<> entryBuilder(worker->entryBlock);
    envUser->setEnvArray(entryBuilder.CreateBitCast(
      worker->envArg,
      PointerType::getUnqual(envBuilder->getEnvArrayTy())
    ));
  }
}

void ParallelizationTechnique::allocateEnvironmentArray (LoopDependenceInfoForParallelizer *LDI) {
  IRBuilder<> builder(LDI->entryPointOfParallelizedLoop);
  envBuilder->generateEnvArray(builder);
  envBuilder->generateEnvVariables(builder);
}

void ParallelizationTechnique::populateLiveInEnvironment (LoopDependenceInfoForParallelizer *LDI) {
  IRBuilder<> builder(LDI->entryPointOfParallelizedLoop);
  for (auto envIndex : LDI->environment->getPreEnvIndices()) {
    builder.CreateStore(LDI->environment->producerAt(envIndex), envBuilder->getEnvVar(envIndex));
  }
}

void ParallelizationTechnique::propagateLiveOutEnvironment (LoopDependenceInfoForParallelizer *LDI) {
  IRBuilder<> builder(LDI->entryPointOfParallelizedLoop);
  for (int envInd : LDI->environment->getPostEnvIndices()) {
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

void ParallelizationTechnique::generateWorkers (
  LoopDependenceInfoForParallelizer *LDI,
  std::vector<TechniqueWorker *> workerStructs
) {
  numWorkerInstances = workerStructs.size();
  for (auto i = 0; i < numWorkerInstances; ++i) {
    auto worker = workerStructs[i];
    workers.push_back(worker);

    auto &cxt = module.getContext();
    worker->order = i;
    worker->F = cast<Function>(module.getOrInsertFunction("", workerType));
    worker->extractFuncArgs();
    worker->entryBlock = BasicBlock::Create(cxt, "", worker->F);
    worker->exitBlock = BasicBlock::Create(cxt, "", worker->F);

    /*
     * Map original preheader to entry block
     */
    worker->basicBlockClones[LDI->preHeader] = worker->entryBlock;

    /*
     * Create one basic block per loop exit, mapping between originals and clones,
     * and branching from them to the function exit block
     */
    for (auto exitBB : LDI->loopExitBlocks) {
      auto newExitBB = BasicBlock::Create(cxt, "", worker->F);
      worker->basicBlockClones[exitBB] = newExitBB;
      worker->loopExitBlocks.push_back(newExitBB);
      IRBuilder<> builder(newExitBB);
      builder.CreateBr(worker->exitBlock);
    }
  }
}

void ParallelizationTechnique::cloneSequentialLoop (
  LoopDependenceInfoForParallelizer *LDI,
  int workerIndex
){
  auto &cxt = module.getContext();
  auto worker = workers[workerIndex];

  /*
   * Clone all basic blocks of the original loop
   */
  for (auto originBB : LDI->liSummary.topLoop->bbs) {

    /*
     * Clone the basic block in the context of the original loop's function
     */
    auto cloneBB = BasicBlock::Create(cxt, "", worker->F);
    worker->basicBlockClones[originBB] = cloneBB;

    /*
     * Clone every instruction in the basic block, adding them in order to the clone
     */
    IRBuilder<> builder(cloneBB);
    for (auto &I : *originBB) {
      auto cloneI = builder.Insert(I.clone());
      worker->instructionClones[&I] = cloneI;
    }
  }
}

void ParallelizationTechnique::cloneSequentialLoopSubset (
  LoopDependenceInfoForParallelizer *LDI,
  int workerIndex,
  std::set<Instruction *> subset
){
  auto &cxt = module.getContext();
  auto worker = workers[workerIndex];
  auto &iClones = worker->instructionClones;

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
    auto cloneBB = BasicBlock::Create(cxt, "", worker->F);
    IRBuilder<> builder(cloneBB);
    for (auto &I : *bb) {
      if (iClones.find(&I) == iClones.end()) continue;
      builder.Insert(iClones[&I]);
    }
    worker->basicBlockClones[bb] = cloneBB;
  }
}

void ParallelizationTechnique::generateCodeToLoadLiveInVariables (
  LoopDependenceInfoForParallelizer *LDI, 
  int workerIndex
){
  auto worker = this->workers[workerIndex];
  IRBuilder<> builder(worker->entryBlock);
  auto envUser = this->envBuilder->getUser(workerIndex);
  for (auto envIndex : envUser->getPreEnvIndices()) {

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
    worker->liveInClones[producer] = cast<Instruction>(envLoad);
  }
}

void ParallelizationTechnique::generateCodeToStoreLiveOutVariables (
  LoopDependenceInfoForParallelizer *LDI, 
  int workerIndex
){
  auto worker = this->workers[workerIndex];
  IRBuilder<> entryBuilder(worker->entryBlock);
  auto envUser = this->envBuilder->getUser(workerIndex);
  for (auto envIndex : envUser->getPostEnvIndices()) {

    /*
     * Create GEP access of the single, or reducable, environment variable
     */
    bool isReduced = this->envBuilder->isReduced(envIndex);
    if (isReduced) {
      envUser->createReducableEnvPtr(entryBuilder, envIndex, numWorkerInstances, worker->instanceIndexV);
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
      auto firstAccumI = *(LDI->sccdagAttrs.getSCCAttrs(producerSCC)->PHIAccumulators.begin());
      auto identityV = LDI->sccdagAttrs.accumOpInfo.generateIdentityFor(firstAccumI);
      entryBuilder.CreateStore(identityV, envPtr);
    }

    /*
     * Store the clone of the producer at the environment pointer
     */
    auto prodClone = worker->instructionClones[producer];
    auto prodCloneBB = worker->basicBlockClones[producer->getParent()];
    IRBuilder<> prodBuilder(prodCloneBB->getTerminator());
    prodBuilder.CreateStore(prodClone, envPtr);
  }

  generateCodeToStoreExitBlockIndex(LDI, workerIndex);
}

void ParallelizationTechnique::adjustDataFlowToUseClones (
  LoopDependenceInfoForParallelizer *LDI,
  int workerIndex
){
  auto &worker = workers[workerIndex];
  auto &bbClones = worker->basicBlockClones;
  auto &iClones = worker->instructionClones;
  auto &liveIns = worker->liveInClones;

  for (auto pair : iClones) {
    auto cloneI = pair.second;

    /*
     * Adjust basic block references of terminators and PHI nodes
     */
    if (auto terminator = dyn_cast<TerminatorInst>(cloneI)) {
      for (int i = 0; i < terminator->getNumSuccessors(); ++i) {
        auto succBB = terminator->getSuccessor(i);
        if (succBB->getParent() == worker->F) continue;
        assert(bbClones.find(succBB) != bbClones.end());
        terminator->setSuccessor(i, bbClones[succBB]);
      }
    }

    if (auto phi = dyn_cast<PHINode>(cloneI)) {
      for (int i = 0; i < phi->getNumIncomingValues(); ++i) {
        auto incomingBB = phi->getIncomingBlock(i);
        if (incomingBB->getParent() == worker->F) continue;
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
          if (opI->getFunction() != worker->F) {
            cloneI->print(errs() << "ERROR:   Instruction has op from another function: "); errs() << "\n";
            opI->print(errs() << "ERROR:   Op: "); errs() << "\n";
          }
        }
      }
    }
  }
}

void ParallelizationTechnique::generateCodeToStoreExitBlockIndex (
  LoopDependenceInfoForParallelizer *LDI,
  int workerIndex
){

  /*
   * Confirm whether an exit block choice is represented in the environment
   */
  auto worker = this->workers[workerIndex];
  if (worker->loopExitBlocks.size() == 1) return ;
  auto exitBlockEnvIndex = LDI->environment->indexOfExitBlock();
  assert(exitBlockEnvIndex != -1);

  auto envUser = this->envBuilder->getUser(workerIndex);
  IRBuilder<> entryBuilder(worker->entryBlock);
  envUser->createEnvPtr(entryBuilder, exitBlockEnvIndex);

  auto int32 = IntegerType::get(module.getContext(), 32);
  for (int i = 0; i < worker->loopExitBlocks.size(); ++i) {
    IRBuilder<> builder(&*worker->loopExitBlocks[i]->begin());
    auto envPtr = envUser->getEnvPtr(exitBlockEnvIndex);
    builder.CreateStore(ConstantInt::get(int32, i), envPtr);
  }
}
