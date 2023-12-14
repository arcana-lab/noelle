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
#include "noelle/tools/DSWP.hpp"
#include "noelle/core/Architecture.hpp"
#include "noelle/core/LoopIterationSCC.hpp"

namespace arcana::noelle {

DSWP::DSWP(Noelle &n, bool forceParallelization, bool enableSCCMerging)
  : ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences{ n,
                                                                    forceParallelization },
    minCores{ 0 },
    enableMergingSCC{ enableSCCMerging },
    queues{},
    queueArrayType{ nullptr },
    sccToStage{},
    stageArrayType{ nullptr },
    zeroIndexForBaseArray{ nullptr } {

  /*
   * Fetch the function that dispatch the parallelized loop.
   */
  auto program = this->noelle.getProgram();
  this->taskDispatcher = program->getFunction("NOELLE_DSWPDispatcher");
  assert(this->taskDispatcher != nullptr);

  return;
}

bool DSWP::canBeAppliedToLoop(LoopContent *LDI, Heuristics *h) const {

  /*
   * Check the parent class.
   */
  if (!ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::
          canBeAppliedToLoop(LDI, h)) {
    return false;
  }

  /*
   * Fetch the profiles
   */
  auto profiles = this->noelle.getProfiles();

  /*
   * Check if there is at least one sequential stage.
   * If there isn't, then this loop is a DOALL. Hence, DSWP is not applicable.
   *
   * Also, compute the coverage of the biggest stage.
   * If the coverage is too high, then the parallelization isn't worth it.
   */
  auto doesSequentialSCCExist = false;
  uint64_t biggestSCC = 0;
  auto sccManager = LDI->getSCCManager();
  auto clonableSCCs =
      this->getClonableSCCs(sccManager, LDI->getLoopHierarchyStructures());
  for (auto nodePair : sccManager->getSCCDAG()->internalNodePairs()) {

    /*
     * Fetch the current SCC.
     */
    auto currentSCC = nodePair.first;
    auto currentSCCInfo = sccManager->getSCCAttrs(currentSCC);

    /*
     * Check the coverage of the SCC.
     */
    auto currentSCCTotalInsts = profiles->getTotalInstructions(currentSCC);
    if (currentSCCTotalInsts > biggestSCC) {
      biggestSCC = currentSCCTotalInsts;
    }
    assert(biggestSCC >= currentSCCTotalInsts);

    /*
     * Check if this will run sequentially.
     */
    if (clonableSCCs.find(currentSCCInfo) != clonableSCCs.end()) {
      continue;
    }

    /*
     * We found a sequential stage.
     */
    doesSequentialSCCExist = true;
  }

  /*
   * If there isn't a sequential SCC, then this loop is a DOALL. Hence, DSWP is
   * not applicable.
   */
  if (!doesSequentialSCCExist) {
    errs()
        << "DSWP: It is not applicable because the loop doesn't have a sequential SCC\n";
    return false;
  }

  /*
   * Check if we are forced to parallelize
   */
  if (this->forceParallelization) {

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
  if (biggestSCCCoverage >= 0.8) {

    /*
     * The pipeline would be too imbalance.
     */
    errs()
        << "DSWP: It is not applicable because the coverage of the biggest SCC is "
        << biggestSCCCoverage << "\n";
    return false;
  }

  /*
   * Ensure there is not too little execution that is too proportionally
   * iteration-independent for DSWP
   */
  auto loopStructure = LDI->getLoopStructure();
  auto loopIDOpt = loopStructure->getID();
  assert(loopIDOpt); // ED: we are potentially parallelizing with DSWP, loops
                     // should have IDs.
  auto loopID = loopIDOpt.value();
  auto averageInstructions =
      profiles->getAverageTotalInstructionsPerIteration(loopStructure);
  auto averageInstructionThreshold = 20;
  bool hasLittleExecution = averageInstructions < averageInstructionThreshold;
  auto minimumSequentialFraction = .5;
  auto skipSCC = [this, sccManager](GenericSCC *scc) -> bool {
    auto skip = this->canBeCloned(scc);
    return skip;
  };
  auto sequentialFraction =
      this->computeSequentialFractionOfExecution(LDI, skipSCC);
  bool hasProportionallyInsignificantSequentialExecution =
      sequentialFraction < minimumSequentialFraction;
  if (hasLittleExecution && hasProportionallyInsignificantSequentialExecution) {
    errs()
        << "Parallelizer:    Loop " << loopID << " has " << averageInstructions
        << " number of sequential instructions on average per loop iteration\n";
    errs()
        << "Parallelizer:    Loop " << loopID << " has " << sequentialFraction
        << " % sequential execution per loop iteration\n";
    errs()
        << "Parallelizer:      It will not be partitioned enough for DSWP. The thresholds are at least "
        << averageInstructionThreshold
        << " instructions per iteration or at least "
        << minimumSequentialFraction << " % sequential execution."
        << "\n";

    return false;
  }

  /*
   * DSWP is applicable.
   */
  return true;
}

bool DSWP::apply(LoopContent *LDI, Heuristics *h) {

  /*
   * Start.
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DSWP: Start\n";
  }

  /*
   * Fetch the managers.
   */
  auto cm = this->noelle.getConstantsManager();
  auto tm = this->noelle.getTypesManager();

  /*
   * Compute the set of SCCs that can be cloned.
   */
  this->clonableSCCs = this->getClonableSCCs(LDI->getSCCManager(),
                                             LDI->getLoopHierarchyStructures());

  /*
   * Fetch the header.
   */
  auto loopSummary = LDI->getLoopStructure();
  auto loopHeader = loopSummary->getHeader();
  auto loopFunction = loopHeader->getParent();
  this->originalFunctionDS = this->noelle.getDominators(loopFunction);

  /*
   * Partition the SCCDAG.
   */
  this->partitionSCCDAG(LDI, h);

  /*
   * Check if the parallelization is worth it.
   */
  if (!this->forceParallelization
      && this->partitioner->numberOfPartitions() == 1) {

    /*
     * The parallelization isn't worth it as there is only one pipeline stage.
     */
    if (this->verbose != Verbosity::Disabled) {
      errs()
          << "DSWP:  There is only 1 partition and therefore the parallelization isn't worth it.\n";
    }

    return false;
  }
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DSWP:  There are " << this->partitioner->numberOfPartitions()
           << " partitions in the SCCDAG\n";
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
  collectDataAndMemoryQueueInfo(LDI, this->noelle);
  collectControlQueueInfo(LDI, this->noelle);
  // assert(areQueuesAcyclical());
  // writeStageQueuesAsDot(*LDI);

  /*
   * Generate code to allocate and initialize the loop environment.
   */
  auto isReducible = [](uint32_t id, bool isLiveOut) -> bool { return false; };
  this->initializeEnvironmentBuilder(LDI, isReducible);
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
  this->zeroIndexForBaseArray = cm->getIntegerConstant(0, 64);
  auto int8Type = tm->getIntegerType(8);
  this->queueArrayType =
      ArrayType::get(PointerType::getUnqual(int8Type), this->queues.size());
  this->stageArrayType =
      ArrayType::get(PointerType::getUnqual(int8Type), this->tasks.size());

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
     * Load pointers of all queues for the current pipeline stage at the
     * function's entry
     */
    generateLoadsOfQueuePointers(this->noelle, i);
    if (this->verbose >= Verbosity::Maximal) {
      errs() << "DSWP:  Loaded queue pointers\n";
    }

    /*
     * Add push/pop operations from queues between the current pipeline stage
     * and the connected ones
     */
    popValueQueues(LDI, this->noelle, i);
    pushValueQueues(LDI, this->noelle, i);
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

    /*
     * HACK: For now, this must follow loading live-ins as this re-wiring
     * overrides the live-in mapping to use locally cloned memory instructions
     * that are live-in to the loop
     */
    auto ltm = LDI->getLoopTransformationsManager();
    if (ltm->isOptimizationEnabled(
            LoopContentOptimization::MEMORY_CLONING_ID)) {
      this->cloneMemoryLocationsLocallyAndRewireLoop(LDI, i);
    }

    /*
     * Fix the data flow within the parallelized loop by redirecting operands of
     * cloned instructions to refer to the other cloned instructions. Currently,
     * they still refer to the original loop's instructions.
     */
    task->adjustDataAndControlFlowToUseClones();

    /*
     * Add the unconditional branch from the entry basic block to the header of
     * the loop.
     */
    IRBuilder<> entryBuilder(task->getEntry());
    entryBuilder.CreateBr(task->getCloneOfOriginalBasicBlock(loopHeader));

    /*
     * Store final results to loop live-out variables.
     * Generate a store to propagate the information about which exit block has
     * been taken from the parallelized loop to the code outside it.
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
      task->getTaskBody()->print(errs() << "Pipeline stage " << i << ":\n");
      errs() << "\n";
      // SubCFGs execGraph(*task->getTaskBody());
      // std::string name = "dswp-task-" + std::to_string(task->getID()) +
      // "-loop-" + std::to_string(LDI->getID()) + ".dot";
      // DGPrinter::writeGraph<SubCFGs, BasicBlock>(name , &execGraph);
    }
  }

  /*
   * Create the whole pipeline by connecting the stages.
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DSWP:  Link pipeline stages\n";
  }
  createPipelineFromStages(LDI, this->noelle);

  delete this->originalFunctionDS;

  /*
   * Set the minimum number of cores.
   */
  this->minCores = this->tasks.size();

  /*
   * Exit
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DSWP: Exit\n";
  }
  return true;
}

uint32_t DSWP::getMinimumNumberOfIdleCores(void) const {
  return Architecture::getNumberOfPhysicalCores();
  return this->minCores;
}

std::string DSWP::getName(void) const {
  return "DSWP";
}

Transformation DSWP::getParallelizationID(void) const {
  return Transformation::DSWP_ID;
}

} // namespace arcana::noelle
