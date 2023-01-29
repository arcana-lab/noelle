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
#include "noelle/core/ReductionSCC.hpp"
#include "noelle/core/InductionVariableSCC.hpp"
#include "noelle/tools/DOALL.hpp"
#include "noelle/tools/DOALLTask.hpp"

namespace llvm::noelle {

DOALL::DOALL(Noelle &noelle)
  : ParallelizationTechnique{ noelle },
    enabled{ true },
    taskDispatcher{ nullptr },
    n{ noelle } {

  /*
   * Fetch the dispatcher to use to jump to a parallelized DOALL loop.
   */
  this->taskDispatcher =
      this->n.getProgram()->getFunction("NOELLE_DOALLDispatcher");
  if (this->taskDispatcher == nullptr) {
    this->enabled = false;
    if (this->verbose != Verbosity::Disabled) {
      errs()
          << "DOALL: WARNING: function NOELLE_DOALLDispatcher couldn't be found. DOALL is disabled\n";
    }
  }

  return;
}

bool DOALL::canBeAppliedToLoop(LoopDependenceInfo *LDI, Heuristics *h) const {
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DOALL: Checking if the loop is DOALL\n";
  }

  /*
   * Fetch information about the loop.
   */
  auto loopStructure = LDI->getLoopStructure();
  auto loopEnv = LDI->getEnvironment();

  /*
   * The loop must have one single exit path.
   */
  auto numOfExits = 0;
  for (auto bb : loopStructure->getLoopExitBasicBlocks()) {

    /*
     * Fetch the last instruction before the terminator
     */
    auto terminator = bb->getTerminator();
    auto prevInst = terminator->getPrevNode();

    /*
     * Check if the last instruction is a call to a function that cannot return
     * (e.g., abort()).
     */
    if (prevInst == nullptr) {
      numOfExits++;
      continue;
    }
    if (auto callInst = dyn_cast<CallInst>(prevInst)) {
      auto callee = callInst->getCalledFunction();
      if (true && (callee != nullptr) && (callee->getName() == "exit")) {
        continue;
      }
    }
    numOfExits++;
  }
  if (numOfExits != 1) {
    if (this->verbose != Verbosity::Disabled) {
      errs() << "DOALL:   More than 1 loop exit blocks\n";
    }
    return false;
  }

  /*
   * The loop must have all live-out variables to be reducable.
   */
  auto sccManager = LDI->getSCCManager();
  auto nonReducibleLiveOuts =
      sccManager->getLiveOutVariablesThatAreNotReducable(loopEnv);
  if (nonReducibleLiveOuts.size() > 0) {
    if (this->verbose != Verbosity::Disabled) {
      errs() << "DOALL:   The next live-out variables are not reducable\n";
      for (auto envID : nonReducibleLiveOuts) {
        errs() << "DOALL:     Live-out ID = " << envID << "\n";
      }
    }
    return false;
  }

  /*
   * The compiler must be able to remove loop-carried data dependences of all
   * SCCs with loop-carried data dependences.
   */
  auto nonDOALLSCCs = DOALL::getSCCsThatBlockDOALLToBeApplicable(LDI, this->n);
  if (nonDOALLSCCs.size() > 0) {
    if (this->verbose != Verbosity::Disabled) {
      for (auto scc : nonDOALLSCCs) {
        errs()
            << "DOALL:   We found an SCC of the loop that is non clonable and non commutative\n";
        if (this->verbose >= Verbosity::Maximal) {

          /*
           * Print the SCC.
           */
          scc->printMinimal(errs(), "DOALL:     ");
          // DGPrinter::writeGraph<SCC, Value>("not-doall-loop-scc-" +
          // std::to_string(LDI->getID()) + ".dot", scc);

          /*
           * Print the loop-carried dependences between instructions of the SCC.
           */
          auto sccInfo = sccManager->getSCCAttrs(scc);
          if (auto loopCarriedSCC = dyn_cast<LoopCarriedSCC>(sccInfo)) {
            errs() << "DOALL:     Loop-carried data dependences\n";
            for (auto dep : loopCarriedSCC->getLoopCarriedDependences()) {
              if (dep->isControlDependence()) {
                continue;
              }
              auto fromInst = dep->getOutgoingT();
              auto toInst = dep->getIncomingT();
              errs() << "DOALL:       " << *fromInst << " ---> " << *toInst;
              if (dep->isMemoryDependence()) {
                errs() << " via memory\n";
              } else {
                errs() << " via variable\n";
              }
            }
          }
        }
      }
    }

    /*
     * There is at least one SCC that blocks DOALL to be applicable.
     */
    return false;
  }

