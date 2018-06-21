#include "DSWP.hpp"

using namespace llvm;

/*
 * Options of the DSWP pass.
 */
static cl::opt<bool> ForceParallelization("dswp-force", cl::ZeroOrMore, cl::Hidden, cl::desc("Force the parallelization"));
static cl::opt<bool> ForceNoSCCMerge("dswp-no-scc-merge", cl::ZeroOrMore, cl::Hidden, cl::desc("Force no SCC merging when parallelizing"));
static cl::opt<bool> Verbose("dswp-verbose", cl::ZeroOrMore, cl::Hidden, cl::desc("Enable verbose output"));

DSWP::DSWP()
  :
  ModulePass{ID}, 
  forceParallelization{false},
  forceNoSCCMerge{false},
  verbose{false}
  {

  return ;
}

bool DSWP::doInitialization (Module &M) {
  this->forceParallelization |= (ForceParallelization.getNumOccurrences() > 0);
  this->forceNoSCCMerge |= (ForceNoSCCMerge.getNumOccurrences() > 0);
  this->verbose |= (Verbose.getNumOccurrences() > 0);

  return false; 
}

bool DSWP::runOnModule (Module &M) {

  /*
   * Fetch the outputs of the passes we rely on.
   */
  auto& parallelizationFramework = getAnalysis<Parallelization>();

  /*
   * Collect some information.
   */
  errs() << "DSWP: Analyzing the module " << M.getName() << "\n";
  if (!collectThreadPoolHelperFunctionsAndTypes(M, parallelizationFramework)) {
    errs() << "DSWP utils not included!\n";
    return false;
  }

  /*
   * Fetch all the loops we want to parallelize.
   */
  auto loopsToParallelize = this->getLoopsToParallelize(M, parallelizationFramework);
  errs() << "DSWP:  There are " << loopsToParallelize.size() << " loops to parallelize\n";

  /*
   * Parallelize the loops selected.
   */
  auto modified = false;
  for (auto loop : loopsToParallelize){

    /*
     * Parallelize the current loop with DSWP.
     */
    modified |= applyDSWP(loop, parallelizationFramework);

    /*
     * Free the memory.
     */
    delete loop;
  }

  return modified;
}

void DSWP::getAnalysisUsage (AnalysisUsage &AU) const {
  AU.addRequired<PDGAnalysis>();
  AU.addRequired<Parallelization>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();

  return ;
}

bool DSWP::applyDSWP (DSWPLoopDependenceInfo *LDI, Parallelization &par) {
  if (this->verbose) {
    errs() << "DSWP: Check if we can parallelize the loop " << *LDI->header->getFirstNonPHI() << " of function " << LDI->function->getName() << "\n";
  }

  /*
   * Partition SCCs of the SCCDAG.
   */
  if (this->verbose) {
    errs() << "DSWP:  Before partition\n";
    printSCCs(LDI->loopSCCDAG);
  }
  partitionSCCDAG(LDI);
  collectParallelizableSingleInstrNodes(LDI);
  collectRemovableSCCsByInductionVars(LDI);
  if (this->verbose) {
    errs() << "DSWP:  After merge\n";
    printSCCs(LDI->loopSCCDAG);
  }

  /*
   * Check whether it is worth parallelizing the current loop.
   */
  if (!isWorthParallelizing(LDI)) {
    if (this->verbose) {
      errs() << "DSWP:  Not enough TLP can be extracted\n";
    }
    return false;
  }

  /*
   * Collect require information to parallelize the current loop.
   */
  collectStageAndQueueInfo(LDI, par);
  if (this->verbose) {
    printStageSCCs(LDI);
    printStageQueues(LDI);
    printEnv(LDI);
  }

  /*
   * Create the pipeline stages.
   */
  if (this->verbose) {
    errs() << "DSWP:  Create " << LDI->stages.size() << " pipeline stages\n";
  }
  for (auto &stage : LDI->stages) {
    createPipelineStageFromSCC(LDI, stage, par);
  }

  /*
   * Create the pipeline (connecting the stages)
   */
  if (this->verbose) {
    errs() << "DSWP:  Link pipeline stages\n";
  }
  createPipelineFromStages(LDI, par);
  assert(LDI->pipelineBB != nullptr);

  /*
   * Link the parallelized loop within the original function that includes the sequential loop.
   */
  if (this->verbose) {
    errs() << "DSWP:  Link the parallelize loop\n";
  }
  auto exitIndex = cast<Value>(ConstantInt::get(par.int64, LDI->environment->indexOfExitBlock()));
  par.linkParallelizedLoopToOriginalFunction(LDI->function->getParent(), LDI->preHeader, LDI->pipelineBB, LDI->envArray, exitIndex, LDI->loopExitBlocks);
  if (this->verbose){
    LDI->function->print(errs() << "Final printout:\n"); errs() << "\n";
  }

  return true;
}

