#include "ParallelizationTechnique.hpp"

using namespace llvm;

ParallelizationTechnique::ParallelizationTechnique (Module &module, Verbosity v)
  : module{module}, verbose{v}, workers{} {}

ParallelizationTechnique::~ParallelizationTechnique () {
  delete envBuilder;
  for (auto worker : workers) delete worker;
}

void ParallelizationTechnique::initEnvBuilder (LoopDependenceInfoForParallelizer *LDI) {
  envBuilder = new EnvBuilder(*LDI->environment, module.getContext());
}

void ParallelizationTechnique::createEnvironment (LoopDependenceInfoForParallelizer *LDI) {
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

void ParallelizationTechnique::createWorkers (int numWorkers) {
  for (auto i = 0; i < numWorkers; ++i) {
    workers->push_back(new TechniqueWorker());
  }
}

void ParallelizationTechnique::cloneSequentialLoop (
  LoopDependenceInfoForParallelizer *LDI,
  int workerIndex
){
  auto &cxt = module->getContext();
  auto worker = workers[workerIndex];

  /*
   * Clone all basic blocks of the original loop
   */
  for (auto originBB : LDI->liSummary.topLoop->bbs) {

    /*
     * Clone the basic block in the context of the original loop's function
     */
    auto cloneBB = BasicBlock::Create(cxt, "", LDI->function);
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
  std::set<SCC *> subset
){
  auto &cxt = module->getContext();
  auto worker = workers[workerIndex];

  /*
   * Clone all basic blocks, whether empty or not
   */
  for (auto bb : LDI->liSummary.topLoop->bbs) {
    auto cloneBB = BasicBlock::Create(cxt, "", LDI->function);
    worker->basicBlockClones[originBB] = cloneBB;
  }

  /*
   * Clone a portion of the original loop (determined by a set of SCCs
   * Determine the set of basic blocks these instructions belong to
   */
  std::set<BasicBlock *> bbSubset;
  for (auto scc : subset) {
    for (auto nodePair : scc->internalNodePairs())
    {
      auto I = cast<Instruction>(nodePair.first);
      worker->instructionClones[I] = I->clone();
      bbSubset.insert(I->getParent());
    }
  }

  /*
   * Add cloned instructions to their respective cloned basic blocks
   */
  for (auto bb : bbSubset) {
    IRBuilder<> builder(worker->basicBlockClones[bb]);
    for (auto &I : *bb) {
      if (worker->instructionClones.find(&I) == worker->instructionClones.end()) continue;
      builder.Insert(worker->instructionClones[&I]);
    }
  }
}

void ParallelizationTechnique::generateCodeToLoadLiveInVariables (
  LoopDependenceInfoForParallelizer *LDI, 
  BasicBlock *appendLoadsInThisBasicBlock,
  std::function<void (Value *originalProducer, Value *generatedLoad)> producerLoadMap
){

  /*
   * Fetch the user.
   */
  auto envUser = this->envBuilder->getUser(0);

  /*
   * Generate loads to load live-in variables.
   */
  IRBuilder<> entryB(appendLoadsInThisBasicBlock);
  for (auto envInd : LDI->environment->getPreEnvIndices()) {
    envUser->createEnvPtr(entryB, envInd);
    auto envLoad = entryB.CreateLoad(envUser->getEnvPtr(envInd));
    auto producer = LDI->environment->producerAt(envInd);
    producerLoadMap(producer, cast<Value>(envLoad));
  }

  return ;
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
        assert(bbClones.find(succBB) != bbClones.end());
        terminator->setSuccessor(i, bbClones[succBB]);
      }
    }

    if (auto phi = dyn_cast<PHINode>(cloneI)) {
      for (int i = 0; i < phi->getNumIncomingValues(); ++i) {
        auto cloneBB = bbClones[phi->getIncomingBlock(i)];
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
       * The value is not a loop live-in one.
       * 
       * If the value is generated by another instruction inside the loop,
       * set it to the equivalent cloned instruction.
       */
      if (auto opI = dyn_cast<Instruction>(opV)) {
        if (iClones.find(opI) != iClones.end()) {
          op.set(iClones[opI]);
        }
      }
    }
  }
}
