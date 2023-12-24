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
#include "noelle/tools/DOALL.hpp"
#include "noelle/core/InductionVariableSCC.hpp"

namespace arcana::noelle {

bool DOALL::canBeAppliedToLoop(LoopContent *LDI, Heuristics *h) const {
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
      if ((callee != nullptr) && (callee->getName() == "exit")) {
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
  auto sccdag = sccManager->getSCCDAG();
  auto nonReducibleLiveOuts =
      sccManager->getLiveOutVariablesThatAreNotReducable(loopEnv);
  std::set<uint32_t> liveOutThatRequireSynchronizations{};
  for (auto liveOutVar : nonReducibleLiveOuts) {

    /*
     * Fetch the SCC that generates the live-out variable.
     */
    auto producer = loopEnv->getProducer(liveOutVar);
    auto scc = sccdag->sccOfValue(producer);
    auto sccInfo = sccManager->getSCCAttrs(scc);
    assert(sccInfo != nullptr);

    /*
     * Check if the SCC can be handled by DOALL.
     */
    if (isa<InductionVariableSCC>(sccInfo)) {
      continue;
    }

    /*
     * The SCC cannot be handled by DOALL.
     */
    liveOutThatRequireSynchronizations.insert(liveOutVar);
  }
  if (liveOutThatRequireSynchronizations.size() > 0) {
    if (this->verbose != Verbosity::Disabled) {
      errs()
          << "DOALL:   The next live-out variables require synchronizations between loop iterations\n";
      for (auto envID : liveOutThatRequireSynchronizations) {
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

          /*
           * Print the loop-carried dependences between instructions of the SCC.
           */
          auto sccInfo = sccManager->getSCCAttrs(scc);
          if (auto loopCarriedSCC = dyn_cast<LoopCarriedSCC>(sccInfo)) {
            errs() << "DOALL:     Loop-carried data dependences\n";
            for (auto dep : loopCarriedSCC->getLoopCarriedDependences()) {
              if (isa<ControlDependence<Value, Value>>(dep)) {
                continue;
              }
              auto fromInst = dep->getSrc();
              auto toInst = dep->getDst();
              errs() << "DOALL:       " << *fromInst << " ---> " << *toInst;
              if (isa<MemoryDependence<Value, Value>>(dep)) {
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
  auto IVManager = LDI->getInductionVariableManager();
  auto loopGoverningIVAttr = IVManager->getLoopGoverningInductionVariable();
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

} // namespace arcana::noelle
