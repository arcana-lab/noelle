/*
 * Copyright 2016 - 2023  Angelo Matni, Simone Campanoni
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
#include "noelle/core/Architecture.hpp"
#include "noelle/core/ReductionSCC.hpp"
#include "noelle/tools/HELIX.hpp"
#include "noelle/tools/DOALL.hpp"

namespace arcana::noelle {

bool HELIX::apply(LoopContent *LDI, Heuristics *h) {

  /*
   * Print the LDI
   */
  if (this->verbose != Verbosity::Disabled) {
    auto prefixStringWithIndentation = std::string{ this->prefixString };
    prefixStringWithIndentation.append("  ");

    /*
     * Print the loop.
     */
    auto ls = LDI->getLoopStructure();
    auto bbs = ls->getBasicBlocks();
    errs() << prefixStringWithIndentation << "Loop has " << bbs.size()
           << " basic blocks:\n";
    for (auto bb : bbs) {
      errs() << *bb << "\n";
    }

    /*
     * Print the sequential code.
     */
    this->printSequentialCode(
        errs(),
        prefixStringWithIndentation,
        LDI,
        DOALL::getSCCsThatBlockDOALLToBeApplicable(LDI, this->noelle));
  }

  /*
   * Create the HELIX task from the original loop without synchronizations
   * between its dynamic instances.
   */
  auto helixTask = this->createParallelizableTask(LDI, h);

  /*
   * Add synchronizations into the HELIX task code.
   */
  auto taskFunctionDG =
      this->constructTaskInternalDependenceGraphFromOriginalLoopDG(LDI);
  auto header = LDI->getLoopStructure()->getHeader();
  auto headerClone = helixTask->getCloneOfOriginalBasicBlock(header);
  assert(headerClone != nullptr);
  auto newLDI =
      this->noelle.getLoopContent(headerClone,
                                  taskFunctionDG,
                                  LDI->getLoopTransformationsManager(),
                                  false);
  auto modified = this->synchronizeTask(newLDI, h, helixTask);

  return modified;
}