void DSWP::collectParallelizableSingleInstrNodes (DSWPLoopDependenceInfo *LDI)
{
  for (auto sccNode : LDI->loopSCCDAG->getNodes())
  {
    if (sccNode->getT()->numInternalNodes() == 1) LDI->singleInstrNodes.insert(sccNode->getT());
  }
}

bool DSWP::isWorthParallelizing (DSWPLoopDependenceInfo *LDI) {
  if (this->forceParallelization){
    return true;
  }
  errs() << "WORTH PAR: " << (LDI->loopSCCDAG->numNodes() - LDI->removableSCCs.size()) << "\n";
  return LDI->nextPartitionID > 1;
}

void DSWP::addRemovableSCCsToStages (DSWPLoopDependenceInfo *LDI)
{
  for (auto &stage : LDI->stages)
  {
    std::set<DGNode<SCC> *> visitedNodes;
    std::queue<DGNode<SCC> *> dependentSCCNodes;

    for (auto scc : stage->stageSCCs)
    {
      dependentSCCNodes.push(LDI->loopSCCDAG->fetchNode(scc));
    }

    while (!dependentSCCNodes.empty())
    {
      auto depSCCNode = dependentSCCNodes.front();
      dependentSCCNodes.pop();

      for (auto sccEdge : depSCCNode->getIncomingEdges())
      {
        auto fromSCCNode = sccEdge->getOutgoingNode();
        auto fromSCC = fromSCCNode->getT();
        if (visitedNodes.find(fromSCCNode) != visitedNodes.end()) continue;
        if (LDI->removableSCCs.find(fromSCC) == LDI->removableSCCs.end()) continue;

        stage->removableSCCs.insert(fromSCC);
        dependentSCCNodes.push(fromSCCNode);
        visitedNodes.insert(fromSCCNode);
      }
    }
  }
}

void DSWP::collectPartitionedSCCQueueInfo (DSWPLoopDependenceInfo *LDI)
{
  for (auto scc : LDI->loopSCCDAG->getNodes())
  {
    for (auto sccEdge : scc->getOutgoingEdges())
    {
      auto sccPair = sccEdge->getNodePair();
      auto fromSCC = sccPair.first->getT();
      auto toSCC = sccPair.second->getT();
      if (LDI->removableSCCs.find(fromSCC) != LDI->removableSCCs.end()) continue;
      if (LDI->removableSCCs.find(toSCC) != LDI->removableSCCs.end()) continue;

      auto fromStage = LDI->sccToStage[fromSCC];
      auto toStage = LDI->sccToStage[toSCC];
      if (fromStage == toStage) continue;

      /*
       * Create value queues for each dependency of the form: producer -> consumers
       */
      for (auto instructionEdge : sccEdge->getSubEdges())
      {
        assert(!instructionEdge->isMemoryDependence());
        if (instructionEdge->isControlDependence()) continue;
        auto producer = cast<Instruction>(instructionEdge->getOutgoingT());
        auto consumer = cast<Instruction>(instructionEdge->getIncomingT());
        registerQueue(LDI, fromStage, toStage, producer, consumer);
      }
    }
  }
}

