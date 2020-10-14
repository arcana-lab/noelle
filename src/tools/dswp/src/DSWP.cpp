/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "DSWP.hpp"

using namespace llvm;
using namespace llvm::noelle;

DSWP::DSWP (
  Module &module,
  Hot &p,
  bool forceParallelization,
  bool enableSCCMerging,
  Verbosity v
) :
  ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences{module, p, forceParallelization, v},
  enableMergingSCC{enableSCCMerging},
  queues{}, queueArrayType{nullptr},
  sccToStage{}, stageArrayType{nullptr},
  zeroIndexForBaseArray{nullptr}
  {

  /*
   * Fetch the function that dispatch the parallelized loop.
   */
  this->taskDispatcher = module.getFunction("NOELLE_DSWPDispatcher");

  /*
   * Fetch the function that executes a stage.
   */
  auto taskExecuter = module.getFunction("stageExecuter");

  /*
   * Define its signature.
   */
  auto taskArgType = taskExecuter->arg_begin()->getType();
  this->taskSignature = cast<FunctionType>(cast<PointerType>(taskArgType)->getElementType());

  return ;
}

void DSWP::reset () {
  ParallelizationTechnique::reset();

  sccToStage.clear();

  for (auto &queue : queues) {
    queue.release();
  }
  queues.clear();

  queueArrayType = nullptr;
  stageArrayType = nullptr;
  zeroIndexForBaseArray = nullptr;
}


bool DSWP::canBeAppliedToLoop (
  LoopDependenceInfo *LDI,
  Noelle &par,
  Heuristics *h
) const {

  /*
   * Check the parent class.
   */
  if (!ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::canBeAppliedToLoop(LDI, par, h)){
    return false;
  }

  /*
   * Fetch the profiles
   */
  auto profiles = par.getProfiles();

  /*
   * Check if there is at least one sequential stage.
   * If there isn't, then this loop is a DOALL. Hence, DSWP is not applicable.
   *
   * Also, compute the coverage of the biggest stage.
   * If the coverage is too high, then the parallelization isn't worth it.
   */
  auto doesSequentialSCCExist = false;
  uint64_t biggestSCC = 0;
  for (auto nodePair : LDI->sccdagAttrs.getSCCDAG()->internalNodePairs()) {

    /*
     * Fetch the current SCC.
     */
    auto currentSCC = nodePair.first;
    auto currentSCCInfo = LDI->sccdagAttrs.getSCCAttrs(currentSCC);

    /*
     * Check the coverage of the SCC.
     */
    auto currentSCCTotalInsts = profiles->getTotalInstructions(currentSCC);
    if (currentSCCTotalInsts > biggestSCC){
      biggestSCC = currentSCCTotalInsts;
    }
    assert(biggestSCC >= currentSCCTotalInsts);

    /*
     * Check if the current SCC can be removed (e.g., because it is due to induction variables).
     * If it is, then this SCC has already been assigned to every dependent partition.
     */
    if (currentSCCInfo->canBeCloned()) {
      continue ;
    }

    /*
     * We found a sequential stage.
     */
    doesSequentialSCCExist = true;
  }

  /*
   * If there isn't a sequential SCC, then this loop is a DOALL. Hence, DSWP is not applicable.
   */
  if (!doesSequentialSCCExist){
    errs() << "DSWP: It is not applicable because the loop doesn't have a sequential SCC\n";
    return false;
  }

  /*
   * Check if we are forced to parallelize
   */
  if (this->forceParallelization){

    /*
     * DSWP is applicable.
     */
    return true;
  }

  /*
   * Check if the parallelization is worth it.
   */
  auto loopTotalInsts = profiles->getTotalInstructions(LDI->getLoopStructure());
  auto biggestSCCCoverage = ((double)biggestSCC) / ((double)loopTotalInsts);
  if (biggestSCCCoverage >= 0.8){

    /*
     * The pipeline would be too imbalance.
     */
    errs() << "DSWP: It is not applicable because the coverage of the biggest SCC is " << biggestSCCCoverage << "\n";
    return false;
  }

  /*
   * Ensure there is not too little execution that is too proportionally iteration-independent for DSWP
   */
  auto loopID = LDI->getID();
  auto loopStructure = LDI->getLoopStructure();
  auto averageInstructions = profiles->getAverageTotalInstructionsPerIteration(loopStructure);
  auto averageInstructionThreshold = 20;
  bool hasLittleExecution = averageInstructions < averageInstructionThreshold;
  auto minimumSequentialFraction = .5;
  auto sequentialFraction = this->computeSequentialFractionOfExecution(LDI, par);
  bool hasProportionallyInsignificantSequentialExecution = sequentialFraction < minimumSequentialFraction;
  if (hasLittleExecution && hasProportionallyInsignificantSequentialExecution) {
    errs() << "Parallelizer:    Loop " << loopID << " has "
      << averageInstructions << " number of sequential instructions on average per loop iteration\n";
    errs() << "Parallelizer:    Loop " << loopID << " has "
      << sequentialFraction << " % sequential execution per loop iteration\n";
    errs() << "Parallelizer:      It will not be partitioned enough for DSWP. The thresholds are at least "
      << averageInstructionThreshold << " instructions per iteration or at least "
      << minimumSequentialFraction << " % sequential execution." << "\n";

    return false;
  }

  /*
   * DSWP is applicable.
   */
  return true ;
}