HELIXTask *HELIX::createParallelizableTask(LoopContent *LDI, Heuristics *h) {

  /*
   * Check if we have the APIs available.
   */
  if (!this->waitSSCall || !this->signalSSCall) {
    errs()
        << this->prefixString
        << "ERROR = sync functions HELIX_wait, HELIX_signal were not both found.\n";
    abort();
  }

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
   * Keep around the original loops' LoopContent.
   * This is necessary because a new LDI will be generated after spilling
   * loop-carried variables and we will need to map original SCCs to the new
   * SCCs of the new LDI.
   */
  this->originalLDI = LDI;

  /*
   * Print the parallelization request.
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << this->prefixString << "Start the parallelization\n";

    /*
     * Print the prologue
     */
    if (this->doesHaveASequentialPrologue(LDI)) {
      errs() << this->prefixString
             << "    The loop will have a sequential prologue\n";
    }
  }

  /*
   * Compute reachability so that determining whether spill loads placed in loop
   * exit blocks could be invalidated by spill stores in the loop. If so,
   * they will have to be placed within the loop (which is less optimal)
   * NOTE: This is computed BEFORE addPredecessorAndSuccessorsBasicBlocksToTasks
   * creates an empty basic block in the original function which will be used to
   * link this task
   */
  auto reachabilityDFR = this->computeReachabilityFromInstructions(LDI);

  /*
   * Define the signature of the task, which will be invoked by the HELIX
   * dispatcher.
   */
  auto tm = noelle.getTypesManager();
  auto int8 = tm->getIntegerType(8);
  auto int64 = tm->getIntegerType(64);
  auto ptrType = tm->getVoidPointerType();
  auto voidType = tm->getVoidType();
  auto funcArgTypes = ArrayRef<Type *>({ ptrType,
                                         ptrType,
                                         ptrType,
                                         ptrType,
                                         int64,
                                         int64,
                                         PointerType::getUnqual(int64) });
  auto taskSignature = FunctionType::get(voidType, funcArgTypes, false);

  /*
   * Generate empty tasks for the HELIX execution.
   */
  auto program = this->noelle.getProgram();
  auto helixTask = new HELIXTask(taskSignature, *program);
  this->fromTaskIDToUserID[helixTask->getID()] = 0;
  this->addPredecessorAndSuccessorsBasicBlocksToTasks(LDI, { helixTask });
  auto ltm = LDI->getLoopTransformationsManager();
  this->numTaskInstances = ltm->getMaximumNumberOfCores();

  /*
   * Fetch the environment of the loop
   */
  auto environment = LDI->getEnvironment();
  assert(environment != nullptr);

  /*
   * Generate code to allocate and initialize the loop environment.
   */
  errs() << this->prefixString << "  Initialize the environment of the loop\n";
  auto isReducible = [this, environment, sccManager](uint32_t id,
                                                     bool isLiveOut) -> bool {
    if (!isLiveOut) {
      return false;
    }

    /*
     * We have a live-out variable.
     *
     * Check if it can be reduced so we can generate more efficient code that
     * does not require a sequential segment.
     */
    auto producer = environment->getProducer(id);
    auto scc = sccManager->getSCCDAG()->sccOfValue(producer);
    auto sccInfo = sccManager->getSCCAttrs(scc);
    if (isa<ReductionSCC>(sccInfo)) {
      errs()
          << this->prefixString
          << "    The following variable is reducable: " << *producer << "\n";
      auto s = this->prefixString;
      s.append("      ");
      scc->print(errs(), s);
      return true;
    }

    return false;
  };
  auto isSkippable = [this, environment, sccManager, helixTask](
                         uint32_t id,
                         bool isLiveOut) -> bool {
    if (isLiveOut) {
      return false;
    }

    /*
     * We have a live-in variable.
     *
     * The initial value of the reduction variable can be skipped,
     * which means the following conditions should all meet
     * 1. This live-in variable only has one user, and
     * 2. The user is a phi node, and
     * 3. The scc contains this phi is not part of the induction variable but
     * reducible operation
     */
    auto producer = environment->getProducer(id);
    if (producer->getNumUses() == 1) {
      if (auto consumer = dyn_cast<PHINode>(*producer->user_begin())) {
        auto scc = sccManager->getSCCDAG()->sccOfValue(consumer);
        auto sccInfo = sccManager->getSCCAttrs(scc);
        if (isa<ReductionSCC>(sccInfo)) {
          helixTask->addSkippedEnvironmentVariable(producer);
          return true;
        }
      }
    }

    return false;
  };
  this->initializeEnvironmentBuilder(LDI, isReducible, isSkippable);

  /*
   * Clone the sequential loop and store the cloned instructions/basic blocks
   * within the single task of HELIX.
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << this->prefixString << "  Cloning loop in task\n";
  }
  this->cloneSequentialLoop(LDI, 0);

  /*
   * Load all loop live-in values at the entry point of the task.
   * Store final results to loop live-out variables.
   */
  auto envUser = this->envBuilder->getUser(0);
  for (auto envID : environment->getEnvIDsOfLiveInVars()) {
    envUser->addLiveIn(envID);
  }
  for (auto envID : environment->getEnvIDsOfLiveOutVars()) {
    envUser->addLiveOut(envID);
  }
  this->generateCodeToLoadLiveInVariables(LDI, 0);

  /*
   * HACK: For now, this must follow loading live-ins as this re-wiring
   * overrides the live-in mapping to use locally cloned memory instructions
   * that are live-in to the loop
   */
  if (ltm->isOptimizationEnabled(LoopContentOptimization::MEMORY_CLONING_ID)) {
    this->cloneMemoryLocationsLocallyAndRewireLoop(LDI, 0);
  }

  /*
   * The operands of the cloned instructions still refer to the original ones.
   *
   * Fix the data flow within the parallelized loop by redirecting operands of
   * cloned instructions to refer to the other cloned instructions.
   */
  helixTask->adjustDataAndControlFlowToUseClones();

  /*
   * Set the initial value of the private versions of the reducable variables to
   * their identity value.
   */
  this->setReducableVariablesToBeginAtIdentityValue(LDI, 0);

  /*
   * Add the unconditional branch from the entry basic block to the header of
   * the loop.
   */
  IRBuilder<> entryBuilder(helixTask->getEntry());
  entryBuilder.CreateBr(helixTask->getCloneOfOriginalBasicBlock(loopHeader));

  /*
   * Spill loop carried dependencies into a separate environment array
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs()
        << this->prefixString
        << "  Check if we need to spill variables because they are part of loop carried data dependencies\n";
  }
  this->spillLoopCarriedDataDependencies(LDI, reachabilityDFR, helixTask);

  /*
   * For IVs that were not spilled, adjust their step size appropriately
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << this->prefixString << "  Adjusting loop IVs\n";
  }
  this->rewireLoopForIVsToIterateNthIterations(LDI);
  this->rewireLoopForPeriodicVariables(LDI);
  /*
   * Delete reachability results
   */
  delete reachabilityDFR;

  return helixTask;
}

