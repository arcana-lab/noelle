/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "HELIX.hpp"
#include "HELIXTask.hpp"

namespace llvm::noelle{

HELIX::HELIX (
  Noelle &n,
  bool forceParallelization
  )
  : ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences{n, forceParallelization},
    loopCarriedLoopEnvironmentBuilder{nullptr}, 
    taskFunctionDG{nullptr},
    lastIterationExecutionBlock{nullptr},
    enableInliner{true}
  {

  /*
   * Fetch the LLVM context.
   */
  auto program = this->noelle.getProgram();

  /*
   * Fetch the dispatcher to use to jump to a parallelized HELIX loop.
   */
  this->taskDispatcherSS = program->getFunction("NOELLE_HELIX_dispatcher_sequentialSegments");
  assert(this->taskDispatcherSS != nullptr);
  this->taskDispatcherCS = program->getFunction("NOELLE_HELIX_dispatcher_criticalSections");
  assert(this->taskDispatcherCS != nullptr);
  this->waitSSCall = program->getFunction("HELIX_wait");
  this->signalSSCall = program->getFunction("HELIX_signal");
  if (!this->waitSSCall  || !this->signalSSCall) {
    errs() << "HELIX: ERROR = sync functions HELIX_wait, HELIX_signal were not both found.\n";
    abort();
  }

  /*
   * Fetch the LLVM types of the HELIX_dispatcher arguments.
   */
  auto tm = noelle.getTypesManager();
  auto int8 = tm->getIntegerType(8);
  auto int64 = tm->getIntegerType(64);
  auto ptrType = tm->getVoidPointerType();
  auto voidType = tm->getVoidType();

  /*
   * Create the LLVM signature of HELIX_dispatcher.
   */
  auto funcArgTypes = ArrayRef<Type*>({
    ptrType,
    ptrType,
    ptrType,
    ptrType,
    int64,
    int64,
    PointerType::getUnqual(int64)
  });
  this->taskSignature = FunctionType::get(voidType, funcArgTypes, false);

  return ;
}

bool HELIX::canBeAppliedToLoop (LoopDependenceInfo *LDI, Heuristics *h) const {

  /*
   * Check the parent class.
   */
  if (!ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::canBeAppliedToLoop(LDI, h)){
    return false;
  }

  /*
   * Check if we are forced to parallelize
   */
  if (this->forceParallelization){

    /*
     * HELIX is applicable.
     */
    return true;
  }

  /*
   * Ensure there is not too little execution that is too proportionally sequential for HELIX 
   */
  auto profiles = this->noelle.getProfiles();
  auto loopID = LDI->getID();
  auto loopStructure = LDI->getLoopStructure();
  auto averageInstructions = profiles->getAverageTotalInstructionsPerIteration(loopStructure);
  auto averageInstructionThreshold = 20;
  auto hasLittleExecution = averageInstructions < averageInstructionThreshold;
  auto maximumSequentialFraction = .2;
  auto sequentialFraction = this->computeSequentialFractionOfExecution(LDI, this->noelle);
  auto hasProportionallySignificantSequentialExecution = sequentialFraction >= maximumSequentialFraction;
  if (hasLittleExecution && hasProportionallySignificantSequentialExecution) {
    errs() << "Parallelizer:    Loop " << loopID << " has "
      << averageInstructions << " number of sequential instructions on average per loop iteration\n";
    errs() << "Parallelizer:    Loop " << loopID << " has "
      << sequentialFraction << " % sequential execution per loop iteration\n";
    errs() << "Parallelizer:      It will be too heavily synchronized for HELIX. The thresholds are at least "
      << averageInstructionThreshold << " instructions per iteration or less than "
      << maximumSequentialFraction << " % sequential execution." << "\n";

    return false;
  }

  return true ;
}

bool HELIX::apply (
  LoopDependenceInfo *LDI,
  Heuristics *h
) {

  /*
   * If a task has not been defined, create such a task from the
   * loop dependence info of the original function's loop
   * Otherwise, add synchronization to the already defined task
   * using the loop dependence info for that task
   */
  if (this->tasks.size() == 0) {
    this->createParallelizableTask(LDI, h);
    return true;
  }

  auto modified = this->synchronizeTask(LDI, h);

  return modified;
}

void HELIX::createParallelizableTask (
  LoopDependenceInfo *LDI,
  Heuristics *h
){

  /*
   * Fetch the header.
   */
  auto loopStructure = LDI->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();

  /*
   * Fetch the SCC manager.
   */
  auto sccManager = LDI->getSCCManager();

  /*
   * NOTE: Keep around the original loops' LoopDependenceInfo for later phases
   * //TODO: we need to specify why this is necessary
   */
  this->originalLDI = LDI;

  /*
   * Print the parallelization request.
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "HELIX: Start the parallelization\n";
    auto nonDOALLSCCs = sccManager->getSCCsWithLoopCarriedDependencies();
    for (auto scc : nonDOALLSCCs) {

      /*
       * Fetch the SCC metadata.
       */
      auto sccInfo = sccManager->getSCCAttrs(scc);

      /*
       * Check the SCC.
       */
      if (sccInfo->canExecuteReducibly()){
        continue ;
      }
      if (sccInfo->canBeCloned()){
        continue ;
      }
      if (LDI->isSCCContainedInSubloop(scc)) {
        continue ;
      }

      /*
       * The current SCC needs to create a sequential segment.
       */
      errs() << "HELIX:   We found an SCC of type " << sccInfo->getType() << " of the loop that is non clonable and non commutative\n" ;
      if (this->verbose >= Verbosity::Maximal) {
        // errs() << "HELIX:     SCC:\n";
        // scc->printMinimal(errs(), "HELIX:       ") ;
        errs() << "HELIX:       Loop-carried dependences\n";
        sccManager->iterateOverLoopCarriedDependences(scc, [](DGEdge<Value> *dep) -> bool {
          auto fromInst = dep->getOutgoingT();
          auto toInst = dep->getIncomingT();
          errs() << "HELIX:       " << *fromInst << " ---> " << *toInst ;

          /*
           * Control dependences.
           */
          if (dep->isControlDependence()){
            errs() << " control\n";
            return false;
          }

          /*
           * Data dependences.
           */
          if (dep->isMemoryDependence()){
            errs() << " via memory\n";
          } else {
            errs() << " via variable\n";
          }
          return false;
            });
      }
    }
  }

  /*
   * Compute reachability so that determining whether spill loads placed in loop
   * exit blocks could be invalidated by spill stores in the loop. If so,
   * they will have to be placed within the loop (which is less optimal)
   * NOTE: This is computed BEFORE addPredecessorAndSuccessorsBasicBlocksToTasks creates an empty basic block
   * in the original function which will be used to link this task
   */
  auto reachabilityDFR = this->computeReachabilityFromInstructions(LDI);

  /*
   * Generate empty tasks for the HELIX execution.
   */
  auto program = this->noelle.getProgram();
  auto helixTask = new HELIXTask(this->taskSignature, *program);
  this->addPredecessorAndSuccessorsBasicBlocksToTasks(LDI, { helixTask });
  auto ltm = LDI->getLoopTransformationsManager();
  this->numTaskInstances = ltm->getMaximumNumberOfCores();
  assert(helixTask == this->tasks[0]);

  /*
   * Fetch the environment of the loop
   */
  auto environment = LDI->getEnvironment();
  assert(environment != nullptr);

  /*
   * Fetch the indices of live-in and live-out variables of the loop being parallelized.
   */
  auto liveInVars = environment->getEnvIndicesOfLiveInVars();
  auto liveOutVars = environment->getEnvIndicesOfLiveOutVars();

  /*
   * Add all live-in and live-out variables as variables to be included in the environment.
   */
  std::set<uint32_t> nonReducableVars(liveInVars.begin(), liveInVars.end());
  std::set<uint32_t> reducableVars{};
  for (auto liveOutIndex : liveOutVars) {

    /*
     * We have a live-out variable.
     *
     * Check if it can be reduced so we can generate more efficient code that does not require a sequential segment.
     */
    auto producer = environment->producerAt(liveOutIndex);
    auto scc = sccManager->getSCCDAG()->sccOfValue(producer);
    auto sccInfo = sccManager->getSCCAttrs(scc);
    if (sccInfo->canExecuteReducibly()){

      /*
       * The live-out variable can be reduced so we can generate more efficient code that does not require a sequential segment.
       */
      reducableVars.insert(liveOutIndex);

    } else {
      nonReducableVars.insert(liveOutIndex);
    }
  }

  /*
   * Add the memory location of the environment used to store the exit block taken to leave the parallelized loop.
   * This location exists only if there is more than one loop exit.
   */
  if (loopStructure->numberOfExitBasicBlocks() > 1){ 
    nonReducableVars.insert(environment->indexOfExitBlockTaken());
  }

  /*
   * Build the single environment that is shared between all instances of the HELIX task.
   */
  this->initializeEnvironmentBuilder(LDI, nonReducableVars, reducableVars);

  /*
   * Clone the sequential loop and store the cloned instructions/basic blocks within the single task of HELIX.
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "HELIX:  Cloning loop in task\n";
  }
  this->cloneSequentialLoop(LDI, 0);

  /*
   * Load all loop live-in values at the entry point of the task.
   * Store final results to loop live-out variables.
   */
  auto envUser = this->envBuilder->getUser(0);
  for (auto envIndex : environment->getEnvIndicesOfLiveInVars()) {
    envUser->addLiveInIndex(envIndex);
  }
  for (auto envIndex : environment->getEnvIndicesOfLiveOutVars()) {
    envUser->addLiveOutIndex(envIndex);
  }
  this->generateCodeToLoadLiveInVariables(LDI, 0);

  /*
   * HACK: For now, this must follow loading live-ins as this re-wiring overrides
   * the live-in mapping to use locally cloned memory instructions that are live-in to the loop
   */
  if (ltm->isOptimizationEnabled(LoopDependenceInfoOptimization::MEMORY_CLONING_ID)) {
    this->cloneMemoryLocationsLocallyAndRewireLoop(LDI, 0);
  }

  /*
   * The operands of the cloned instructions still refer to the original ones.
   *
   * Fix the data flow within the parallelized loop by redirecting operands of cloned instructions to refer to the other cloned instructions. 
   */
  this->adjustDataFlowToUseClones(LDI, 0);
  this->setReducableVariablesToBeginAtIdentityValue(LDI, 0);

  /*
   * Add the unconditional branch from the entry basic block to the header of the loop.
   */
  IRBuilder<> entryBuilder(helixTask->getEntry());
  entryBuilder.CreateBr(helixTask->getCloneOfOriginalBasicBlock(loopHeader));

  /*
   * Spill loop carried dependencies into a separate environment array
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "HELIX:  Check if we need to spill variables because they are part of loop carried data dependencies\n";
  }
  this->spillLoopCarriedDataDependencies(LDI, reachabilityDFR);

  /*
   * For IVs that were not spilled, adjust their step size appropriately
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "HELIX:  Adjusting loop IVs\n";
  }
  this->rewireLoopForIVsToIterateNthIterations(LDI);

  /*
   * Add the final return instruction to the single task's exit block.
   */
  IRBuilder<> exitB(helixTask->getExit());
  exitB.CreateRetVoid();

  if (this->verbose >= Verbosity::Maximal) {
    SubCFGs execGraph(*helixTask->getTaskBody());
    // DGPrinter::writeGraph<SubCFGs, BasicBlock>("unsync-helixtask-loop" + std::to_string(LDI->getID()) + ".dot", &execGraph);
  }

  /*
   * Delete reachability results
   */
  delete reachabilityDFR;

  return ;
}