  /*
   * The loop must have at least one induction variable.
   * This is because the trip count must be controlled by an induction variable.
   */
  auto loopGoverningIVAttr = LDI->getLoopGoverningIVAttribution();
  if (!loopGoverningIVAttr) {
    if (this->verbose != Verbosity::Disabled) {
      errs()
          << "DOALL:   Loop does not have an induction variable to control the number of iterations\n";
    }
    return false;
  }

  /*
   * NOTE: Due to a limitation in our ability to chunk induction variables,
   * all induction variables must have step sizes that are loop invariant
   */
  auto IVManager = LDI->getInductionVariableManager();
  for (auto IV : IVManager->getInductionVariables(*loopStructure)) {
    if (IV->isStepValueLoopInvariant()) {
      continue;
    }
    if (this->verbose != Verbosity::Disabled) {
      errs()
          << "DOALL:  Loop has an induction variable with step size that is not loop invariant\n";
    }
    return false;
  }

  /*
   * Check if the final value of the induction variable is a loop invariant.
   */
  auto invariantManager = LDI->getInvariantManager();
  LoopGoverningIVUtility ivUtility(loopStructure,
                                   *IVManager,
                                   *loopGoverningIVAttr);
  auto &derivation = ivUtility.getConditionValueDerivation();
  for (auto I : derivation) {
    if (!invariantManager->isLoopInvariant(I)) {
      if (this->verbose != Verbosity::Disabled) {
        errs()
            << "DOALL:  Loop has the governing induction variable that is compared against a non-invariant\n";
        errs() << "DOALL:     The non-invariant is = " << *I << "\n";
      }
      return false;
    }
  }

  /*
   * The loop is a DOALL one.
   */
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DOALL:   The loop can be parallelized with DOALL\n";
  }
  return true;
}

bool DOALL::apply(LoopDependenceInfo *LDI, Heuristics *h) {

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
  auto chunkerTask = new DOALLTask(taskSignature, *this->n.getProgram());
  this->addPredecessorAndSuccessorsBasicBlocksToTasks(LDI, { chunkerTask });
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
  auto isSkippable = [this, loopEnvironment, sccManager, chunkerTask](
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
          chunkerTask->addSkippedEnvironmentVariable(producer);
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
   * HACK: For now, this must follow loading live-ins as this re-wiring
   * overrides the live-in mapping to use locally cloned memory instructions
   * that are live-in to the loop
   */
  if (ltm->isOptimizationEnabled(
          LoopDependenceInfoOptimization::MEMORY_CLONING_ID)) {
    this->cloneMemoryLocationsLocallyAndRewireLoop(LDI, 0);
  }

  /*
   * Fix the data flow within the parallelized loop by redirecting operands of
   * cloned instructions to refer to the other cloned instructions. Currently,
   * they still refer to the original loop's instructions.
   */
  this->adjustDataFlowToUseClones(LDI, 0);
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DOALL:  Adjusted data flow\n";
  }

  /*
   * Handle the reduction variables.
   */
  this->setReducableVariablesToBeginAtIdentityValue(LDI, 0);

  /*
   * Add the jump to start the loop from within the task.
   */
  this->addJumpToLoop(LDI, chunkerTask);

  /*
   * Perform the iteration-chunking optimization
   */
  this->rewireLoopToIterateChunks(LDI);
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DOALL:  Rewired induction variables and reducible variables\n";
  }

  /*
   * Add the final return to the single task's exit block.
   */
  IRBuilder<> exitB(tasks[0]->getExit());
  exitB.CreateRetVoid();

  /*
   * Store final results to loop live-out variables. Note this occurs after
   * all other code is generated. Propagated PHIs through the generated
   * outer loop might affect the values stored
   */
  this->generateCodeToStoreLiveOutVariables(LDI, 0);

  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DOALL:  Stored live outs\n";
  }

  this->addChunkFunctionExecutionAsideOriginalLoop(LDI, loopFunction, this->n);

  /*
   * Final printing.
   */
  if (this->verbose >= Verbosity::Maximal) {
    // loopFunction->print(errs() << "DOALL:  Final outside-loop code:\n" );
    // errs() << "\n";
    tasks[0]->getTaskBody()->print(errs()
                                   << "DOALL:  Final parallelized loop:\n");
    errs() << "\n";
    // SubCFGs execGraph(*chunkerTask->getTaskBody());
    // DGPrinter::writeGraph<SubCFGs, BasicBlock>("doalltask-loop" +
    // std::to_string(LDI->getID()) + ".dot", &execGraph); SubCFGs
    // execGraph2(*loopFunction); DGPrinter::writeGraph<SubCFGs,
    // BasicBlock>("doall-loop-" + std::to_string(LDI->getID()) +
    // "-function.dot", &execGraph);
  }
  if (this->verbose != Verbosity::Disabled) {
    errs() << "DOALL: Exit\n";
  }

  return true;
}