bool HELIX::synchronizeTask(LoopContent *LDI,
                            Heuristics *h,
                            HELIXTask *helixTask) {
  assert(LDI != nullptr);
  assert(h != nullptr);
  assert(helixTask != nullptr);

  /*
   * Compute reachability analysis for computing SS frontiers and scheduling SS
   * instructions
   */
  auto reachabilityDFR = this->computeReachabilityFromInstructions(LDI);

  /*
   * Schedule the code to minimize the instructions within each sequential
   * segment. HACK: Entries and exits are determined when identifying a
   * sequential segment. They aren't adjusted after squeezing. Delay computing
   * entry and exit frontiers for identified sequential segments until AFTER
   * squeezing.
   */
  auto sequentialSegments = this->identifySequentialSegments(this->originalLDI,
                                                             LDI,
                                                             reachabilityDFR,
                                                             helixTask);
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
    errs() << this->prefixString << "  Identifying sequential segments\n";
  }
  reachabilityDFR = this->computeReachabilityFromInstructions(LDI);
  sequentialSegments = this->identifySequentialSegments(this->originalLDI,
                                                        LDI,
                                                        reachabilityDFR,
                                                        helixTask);

  /*
   * Schedule the sequential segments to overlap parallel and sequential
   * segments.
   */
  this->scheduleSequentialSegments(LDI, &sequentialSegments, reachabilityDFR);

  /*
   * Delete reachability results here before we decide whether to continue with
   * the HELIX parallelization
   */
  delete reachabilityDFR;

  /*
   * Check if any sequential segment's entry and exit frontier spans the entire
   * loop execution If so, do not parallelize
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
      if (!entryAtHeader || !exitAtLatch)
        continue;

      /*
       * The HELIX parallelization isn't worth it.
       */
      if (this->verbose != Verbosity::Disabled) {
        errs()
            << "HELIX: There is a sequential segment spanning the entire loop; therefore, the parallelization isn't worth it.\n";
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
  this->addSynchronizations(LDI, &sequentialSegments, helixTask);

  /*
   * Store final results of loop live-out variables.
   *
   * Note this occurs after synchronization has been put in place. This is to
   * ensure that reducible variables not tracked in the loop carried environment
   * are properly propagated as live outs even when check-exit fails and
   * branches directly to the task function's exit block (it can't branch to the
   * task loop's exit blocks because logic in those exit blocks should only be
   * executed by the last iteration, not by all cores)
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "HELIX:  Storing live out variables and exit block index\n";
  }

  /*
   * HACK: swap the mapping so that original loop instructions map to possible
   * duplicates that were created should the original header have had sequential
   * instructions that were moved to the loop body and duplicated after exiting
   * the loop
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
   * The assumption being made here is that if we have a last iteration
   * execution block, it is because the loop has a loop governing IV. Our
   * attribution relies on there being only one loop exit that is controlled by
   * an IV. Hence, we fetch the unique exit block.
   */
  auto originalExitBlocks =
      this->originalLDI->getLoopStructure()->getLoopExitBasicBlocks();
  auto originalSingleExitBlock = *originalExitBlocks.begin();
  BasicBlock *cloneLoopExitBlock = nullptr;
  if (this->lastIterationExecutionBlock) {
    assert(originalExitBlocks.size() == 1
           && "loop governing IV attribution relies on only one exit block!");
    cloneLoopExitBlock =
        helixTask->getCloneOfOriginalBasicBlock(originalSingleExitBlock);
    helixTask->addBasicBlock(originalSingleExitBlock,
                             lastIterationExecutionBlock);
  }

  /*
   * Generate stores for live out variables
   * Generate a store to propagate information about which exit block the
   * parallelized loop took.
   */
  this->generateCodeToStoreLiveOutVariables(this->originalLDI, 0);
  this->generateCodeToStoreExitBlockIndex(this->originalLDI, 0);

  /*
   * HACK: reset the last clone map to reflect the loop exit block which is the
   * successor to the if else branch determining whether to execute the last
   * iteration block before the loop exit block
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
  this->invokeParallelizedLoop(this->originalLDI, sequentialSegments.size());

  /*
   * Inline calls to HELIX functions.
   */
  this->inlineCalls(helixTask);

  /*
   * Make PRVGs reentrant to avoid cache sharing.
   */
  auto com = this->noelle.getCompilationOptionsManager();
  if (com->arePRVGsNonDeterministic()) {
    errs() << "HELIX:  Make PRVGs reentrant\n";
    this->makePRVGsReentrant();
  }

  /*
   * Print the HELIX task.
   */
  if (this->verbose >= Verbosity::Maximal) {
    helixTask->getTaskBody()->print(errs() << "HELIX:  Task code:\n");
    errs() << "\n";
  }

  /*
   * Free the memory.
   */
  for (auto ss : sequentialSegments) {
    delete ss;
  }

  return true;
}

} // namespace arcana::noelle