void DSWP::collectTransitiveCondBrs (DSWPLoopDependenceInfo *LDI,
  std::set<TerminatorInst *> &bottomLevelBrs,
  std::set<TerminatorInst *> &descendantCondBrs)
{
  std::queue<DGNode<Value> *> queuedBrs;
  std::set<TerminatorInst *> visitedBrs;
  for (auto br : bottomLevelBrs)
  {
    queuedBrs.push(LDI->loopInternalDG->fetchNode(cast<Value>(br)));
    visitedBrs.insert(br);
  }

  while (!queuedBrs.empty())
  {
    auto brNode = queuedBrs.front();
    auto term = cast<TerminatorInst>(brNode->getT());
    queuedBrs.pop();
    if (term->getNumSuccessors() > 1) descendantCondBrs.insert(term);

    for (auto edge : brNode->getIncomingEdges())
    {
      if (auto termI = dyn_cast<TerminatorInst>(edge->getOutgoingT()))
      {
        if (visitedBrs.find(termI) == visitedBrs.end())
        {
          queuedBrs.push(edge->getOutgoingNode());
          visitedBrs.insert(termI);
        }
      }
    }
  }
}

void DSWP::collectRemovableSCCQueueInfo (DSWPLoopDependenceInfo *LDI)
{
  for (auto &stage : LDI->stages)
  {
    auto toStage = stage.get();
    for (auto removableSCC : stage->removableSCCs)
    {
      for (auto sccEdge : LDI->loopSCCDAG->fetchNode(removableSCC)->getIncomingEdges())
      {
        auto fromSCC = sccEdge->getOutgoingT();
        if (LDI->removableSCCs.find(fromSCC) != LDI->removableSCCs.end()) continue;
        auto fromStage = LDI->sccToStage[fromSCC];

        /*
         * Create value queues for each dependency of the form: producer -> consumers
         */
        for (auto instructionEdge : sccEdge->getSubEdges())
        {
          assert(!instructionEdge->isMemoryDependence());
          if (instructionEdge->isControlDependence()) continue;
          auto producer = cast<Instruction>(instructionEdge->getOutgoingT());
          auto consumer = cast<Instruction>(instructionEdge->getIncomingT());
          registerQueue(LDI, fromStage, toStage, producer, consumer);
        }
      }
    }
  }
}

void DSWP::configureDependencyStorage (DSWPLoopDependenceInfo *LDI, Parallelization &par)
{
  LDI->zeroIndexForBaseArray = cast<Value>(ConstantInt::get(par.int64, 0));
  LDI->envArrayType = ArrayType::get(PointerType::getUnqual(par.int8), LDI->environment->envSize());
  LDI->queueArrayType = ArrayType::get(PointerType::getUnqual(par.int8), LDI->queues.size());
  LDI->stageArrayType = ArrayType::get(PointerType::getUnqual(par.int8), LDI->stages.size());
}

void DSWP::collectStageAndQueueInfo (DSWPLoopDependenceInfo *LDI, Parallelization &par)
{
  createStagesfromPartitionedSCCs(LDI);
  addRemovableSCCsToStages(LDI);

  collectPartitionedSCCQueueInfo(LDI);
  trimCFGOfStages(LDI);
  collectControlQueueInfo(LDI);
  collectRemovableSCCQueueInfo(LDI);

  LDI->environment = std::make_unique<EnvInfo>();
  LDI->environment->exitBlockType = par.int32;
  collectPreLoopEnvInfo(LDI);
  collectPostLoopEnvInfo(LDI);

  configureDependencyStorage(LDI, par);
}