void DOALL::addChunkFunctionExecutionAsideOriginalLoop(LoopDependenceInfo *LDI,
                                                       Function *loopFunction,
                                                       Noelle &par) {

  /*
   * Create the environment.
   */
  this->allocateEnvironmentArray(LDI);
  this->populateLiveInEnvironment(LDI);

  /*
   * Fetch the pointer to the environment.
   */
  auto envPtr = envBuilder->getEnvironmentArrayVoidPtr();

  /*
   * Fetch the number of cores
   */
  auto ltm = LDI->getLoopTransformationsManager();
  auto cm = par.getConstantsManager();
  auto numCores = cm->getIntegerConstant(ltm->getMaximumNumberOfCores(), 64);

  /*
   * Fetch the chunk size.
   */
  auto chunkSize = cm->getIntegerConstant(ltm->getChunkSize(), 64);

  /*
   * Call the function that incudes the parallelized loop.
   */
  IRBuilder<> doallBuilder(this->entryPointOfParallelizedLoop);
  auto doallCallInst = doallBuilder.CreateCall(
      this->taskDispatcher,
      ArrayRef<Value *>(
          { tasks[0]->getTaskBody(), envPtr, numCores, chunkSize }));
  auto numThreadsUsed =
      doallBuilder.CreateExtractValue(doallCallInst, (uint64_t)0);

  /*
   * Propagate the last value of live-out variables to the code outside the
   * parallelized loop.
   */
  auto latestBBAfterDOALLCall =
      this->performReductionToAllReducableLiveOutVariables(LDI, numThreadsUsed);

  /*
   * Jump to the unique successor of the loop.
   */
  IRBuilder<> afterDOALLBuilder{ latestBBAfterDOALLCall };
  afterDOALLBuilder.CreateBr(this->exitPointOfParallelizedLoop);

  return;
}

Value *DOALL::fetchClone(Value *original) const {
  auto task = this->tasks[0];
  if (isa<ConstantData>(original))
    return original;

  if (task->isAnOriginalLiveIn(original)) {
    return task->getCloneOfOriginalLiveIn(original);
  }

  assert(isa<Instruction>(original));
  auto iClone =
      task->getCloneOfOriginalInstruction(cast<Instruction>(original));
  assert(iClone != nullptr);
  return iClone;
}

void DOALL::addJumpToLoop(LoopDependenceInfo *LDI, Task *t) {

  /*
   * Fetch the header within the task.
   */
  auto loopStructure = LDI->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();
  auto headerClone = t->getCloneOfOriginalBasicBlock(loopHeader);

  /*
   * Add a jump to the loop within the task.
   */
  IRBuilder<> entryBuilder(t->getEntry());
  entryBuilder.CreateBr(headerClone);

  return;
}

uint32_t DOALL::getMinimumNumberOfIdleCores(void) const {
  return 2;
}

std::string DOALL::getName(void) const {
  return "DOALL";
}

} // namespace llvm::noelle
