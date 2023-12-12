/*
 * Copyright 2019 - 2021 Angelo Matni, Simone Campanoni
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
#include "EnablersManager.hpp"
#include "noelle/core/LoopCarriedUnknownSCC.hpp"
#include "noelle/tools/DOALL.hpp"

namespace arcana::noelle {

bool EnablersManager::applyEnablers(
    LoopContent *LDI,
    Noelle &par,
    LoopTransformer &LoopTransformer,
    LoopInvariantCodeMotion &loopInvariantCodeMotion,
    SCEVSimplification &scevSimplification) {

  /*
   * Apply loop distribution.
   */
  if (par.isTransformationEnabled(Transformation::LOOP_DISTRIBUTION_ID)) {
    errs() << "EnablersManager:     Try to apply loop distribution\n";
    if (this->applyLoopDistribution(LDI, par, LoopTransformer)) {
      errs() << "EnablersManager:       Distributed loop\n";
      return true;
    }
  }

  /*
   * Try to devirtualize functions.
   */
  if (par.isTransformationEnabled(Transformation::DEVIRTUALIZER_ID)) {
    errs() << "EnablersManager:     Try to devirtualize indirect calls\n";
    if (this->applyDevirtualizer(LDI, par, LoopTransformer)) {
      errs() << "EnablersManager:       Some calls have been devirtualized\n";
      return true;
    }
  }

  /*
   * Run the whilifier.
   */
  if (par.isTransformationEnabled(Transformation::LOOP_WHILIFIER_ID)) {
    errs() << "EnablersManager:     Try to whilify loops\n";
    if (this->applyLoopWhilifier(LDI, par, LoopTransformer)) {
      errs() << "EnablersManager:       The loop has been whilified\n";
      return true;
    }
  }

  /*
   * Run the extraction.
   */
  if (par.isTransformationEnabled(
          Transformation::LOOP_INVARIANT_CODE_MOTION_ID)) {
    errs() << "EnablersManager:     Try to extract invariants out of loops\n";
    if (loopInvariantCodeMotion.extractInvariantsFromLoop(*LDI)) {
      errs() << "EnablersManager:       Loop invariants have been extracted\n";
      return true;
    }
  }

  /*
   * Run the SCEV simplification pass
   */
  if (par.isTransformationEnabled(Transformation::SCEV_SIMPLIFICATION_ID)) {
    errs()
        << "EnablersManager:     Try to simplify IV related SCEVs and their corresponding instructions in loops\n";
    if (scevSimplification.simplifyIVRelatedSCEVs(*LDI)) {
      /*auto function = LDI->getLoopStructure()->getFunction();
        auto& SE = getAnalysis<ScalarEvolutionWrapperPass>(*function).getSE();
        if (scevSimplification.simplifyLoopGoverningIVGuards(*LDI, SE)){*/
      errs()
          << "EnablersManager:       Loop IV related SCEVs have been simplified\n";
      return true;
    }
    errs()
        << "EnablersManager:     Try to simplify constant SCEVs and their corresponding instructions in loops\n";
    if (scevSimplification.simplifyConstantPHIs(*LDI)) {
      errs()
          << "EnablersManager:       Loop constant PHIs have been simplified\n";
      return true;
    }
  }

  return false;
}

bool EnablersManager::applyLoopWhilifier(LoopContent *LDI,
                                         Noelle &par,
                                         LoopTransformer &LoopTransformer) {
  assert(LDI != nullptr);

  /*
   * Check if we need to whilify the loop.
   */
  auto loopStructure = LDI->getLoopStructure();
  auto IVManager = LDI->getInductionVariableManager();
  auto loopGoverningIVAttr = IVManager->getLoopGoverningInductionVariable();
  if ((loopGoverningIVAttr != nullptr)
      && (loopStructure->numberOfExitBasicBlocks() == 1)) {

    /*
     * The prologue is empty and the loop is in while form already.
     */
    return false;
  }
  DOALL doall{ par };
  if (doall.canBeAppliedToLoop(LDI, nullptr)) {

    /*
     * This is a DOALL loop, so the prologue is empty.
     */
    return false;
  }

  /*
   * Whilify the loop
   */
  auto modified = LoopTransformer.whilifyLoop(LDI);

  return modified;
}

