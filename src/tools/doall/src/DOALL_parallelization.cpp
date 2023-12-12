/*
 * Copyright 2016 - 2023  Simone Campanoni
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
#include "noelle/core/ReductionSCC.hpp"
#include "noelle/core/InductionVariableSCC.hpp"
#include "noelle/tools/DOALL.hpp"
#include "noelle/tools/DOALLTask.hpp"

namespace arcana::noelle {

bool DOALL::apply(LoopContent *LDI, Heuristics *h) {

  /*
   * Check if DOALL is enabled.
   */
  if (!this->enabled) {
    return false;
  }

  /*
   * Fetch the headers.
   */
  auto loopStructure = LDI->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();
  auto loopPreHeader = loopStructure->getPreHeader();

  /*
   * Fetch the loop function.
   */
  auto loopFunction = loopStructure->getFunction();

  /*
   * Fetch the environment of the loop.
   */
  auto loopEnvironment = LDI->getEnvironment();
  assert(loopEnvironment != nullptr);

  /*
   * Fetch the maximum number of cores we can use for this loop.
   */
  auto ltm = LDI->getLoopTransformationsManager();
  auto maxCores = ltm->getMaximumNumberOfCores();

  /*
   * Print the parallelization request.
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DOALL: Start the parallelization\n";
    errs() << "DOALL:   Number of threads to extract = " << maxCores << "\n";
    errs() << "DOALL:   Chunk size = " << ltm->getChunkSize() << "\n";
  }

  /*
   * Define the signature of the task, which will be invoked by the DOALL
   * dispatcher.
   */
  auto tm = this->n.getTypesManager();
  auto funcArgTypes = ArrayRef<Type *>({ tm->getVoidPointerType(),
                                         tm->getIntegerType(64),
                                         tm->getIntegerType(64),
                                         tm->getIntegerType(64) });
  auto taskSignature =
      FunctionType::get(tm->getVoidType(), funcArgTypes, false);

  /*
   * Generate an empty task for the parallel DOALL execution.
   */
  auto doallTask = new DOALLTask(taskSignature, *this->n.getProgram());
  this->fromTaskIDToUserID[doallTask->getID()] = 0;
  this->addPredecessorAndSuccessorsBasicBlocksToTasks(LDI, { doallTask });
  this->numTaskInstances = maxCores;

  /*
   * Generate code to allocate and initialize the loop environment.
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DOALL:   Reduced variables:\n";
  }
  auto sccManager = LDI->getSCCManager();
  auto isReducible =
      [this, loopEnvironment, sccManager](uint32_t id, bool isLiveOut) -> bool {
    if (!isLiveOut) {
      return false;
    }

    /*
     * We have a live-out variable.
     *
     * Check if this is an IV.
     * IVs are not reducable because they get re-computed locally by each
     * thread.
     */
    auto producer = loopEnvironment->getProducer(id);
    auto scc = sccManager->getSCCDAG()->sccOfValue(producer);
    auto sccInfo = sccManager->getSCCAttrs(scc);
    if (isa<InductionVariableSCC>(sccInfo)) {

      /*
       * The current live-out variable is an induction variable.
       */
      return false;
    }

    /*
     * The current live-out variable is not an IV.
     * Because this loop is a DOALL, then this live-out variable must be
     * reducable (this is checked by the "canBeApplied" method).
     */
    if (this->verbose != Verbosity::Disabled) {
      errs() << "DOALL:     " << *producer << "\n";
    }

    return true;
  };
  auto isSkippable = [this, loopEnvironment, sccManager, doallTask](
                         uint32_t id,
                         bool isLiveOut) -> bool {
    if (isLiveOut) {
      return false;
    }

    /*
     * We have a live-in variable.
     *
     * We can avoid to propagate this live-in variable if its only purpose is to
     * propagate the initial value to a reduction variable. This is the case if
     * the following conditions are all met:
     * 1. This live-in variable only has one user within the loop, and
     * 2. This user is a PHI node, and
     * 3. The SCC that contains this PHI is a reduction variable.
     */
    auto producer = loopEnvironment->getProducer(id);
    if (producer->getNumUses() == 1) {
      if (auto consumer = dyn_cast<PHINode>(*producer->user_begin())) {
        auto scc = sccManager->getSCCDAG()->sccOfValue(consumer);
        auto sccInfo = sccManager->getSCCAttrs(scc);
        if (isa<ReductionSCC>(sccInfo)) {
          doallTask->addSkippedEnvironmentVariable(producer);
          return true;
        }
      }
    }

    return false;
  };
  this->initializeEnvironmentBuilder(LDI, isReducible, isSkippable);

  /*
   * Clone loop into the single task used by DOALL
   */
  this->cloneSequentialLoop(LDI, 0);
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DOALL:  Cloned loop\n";
  }

  /*
   * Load all loop live-in values at the entry point of the task.
   */
  auto envUser = this->envBuilder->getUser(0);
  assert(envUser != nullptr);
  for (auto envID : loopEnvironment->getEnvIDsOfLiveInVars()) {
    envUser->addLiveIn(envID);
  }
  for (auto envID : loopEnvironment->getEnvIDsOfLiveOutVars()) {
    envUser->addLiveOut(envID);
  }
  this->generateCodeToLoadLiveInVariables(LDI, 0);

  /*
   * This must follow loading live-ins as this re-wiring
   * overrides the live-in mapping to use locally cloned memory instructions
   * that are live-in to the loop
   */
  if (ltm->isOptimizationEnabled(LoopContentOptimization::MEMORY_CLONING_ID)) {
    this->cloneMemoryLocationsLocallyAndRewireLoop(LDI, 0);
  }
  doallTask->adjustDataAndControlFlowToUseClones();

  /*
   * Handle the reduction variables.
   */
  this->setReducableVariablesToBeginAtIdentityValue(LDI, 0);

  /*
   * Add the jump to start the loop from within the task.
   */
  auto headerClone = doallTask->getCloneOfOriginalBasicBlock(loopHeader);
  IRBuilder<> entryBuilder(doallTask->getEntry());
  entryBuilder.CreateBr(headerClone);

  /*
   * Perform the iteration-chunking optimization
   */
  this->rewireLoopToIterateChunks(LDI, doallTask);
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DOALL:  Rewired induction variables and reducible variables\n";
  }

  /*
   * Store final results to loop live-out variables. Note this occurs after
   * all other code is generated. Propagated PHIs through the generated
   * outer loop might affect the values stored
   */
  this->generateCodeToStoreLiveOutVariables(LDI, 0);
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DOALL:  Stored live outs\n";
  }

  /*
   * Add code to invoke the parallelized loop.
   */
  this->invokeParallelizedLoop(LDI);

  /*
   * Make PRVGs reentrant to avoid cache sharing.
   */
  auto com = this->noelle.getCompilationOptionsManager();
  if (com->arePRVGsNonDeterministic()) {
    errs() << "DOALL:  Make PRVGs reentrant\n";
    this->makePRVGsReentrant();
  }

  /*
   * Final printing.
   */
  if (this->verbose >= Verbosity::Maximal) {
    doallTask->getTaskBody()->print(errs()
                                    << "DOALL:  Final parallelized loop:\n");
    errs() << "\n";
  }
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DOALL: Exit\n";
  }

  return true;
}

} // namespace arcana::noelle