bool DSWP::apply (
  LoopDependenceInfo *LDI,
  Noelle &par,
  Heuristics *h
) {

  /*
   * Start.
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DSWP: Start\n";
  }

  /*
   * Fetch the header.
   */
  auto loopSummary = LDI->getLoopStructure();
  auto loopHeader = loopSummary->getHeader();
  auto loopFunction = loopHeader->getParent();
  DominatorTree dt(*loopFunction);
  PostDominatorTree pdt(*loopFunction);
  this->originalFunctionDS = new DominatorSummary(dt, pdt);

  /*
   * Partition the SCCDAG.
   */
  this->partitionSCCDAG(LDI, h);

  /*
   * Check if the parallelization is worth it.
   */
  if (!this->forceParallelization && this->partitioner->numberOfPartitions() == 1){

    /*
     * The parallelization isn't worth it as there is only one pipeline stage.
     */
    if (this->verbose != Verbosity::Disabled) {
      errs() << "DSWP:  There is only 1 partition and therefore the parallelization isn't worth it.\n";
    }

    return false;
  }
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DSWP:  There are " << this->partitioner->numberOfPartitions() << " partitions in the SCCDAG\n";
  }

  /*
   * Determine DSWP tasks (stages)
   */
  generateStagesFromPartitionedSCCs(LDI);
  addClonableSCCsToStages(LDI);
  // writeStageGraphsAsDot(*LDI);
  assert(isCompleteAndValidStageStructure(LDI));

  /*
   * Collect which queues need to exist between tasks
   *
   * NOTE: The trimming of the call graph for all tasks is an optimization
   *  that lessens the number of control queues necessary. However,
   *  the algorithm that pops queue values is naive, so the trimming
   *  optimization requires non-control queue information to be collected
   *  prior to its execution. Hence, its weird placement:
   */
  collectDataAndMemoryQueueInfo(LDI, par);
  collectControlQueueInfo(LDI, par);
  // assert(areQueuesAcyclical());
  // writeStageQueuesAsDot(*LDI);

  /*
   * Collect information on stages' environments
   */
  auto liveInVars = LDI->environment->getEnvIndicesOfLiveInVars();
  auto liveOutVars = LDI->environment->getEnvIndicesOfLiveOutVars();
  std::set<int> nonReducableVars(liveInVars.begin(), liveInVars.end());
  nonReducableVars.insert(liveOutVars.begin(), liveOutVars.end());
  std::set<int> reducableVars;

  /*
   * Should an exit block environment variable be necessary, register one 
   */
  if (LDI->numberOfExits() > 1){ 
    nonReducableVars.insert(LDI->environment->indexOfExitBlock());
  }

  initializeEnvironmentBuilder(LDI, nonReducableVars, reducableVars);
  collectLiveInEnvInfo(LDI);
  collectLiveOutEnvInfo(LDI);

  if (this->verbose >= Verbosity::Minimal) {
    printStageSCCs(LDI);
  }
  if (this->verbose >= Verbosity::Minimal) {
    printStageQueues(LDI);
    printEnv(LDI);
  }
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DSWP:  Create " << this->tasks.size() << " pipeline stages\n";
  }

  /*
   * Helper declarations
   */
  this->zeroIndexForBaseArray = cast<Value>(ConstantInt::get(par.int64, 0));
  this->queueArrayType = ArrayType::get(PointerType::getUnqual(par.int8), this->queues.size());
  this->stageArrayType = ArrayType::get(PointerType::getUnqual(par.int8), this->tasks.size());

  /*
   * Create the pipeline stages (technique tasks)
   */
  for (auto i = 0; i < this->tasks.size(); ++i) {
    auto task = (DSWPTask *)this->tasks[i];

    /*
     * Add instructions of the current pipeline stage to the task function
     */
    generateLoopSubsetForStage(LDI, i);
    // if (this->verbose >= Verbosity::Maximal) {
      // printStageClonedValues(*LDI, i);
    // }

    /*
     * Load pointers of all queues for the current pipeline stage at the function's entry
     */
    generateLoadsOfQueuePointers(par, i);
    if (this->verbose >= Verbosity::Maximal) {
      errs() << "DSWP:  Loaded queue pointers\n";
    }

    /*
     * Add push/pop operations from queues between the current pipeline stage and the connected ones
     */
    popValueQueues(LDI, par, i);
    pushValueQueues(LDI, par, i);
    if (this->verbose >= Verbosity::Maximal) {
      errs() << "DSWP:  Added queue pop and push instructions\n";
    }

    /*
     * Load all loop live-in values at the entry point of the task.
     */
    generateCodeToLoadLiveInVariables(LDI, i);
    if (this->verbose >= Verbosity::Maximal) {
      errs() << "DSWP:  Loaded live-in variables\n";
    }

    // SubCFGs execGraph(*task->getTaskBody());
    // DGPrinter::writeGraph<SubCFGs, BasicBlock>("dswp-loop-" + std::to_string(LDI->getID()) + "-task-" + std::to_string(i) + ".dot", &execGraph);
    // dumpToFile(*LDI);

    /*
     * HACK: For now, this must follow loading live-ins as this re-wiring overrides
     * the live-in mapping to use locally cloned memory instructions that are live-in to the loop
     */
    if (LDI->isOptimizationEnabled(LoopDependenceInfoOptimization::MEMORY_CLONING_ID)) {
      this->cloneMemoryLocationsLocallyAndRewireLoop(LDI, i);
    }

    /*
     * Fix the data flow within the parallelized loop by redirecting operands of
     * cloned instructions to refer to the other cloned instructions. Currently,
     * they still refer to the original loop's instructions.
     */
    adjustDataFlowToUseClones(LDI, i);
    if (this->verbose >= Verbosity::Maximal) {
      errs() << "DSWP:  Adjusted data flow between cloned instructions\n";
    }

    /*
     * Add the unconditional branch from the entry basic block to the header of the loop.
     */
    IRBuilder<> entryBuilder(task->getEntry());
    entryBuilder.CreateBr(task->getCloneOfOriginalBasicBlock(loopHeader));

    /*
     * Add the return instruction at the end of the exit basic block.
     */
    IRBuilder<> exitBuilder(task->getExit());
    exitBuilder.CreateRetVoid();

    /*
     * Store final results to loop live-out variables.
     * Generate a store to propagate the information about which exit block has been taken from the parallelized loop to the code outside it.
     */
    generateCodeToStoreLiveOutVariables(LDI, i);
    generateCodeToStoreExitBlockIndex(LDI, i);
    if (this->verbose >= Verbosity::Maximal) {
      errs() << "DSWP:  Stored live out instructions\n";
    }

    /*
     * Inline recursively calls to queues.
     */
    inlineQueueCalls(i);

    if (this->verbose >= Verbosity::Maximal) {
      task->getTaskBody()->print(errs() << "Pipeline stage " << i << ":\n"); errs() << "\n";
      // SubCFGs execGraph(*task->getTaskBody());
      // std::string name = "dswp-task-" + std::to_string(task->getID()) + "-loop-" + std::to_string(LDI->getID()) + ".dot";
      // DGPrinter::writeGraph<SubCFGs, BasicBlock>(name , &execGraph);
    }
  }

  /*
   * Create the whole pipeline by connecting the stages.
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DSWP:  Link pipeline stages\n";
  }
  createPipelineFromStages(LDI, par);

  delete this->originalFunctionDS;

  /*
   * Exit
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DSWP: Exit\n";
  }
  return true;
}