bool EnablersManager::applyLoopDistribution(LoopContent *LDI,
                                            Noelle &par,
                                            LoopTransformer &loopTransformer) {

  /*
   * Fetch the SCC manager.
   */
  auto sccManager = LDI->getSCCManager();

  /*
   * Fetch the SCCDAG of the loop.
   */
  auto SCCDAG = sccManager->getSCCDAG();

  /*
   * Define the set of SCCs to bring outside the loop.
   */
  std::set<SCC *> SCCsToBringOutsideParallelizedLoop{};

  /*
   * Collect all sequential SCCs.
   */
  std::set<SCC *> sequentialSCCs{};
  auto collectSequentialSCCsFunction =
      [sccManager, &sequentialSCCs](SCC *currentSCC) -> bool {
    /*
     * Fetch the SCC metadata.
     */
    auto sccInfo = sccManager->getSCCAttrs(currentSCC);

    /*
     * Check if the current SCC can be removed (e.g., because it is due to
     * induction variables). If it is, then we do not need to remove it from the
     * loop to be parallelized.
     */
    if (!isa<LoopCarriedUnknownSCC>(sccInfo)) {
      return false;
    }

    /*
     * The current SCC must run sequentially.
     */
    sequentialSCCs.insert(currentSCC);
    return false;
  };
  SCCDAG->iterateOverSCCs(collectSequentialSCCsFunction);

  /*
   * Check every sequential SCC of the loop and decide which ones to bring
   * outside the loop to parallelize.
   */
  for (auto SCC : sequentialSCCs) {

    /*
     * Try to bring the sequential SCC outside the loop.
     */
    std::set<Instruction *> instsRemoved;
    std::set<Instruction *> instsAdded;
    auto splitted =
        loopTransformer.splitLoop(LDI, { SCC }, instsRemoved, instsAdded);
    if (!splitted) {
      continue;
    }

    /*
     * The SCC has been pulled out the loop.
     * We need to update all metadata about loops.
     * To do so, we currently quit and rerun noelle-enable
     */
    return true;
  }

  return false;
}

bool EnablersManager::applyDevirtualizer(LoopContent *LDI,
                                         Noelle &par,
                                         LoopTransformer &lt) {

  /*
   * We want to fully unroll a loop if this can help the parallelization of an
   * outer loop that includes it.
   *
   * One condition that allow this improvement is when the inner loop iterates
   * over function pointers. For example: while (...){ for (auto i=0; i < 10;
   * i++){ auto functionPtr = array[i];
   *      (*functionPtr)(...)
   *    }
   * }
   */

  /*
   * Fetch the loop information.
   */
  auto ls = LDI->getLoopStructure();
  auto IVM = LDI->getInductionVariableManager();

  /*
   * Check if the loop includes at least one call to unknown callee.
   */
  auto fullyUnroll = false;
  for (auto bb : ls->getBasicBlocks()) {
    for (auto &inst : *bb) {

      /*
       * Check if the instruction is a call one.
       */
      if (!isa<CallInst>(&inst)) {
        continue;
      }
      auto callInst = cast<CallInst>(&inst);

      /*
       * Check if the callee is known.
       */
      auto callee = callInst->getCalledFunction();
      if (callee != nullptr) {
        continue;
      }

      /*
       * The callee is unknown.
       *
       * Check if the callee is determined by the result of a load instruction.
       */
      auto calleePtr = callInst->getCalledOperand();
      if (!isa<LoadInst>(calleePtr)) {
        continue;
      }
      auto loadInst = cast<LoadInst>(calleePtr);

      /*
       * Check if the address of the load instruction is the result of a GEP.
       */
      auto addr = loadInst->getPointerOperand();
      if (!isa<GetElementPtrInst>(addr)) {
        continue;
      }
      auto addrComputation = cast<GetElementPtrInst>(addr);

      /*
       * Check if the GEP relies on an induction variable.
       */
      for (auto &idx : addrComputation->indices()) {
        if (!isa<Instruction>(idx)) {
          continue;
        }
        auto instIdx = cast<Instruction>(idx);
        if (IVM->doesContributeToComputeAnInductionVariable(instIdx)) {
          fullyUnroll = true;
          break;
        }
      }
      if (fullyUnroll) {
        break;
      }
    }
  }
  if (!fullyUnroll) {
    return false;
  }

  /*
   * Fully unroll the loop.
   */
  auto modified = lt.fullyUnrollLoop(LDI);

  return modified;
}

} // namespace arcana::noelle