void DSWP::createInstAndBBForSCC (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
{
  auto &context = LDI->function->getParent()->getContext();

  /*
   * Clone instructions within the stage's scc, and removable sccs
   */
  std::set<BasicBlock *> allBBs;
  auto cloneInstructionsOf = [&](SCC *scc) -> void {
    for (auto nodePair : scc->internalNodePairs())
    {
      auto I = cast<Instruction>(nodePair.first);
      stageInfo->iCloneMap[I] = I->clone();
      allBBs.insert(I->getParent());
    }
  };

  for (auto scc : stageInfo->stageSCCs) cloneInstructionsOf(scc);
  for (auto scc : stageInfo->removableSCCs) cloneInstructionsOf(scc);

  /*
   * Clone loop basic blocks and terminators
   */
  for (auto B : LDI->loopBBs) {
    stageInfo->sccBBCloneMap[B] = BasicBlock::Create(context, "", stageInfo->sccStage);
    auto terminator = cast<Instruction>(B->getTerminator());
    if (stageInfo->iCloneMap.find(terminator) == stageInfo->iCloneMap.end()) {
      if (stageInfo->usedCondBrs.find(B->getTerminator()) != stageInfo->usedCondBrs.end())
      {
        stageInfo->iCloneMap[terminator] = terminator->clone();
        continue;
      }
      stageInfo->iCloneMap[terminator] = BranchInst::Create(LDI->loopBBtoPD[B]);
    }
  }
  for (int i = 0; i < LDI->loopExitBlocks.size(); ++i) {
    stageInfo->sccBBCloneMap[LDI->loopExitBlocks[i]] = stageInfo->loopExitBlocks[i];
  }

  /*
   * Attach SCC instructions to their basic blocks in correct relative order
   */
  int size = stageInfo->iCloneMap.size();
  int instrInserted = 0;
  for (auto B : LDI->loopBBs) {
    IRBuilder<> builder(stageInfo->sccBBCloneMap[B]);
    for (auto &I : *B)
    {
      if (stageInfo->iCloneMap.find(&I) == stageInfo->iCloneMap.end()) continue;
      builder.Insert(stageInfo->iCloneMap[&I]);
      instrInserted++;
    }
  }
  assert(instrInserted == size);
}

void DSWP::loadAndStoreEnv (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo, Parallelization &par)
{
  IRBuilder<> entryBuilder(stageInfo->entryBlock);

  auto envArg = &*(stageInfo->sccStage->arg_begin());
  stageInfo->envAlloca = entryBuilder.CreateBitCast(envArg, PointerType::getUnqual(LDI->envArrayType));

  auto accessProducerFromIndex = [&](int envIndex, IRBuilder<> builder) -> Value * {
    auto envIndexValue = cast<Value>(ConstantInt::get(par.int64, envIndex));
    auto envPtr = builder.CreateInBoundsGEP(stageInfo->envAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndexValue }));
    auto envType = LDI->environment->envProducers[envIndex]->getType();
    return builder.CreateBitCast(builder.CreateLoad(envPtr), PointerType::getUnqual(envType));
  };

  /*
   * Store (SCC -> outside of loop) dependencies within the environment array
   */
  for (auto outgoingEnvPair : stageInfo->outgoingEnvs)
  {
    auto outgoingDepClone = stageInfo->iCloneMap[outgoingEnvPair.first];
    auto outgoingDepBB = outgoingDepClone->getParent();
    IRBuilder<> outgoingBuilder(outgoingDepBB->getTerminator());
    auto envVar = accessProducerFromIndex(outgoingEnvPair.second, outgoingBuilder);
    outgoingBuilder.CreateStore(outgoingDepClone, envVar);
  }

  /*
   * Store exit index in the exit environment variable
   */
  for (int i = 0; i < stageInfo->loopExitBlocks.size(); ++i)
  {
    IRBuilder<> builder(&*stageInfo->loopExitBlocks[i]->begin());
    auto envIndexValue = cast<Value>(ConstantInt::get(par.int64, LDI->environment->indexOfExitBlock()));
    auto envPtr = builder.CreateInBoundsGEP(stageInfo->envAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndexValue }));
    auto envVar = builder.CreateBitCast(builder.CreateLoad(envPtr), PointerType::getUnqual(par.int32));
    builder.CreateStore(ConstantInt::get(par.int32, i), envVar);
  }

  /*
   * Load (outside of loop -> SCC) dependencies from the environment array 
   */
  for (auto envIndex : stageInfo->incomingEnvs)
  {
    auto envVar = accessProducerFromIndex(envIndex, entryBuilder);
    auto envLoad = entryBuilder.CreateLoad(envVar);
    stageInfo->envLoadMap[envIndex] = cast<Instruction>(envLoad);
  }
}

