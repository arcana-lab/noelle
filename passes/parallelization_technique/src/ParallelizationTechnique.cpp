#include "ParallelizationTechnique.hpp"

using namespace llvm;

ParallelizationTechnique::ParallelizationTechnique (Module &module, Verbosity v)
  :
  module{module},
  verbose{v}
  {

  return ;
}

ParallelizationTechnique::~ParallelizationTechnique () {}

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

void ParallelizationTechnique::cloneSequentialLoop (
  LoopDependenceInfoForParallelizer *LDI, 
  std::function<BasicBlock * (void)> createNewBasicBlock,
  std::function<void (BasicBlock *, BasicBlock *)> basicBlockMap,
  std::function<void (Instruction *, Instruction *)> instructionMap
  ){

  /*
   * Create inner loop
   */
  for (auto originBB : LDI->liSummary.topLoop->bbs) {

    /*
     * Create a new basic block for the function that will include the cloned loop.
     */
    auto cloneBB = createNewBasicBlock();
    basicBlockMap(originBB, cloneBB);

    /*
     * Clone every instruction of the current basic block and add them to the cloned basic block just created.
     */
    IRBuilder<> builder(cloneBB);
    for (auto &I : *originBB) {
      auto cloneI = builder.Insert(I.clone());
      instructionMap(&I, cloneI);
    }
  }

  return ;
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

void ParallelizationTechnique::adjustDataFlowToUseClonedInstructions (
  LoopDependenceInfoForParallelizer *LDI,
  unordered_map<Instruction *, Instruction *> fromOriginalToClonedInstruction,
  unordered_map<BasicBlock *, BasicBlock *> fromOriginalToClonedBasicBlock,
  unordered_map<Value *, Value *> fromOriginalToClonedLiveInVariable
  ){

  for (auto iPair : fromOriginalToClonedInstruction) {
    auto cloneI = iPair.second;
    if (auto terminator = dyn_cast<TerminatorInst>(cloneI)) {
      for (int i = 0; i < terminator->getNumSuccessors(); ++i) {
        auto succBB = terminator->getSuccessor(i);
        assert(fromOriginalToClonedBasicBlock.find(succBB) != fromOriginalToClonedBasicBlock.end());
        terminator->setSuccessor(i, fromOriginalToClonedBasicBlock[succBB]);
      }
    }

    if (auto phi = dyn_cast<PHINode>(cloneI)) {
      for (int i = 0; i < phi->getNumIncomingValues(); ++i) {
        auto cloneBB = fromOriginalToClonedBasicBlock[phi->getIncomingBlock(i)];
        phi->setIncomingBlock(i, cloneBB);
      }
    }

    // TODO(angelo): Add exhaustive search of types to parallelization
    // utilities for use in DSWP and here in DOALL
    for (auto &op : cloneI->operands()) {

      /*
       * Fetch the value used by the instruction.
       */
      auto opV = op.get();

      /*
       * Check if the value is a loop live-in one.
       */
      if (fromOriginalToClonedLiveInVariable.find(opV) != fromOriginalToClonedLiveInVariable.end()) {

        /*
         * The value is a loop live-in one.
         * Set it to the value loaded outside the parallelized loop.
         */
        op.set(fromOriginalToClonedLiveInVariable[opV]);
        continue ;
      }

      /*
       * The value is not a loop live-in one.
       * 
       * Check if the value is generated by another instruction inside the loop.
       */
      if (auto opI = dyn_cast<Instruction>(opV)) {
        if (fromOriginalToClonedInstruction.find(opI) != fromOriginalToClonedInstruction.end()) {

         /*
          * The value is generated by another instruction inside the loop.
          * Set it to be the value generated by the equivalent instruction included in the parallelized loop.
          */
          op.set(fromOriginalToClonedInstruction[opI]);
        }
      }
    }
  }

  return ;
}