bool HELIX::synchronizeTask (
  LoopDependenceInfo *LDI,
  Heuristics *h
){

  /*
   * Fetch the HELIX task.
   */
  auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);

  /*
   * Compute reachability analysis for computing SS frontiers and scheduling SS instructions
   */
  auto reachabilityDFR = this->computeReachabilityFromInstructions(LDI);

  /*
   * Schedule the code to minimize the instructions within each sequential segment.
   * HACK: Entries and exits are determined when identifying a sequential segment. They
   * aren't adjusted after squeezing. Delay computing entry and exit frontiers for identified
   * sequential segments until AFTER squeezing.
   */
  auto sequentialSegments = this->identifySequentialSegments(originalLDI, LDI, reachabilityDFR);
  this->squeezeSequentialSegments(LDI, &sequentialSegments, reachabilityDFR);

  /*
   * Free the memory.
   */
  delete reachabilityDFR;
  for (auto ss : sequentialSegments) {
    delete ss;
  }

  /*
   * Re-compute reachability analysis after squeezing sequential segments
   * Identify the sequential segments.
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "HELIX:  Identifying sequential segments\n";
  }
  reachabilityDFR = this->computeReachabilityFromInstructions(LDI);
  sequentialSegments = this->identifySequentialSegments(originalLDI, LDI, reachabilityDFR);

  /*
   * Schedule the sequential segments to overlap parallel and sequential segments.
   */
  this->scheduleSequentialSegments(LDI, &sequentialSegments, reachabilityDFR);

  /*
   * Delete reachability results here before we decide whether to continue with the HELIX parallelization
   */
  delete reachabilityDFR;

  /*
   * Check if any sequential segment's entry and exit frontier spans the entire loop execution
   * If so, do not parallelize
   */
  if (!this->forceParallelization) {
    auto loopStructure = LDI->getLoopStructure();
    auto loopHeader = loopStructure->getHeader();
    auto loopLatches = loopStructure->getLatches();
    for (auto sequentialSegment : sequentialSegments) {
      bool entryAtHeader = false, exitAtLatch = false;
      sequentialSegment->forEachEntry([&](Instruction *entry) -> void {
        auto entryBlock = entry->getParent();
        entryAtHeader |= loopHeader == entryBlock;
        exitAtLatch |= loopLatches.find(entryBlock) != loopLatches.end();
      });
      if (!entryAtHeader || !exitAtLatch) continue;

      /*
       * The HELIX parallelization isn't worth it.
       */
      if (this->verbose != Verbosity::Disabled) {
        errs() << "HELIX: There is a sequential segment spanning the entire loop; therefore, the parallelization isn't worth it.\n";
      }

      /*
       * Free the memory.
       */
      for (auto ss : sequentialSegments) {
        delete ss;
      }

      return false;
    }
  }

  /*
   * Add synchronization instructions.
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "HELIX:  Synchronizing sequential segments\n";
  }
  this->addSynchronizations(LDI, &sequentialSegments);

  /*
   * Store final results of loop live-out variables. 
   *
   * Note this occurs after synchronization has been put in place. This is to ensure
   * that reducible variables not tracked in the loop carried environment are properly
   * propagated as live outs even when check-exit fails and branches directly to the task function's
   * exit block (it can't branch to the task loop's exit blocks because logic in those exit blocks
   * should only be executed by the last iteration, not by all cores)
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "HELIX:  Storing live out variables and exit block index\n";
  }

  /*
   * HACK: swap the mapping so that original loop instructions map to possible duplicates
   * that were created should the original header have had sequential instructions that
   * were moved to the loop body and duplicated after exiting the loop
   *
   * Once live out storing is finished, restore the mapping
   */
  std::unordered_map<Instruction *, Instruction *> loopBodyExecutionMap;
  for (auto lastExecutionInstPair : this->lastIterationExecutionDuplicateMap) {
    auto originalI = lastExecutionInstPair.first;
    auto exitCloneI = lastExecutionInstPair.second;
    auto loopBodyCloneI = helixTask->getCloneOfOriginalInstruction(originalI);
    helixTask->addInstruction(originalI, exitCloneI);
    loopBodyExecutionMap.insert(std::make_pair(originalI, loopBodyCloneI));
  }

  /*
   * NOTE: The assumption being made here is that if we have a last iteration execution block,
   * it is because we have attributed a loop governing IV. Our attribution relies on there
   * being only one loop exit that is controlled by an IV. Hence, we fetch the lone exit block
   */
  auto originalExitBlocks = this->originalLDI->getLoopStructure()->getLoopExitBasicBlocks();
  auto originalSingleExitBlock = *originalExitBlocks.begin();
  BasicBlock * cloneLoopExitBlock = nullptr;
  if (this->lastIterationExecutionBlock) {
    assert(originalExitBlocks.size() == 1 && "loop governing IV attribution relies on only one exit block!");
    cloneLoopExitBlock = helixTask->getCloneOfOriginalBasicBlock(originalSingleExitBlock);
    helixTask->addBasicBlock(originalSingleExitBlock, lastIterationExecutionBlock);
  }

  /*
   * Generate stores for live out variables
   * Generate a store to propagate information about which exit block the parallelized loop took.
   */
  this->generateCodeToStoreLiveOutVariables(this->originalLDI, 0);
  this->generateCodeToStoreExitBlockIndex(this->originalLDI, 0);

  /*
   * HACK: reset the last clone map to reflect the loop exit block which is the successor
   * to the if else branch determining whether to execute the last iteration block before the loop exit block  
   */
  for (auto loopBodyExecutionInstPair : loopBodyExecutionMap) {
    auto originalI = loopBodyExecutionInstPair.first;
    auto loopBodyCloneI = loopBodyExecutionInstPair.second;
    auto exitCloneI = helixTask->getCloneOfOriginalInstruction(originalI);
    helixTask->addInstruction(originalI, loopBodyCloneI);
  }
  if (this->lastIterationExecutionBlock) {
    helixTask->addBasicBlock(originalSingleExitBlock, cloneLoopExitBlock);
  }

  /*
   * Link the parallelize code to the original one.
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "HELIX:  Linking task function\n";
  }
  this->addChunkFunctionExecutionAsideOriginalLoop(this->originalLDI, sequentialSegments.size());

  /*
   * Inline calls to HELIX functions.
   */
  this->inlineCalls(helixTask);

  /*
   * Print the HELIX task.
   */
  if (this->verbose >= Verbosity::Maximal) {
    helixTask->getTaskBody()->print(errs() << "HELIX:  Task code:\n"); errs() << "\n";
    // errs() << "HELIX: Exit\n";
    // SubCFGs execGraph(*helixTask->getTaskBody());
    // DGPrinter::writeGraph<SubCFGs, BasicBlock>("helixtask-loop" + std::to_string(LDI->getID()) + ".dot", &execGraph);
  }

  /*
   * Free the memory.
   */
  for (auto ss : sequentialSegments) {
    delete ss;
  }

  return true;
}

Function * HELIX::getTaskFunction (void) const {
  return tasks[0]->getTaskBody();
}

}