void DSWP::loadAllQueuePointersInEntry (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo, Parallelization &par) {
  IRBuilder<> entryBuilder(stageInfo->entryBlock);
  auto argIter = stageInfo->sccStage->arg_begin();
  auto queuesArray = entryBuilder.CreateBitCast(&*(++argIter), PointerType::getUnqual(LDI->queueArrayType));

  /*
   * Load this stage's relevant queues
   */
  auto loadQueuePtrFromIndex = [&](int queueIndex) -> void {
    auto queueInfo = LDI->queues[queueIndex].get();
    auto queueIndexValue = cast<Value>(ConstantInt::get(par.int64, queueIndex));
    auto queuePtr = entryBuilder.CreateInBoundsGEP(queuesArray, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, queueIndexValue }));
    auto queueCast = entryBuilder.CreateBitCast(queuePtr, PointerType::getUnqual(queueTypes[queueSizeToIndex[queueInfo->bitLength]]));

    auto queueInstrs = std::make_unique<QueueInstrs>();
    queueInstrs->queuePtr = entryBuilder.CreateLoad(queueCast);
    queueInstrs->alloca = entryBuilder.CreateAlloca(queueInfo->dependentType);
    queueInstrs->allocaCast = entryBuilder.CreateBitCast(queueInstrs->alloca, PointerType::getUnqual(queueElementTypes[queueSizeToIndex[queueInfo->bitLength]]));
    stageInfo->queueInstrMap[queueIndex] = std::move(queueInstrs);
  };

  for (auto queueIndex : stageInfo->pushValueQueues) loadQueuePtrFromIndex(queueIndex);
  for (auto queueIndex : stageInfo->popValueQueues) loadQueuePtrFromIndex(queueIndex);
}

void DSWP::remapOperandsOfInstClones (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
{
  auto &iCloneMap = stageInfo->iCloneMap;
  auto &envMap = LDI->environment->producerIndexMap;
  auto &queueMap = stageInfo->producedPopQueue;

  auto ignoreOperandAbort = [&](Value *opV, Instruction *cloneInstruction) -> void {
    opV->print(errs() << "Ignore operand\t"); cloneInstruction->print(errs() << "\nInstr:\t"); errs() << "\n";
    stageInfo->sccStage->print(errs() << "Current function state:\n"); errs() << "\n";
    abort();
  };

  for (auto ii = iCloneMap.begin(); ii != iCloneMap.end(); ++ii) {
    auto cloneInstruction = ii->second;

    for (auto &op : cloneInstruction->operands()) {
      auto opV = op.get();
      if (auto opI = dyn_cast<Instruction>(opV)) {
        if (iCloneMap.find(opI) != iCloneMap.end()) {
          op.set(iCloneMap[opI]);
        } else if (LDI->environment->isPreLoopEnv(opV)) {
          op.set(stageInfo->envLoadMap[envMap[opV]]);
        } else if (queueMap.find(opI) != queueMap.end()) {
          op.set(stageInfo->queueInstrMap[queueMap[opI]]->load);
        } else {
          ignoreOperandAbort(opV, cloneInstruction);
        }
        continue;
      } else if (auto opA = dyn_cast<Argument>(opV)) {
        if (LDI->environment->isPreLoopEnv(opV)) {
          op.set(stageInfo->envLoadMap[envMap[opV]]);
        } else {
          ignoreOperandAbort(opV, cloneInstruction);
        }
      } else if (isa<Constant>(opV) || isa<BasicBlock>(opV) || isa<Function>(opV)) {
        continue;
      } else if (isa<MetadataAsValue>(opV) || isa<InlineAsm>(opV) || isa<DerivedUser>(opV) || isa<Operator>(opV)) {
        continue;
      } else {
        opV->print(errs() << "Unknown what to do with operand\n"); opV->getType()->print(errs() << "\tType:\t");
        cloneInstruction->print(errs() << "\nInstr:\t"); errs() << "\n";
        stageInfo->sccStage->print(errs() << "Current function state:\n"); errs() << "\n";
        abort();
      }
    }
  }
}

Value * DSWP::createEnvArrayFromStages (DSWPLoopDependenceInfo *LDI, IRBuilder<> funcBuilder, IRBuilder<> builder, Parallelization &par)
{
  /*
   * Create empty environment array for producers, exit block tracking
   */
  std::vector<Value*> envPtrs;
  for (int i = 0; i < LDI->environment->envSize(); ++i)
  {
    Type *envType = LDI->environment->typeOfEnv(i);
    auto varAlloca = funcBuilder.CreateAlloca(envType);
    envPtrs.push_back(varAlloca);
    auto envIndex = cast<Value>(ConstantInt::get(par.int64, i));
    auto envPtr = funcBuilder.CreateInBoundsGEP(LDI->envArray, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndex }));
    auto depCast = funcBuilder.CreateBitCast(envPtr, PointerType::getUnqual(PointerType::getUnqual(envType)));
    funcBuilder.CreateStore(varAlloca, depCast);
  }

  /*
   * Insert pre-loop producers into the environment array
   */
  for (int envIndex : LDI->environment->preLoopEnv)
  {
    builder.CreateStore(LDI->environment->envProducers[envIndex], envPtrs[envIndex]);
  }
  
  return cast<Value>(builder.CreateBitCast(LDI->envArray, PointerType::getUnqual(par.int8)));
}

Value * DSWP::createStagesArrayFromStages (DSWPLoopDependenceInfo *LDI, IRBuilder<> funcBuilder, Parallelization &par)
{
  auto stagesAlloca = cast<Value>(funcBuilder.CreateAlloca(LDI->stageArrayType));
  auto stageCastType = PointerType::getUnqual(LDI->stages[0]->sccStage->getType());
  for (int i = 0; i < LDI->stages.size(); ++i)
  {
    auto &stage = LDI->stages[i];
    auto stageIndex = cast<Value>(ConstantInt::get(par.int64, i));
    auto stagePtr = funcBuilder.CreateInBoundsGEP(stagesAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, stageIndex }));
    auto stageCast = funcBuilder.CreateBitCast(stagePtr, stageCastType);
    funcBuilder.CreateStore(stage->sccStage, stageCast);
  }
  return cast<Value>(funcBuilder.CreateBitCast(stagesAlloca, PointerType::getUnqual(par.int8)));
}

Value * DSWP::createQueueSizesArrayFromStages (DSWPLoopDependenceInfo *LDI, IRBuilder<> funcBuilder, Parallelization &par)
{
  auto queuesAlloca = cast<Value>(funcBuilder.CreateAlloca(ArrayType::get(par.int64, LDI->queues.size())));
  for (int i = 0; i < LDI->queues.size(); ++i)
  {
    auto &queue = LDI->queues[i];
    auto queueIndex = cast<Value>(ConstantInt::get(par.int64, i));
    auto queuePtr = funcBuilder.CreateInBoundsGEP(queuesAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, queueIndex }));
    auto queueCast = funcBuilder.CreateBitCast(queuePtr, PointerType::getUnqual(par.int64));
    funcBuilder.CreateStore(ConstantInt::get(par.int64, queue->bitLength), queueCast);
  }
  return cast<Value>(funcBuilder.CreateBitCast(queuesAlloca, PointerType::getUnqual(par.int64)));
}

void DSWP::storeOutgoingDependentsIntoExternalValues (DSWPLoopDependenceInfo *LDI, IRBuilder<> builder, Parallelization &par)
{
  /*
   * Extract the outgoing dependents for each stage
   */
  for (int envInd : LDI->environment->postLoopEnv)
  {
    auto prod = LDI->environment->envProducers[envInd];
    auto envIndex = cast<Value>(ConstantInt::get(par.int64, envInd));
    auto depInEnvPtr = builder.CreateInBoundsGEP(LDI->envArray, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndex }));
    auto envVarCast = builder.CreateBitCast(builder.CreateLoad(depInEnvPtr), PointerType::getUnqual(prod->getType()));
    auto envVar = builder.CreateLoad(envVarCast);

    for (auto consumer : LDI->environment->prodConsumers[prod])
    {
      if (auto depPHI = dyn_cast<PHINode>(consumer))
      {
        depPHI->addIncoming(envVar, LDI->pipelineBB);
        continue;
      }
      LDI->pipelineBB->eraseFromParent();
      prod->print(errs() << "Producer of environment variable:\t"); errs() << "\n";
      errs() << "Loop not in LCSSA!\n";
      abort();
    }
  }
}

// Next there is code to register your pass to "opt"
char llvm::DSWP::ID = 0;
static RegisterPass<DSWP> X("DSWP", "DSWP parallelization");

// Next there is code to register your pass to "clang"
static DSWP * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new DSWP());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new DSWP());}});// ** for -O0
