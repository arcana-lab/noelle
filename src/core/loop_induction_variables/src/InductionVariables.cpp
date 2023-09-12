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
#include "noelle/core/InductionVariables.hpp"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

namespace llvm::noelle {

InductionVariableManager::InductionVariableManager(LoopTree *loopNode,
                                                   InvariantManager &IVM,
                                                   ScalarEvolution &SE,
                                                   SCCDAG &sccdag,
                                                   LoopEnvironment &loopEnv,
                                                   Loop &LLVMLoop)
  : loop{ loopNode },
    loopToIVsMap{},
    loopToGoverningIVAttrMap{} {
  assert(this->loop != nullptr);

  /*
   * Fetch the loop to analyze.
   */
  auto loopToAnalyze = this->loop->getLoop();
  assert(loopToAnalyze != nullptr);

  /*
   * Fetch the function that includes the loop.
   */
  auto &F = *loopToAnalyze->getHeader()->getParent();

  /*
   * Identify the induction variables.
   */
  ScalarEvolutionReferentialExpander referentialExpander(SE, F);
  for (auto loop : this->loop->getLoops()) {
    this->loopToIVsMap[loop] = std::unordered_set<InductionVariable *>();

    /*
     * Fetch the loop header and pre-header.
     */
    auto header = loop->getHeader();
    auto preHeader = loop->getPreHeader();

    /*
     * Iterate over all phis within the loop header.
     */
    for (auto &phi : header->phis()) {

      /*
       * Check if LLVM considers this PHI to be an induction variable
       */
      InductionDescriptor ID{};
      auto llvmDeterminedValidIV = false;
      auto llvmLoopValidForInductionAnalysis =
          (phi.getBasicBlockIndex(preHeader) >= 0);
      if (llvmLoopValidForInductionAnalysis
          && InductionDescriptor::isInductionPHI(&phi, &LLVMLoop, &SE, ID)) {
        llvmDeterminedValidIV = true;

      } else if (phi.getType()->isFloatingPointTy()
                 && InductionDescriptor::isFPInductionPHI(&phi,
                                                          &LLVMLoop,
                                                          &SE,
                                                          ID)) {
        llvmDeterminedValidIV = true;
      }

      /*
       * Check if NOELLE considers this PHI an induction variable.
       *
       * First, let's check if the PHI node can be analyzed by the SCEV
       * analysis.
       */
      auto sccContainingIV = sccdag.sccOfValue(&phi);
      auto noelleDeterminedValidIV = true;
      auto stepMultiplier = 0;
      InductionVariable *IV = nullptr;
      if (!SE.isSCEVable(phi.getType())) {
        noelleDeterminedValidIV = false;
      } else {

        /*
         * Fetch the SCEV and check if it suggests this is an induction
         * variable.
         */
        auto scev = SE.getSCEV(&phi);
        if (!scev) {
          noelleDeterminedValidIV = false;
        } else if (scev->getSCEVType() != SCEVTypes::scAddRecExpr) {
          noelleDeterminedValidIV = false;

          PHINode *internalPHI;
          auto found = false;
          for (auto internalNode = sccContainingIV->begin_internal_node_map();
               internalNode != sccContainingIV->end_internal_node_map();
               internalNode++) {
            if (isa<PHINode>(internalNode->first)
                && internalNode->first != &phi) {
              internalPHI = cast<PHINode>(internalNode->first);
              auto internalSCEV = SE.getSCEV(internalPHI);
              if (internalSCEV->getSCEVType() == SCEVTypes::scAddRecExpr) {
                auto internalAddRecExpr = cast<SCEVAddRecExpr>(internalSCEV);

                if (this->loop->isIncludedInItsSubLoops(internalPHI)) {
                  found = true;
                  break;
                }
              }
            }
          }
          if (found) {
            auto innerHeaderTerminator =
                cast<BranchInst>(internalPHI->getParent()->getTerminator());
            if (innerHeaderTerminator->getNumSuccessors() == 2
                && loop->getLatches().find(
                       innerHeaderTerminator->getSuccessor(0))
                       != loop->getLatches().end()) {
              auto innerExitCond =
                  cast<Instruction>(innerHeaderTerminator->getCondition());
              if (isa<ConstantInt>(
                      innerExitCond->getOperand(1))) { // TODO: generalize
                auto innerLoopIterations =
                    cast<ConstantInt>(innerExitCond->getOperand(1));
                // errs() << "loop iterations: "
                //        << innerLoopIterations->getSExtValue() << "\n";

                // update SCEV, create IV
                stepMultiplier = innerLoopIterations->getSExtValue();

                IV = new InductionVariable(loop,
                                           IVM,
                                           SE,
                                           stepMultiplier,
                                           &phi,
                                           internalPHI,
                                           *sccContainingIV,
                                           loopEnv,
                                           referentialExpander);
              }
            }
          }
        }
      }

      /*
       * Allocate the induction variable.
       */
      if (noelleDeterminedValidIV) {
        IV = new InductionVariable(loop,
                                   IVM,
                                   SE,
                                   &phi,
                                   *sccContainingIV,
                                   loopEnv,
                                   referentialExpander);
      } else if (llvmDeterminedValidIV) {
        IV = new InductionVariable(loop,
                                   IVM,
                                   SE,
                                   &phi,
                                   *sccContainingIV,
                                   loopEnv,
                                   referentialExpander,
                                   ID);
      }

      /*
       * Check if we succeeded to have an IV.
       */
      if (IV == nullptr) {

        /*
         * The current PHI is not an IV.
         */
        continue;
      }

      /*
       * Only save IVs for which the step size is understood
       */
      if (!IV->getStepSCEV()) {
        delete IV;
        continue;
      }

      /*
       * Save the IV.
       */
      this->loopToIVsMap[loop].insert(IV);
      auto exitBlocks = loop->getLoopExitBasicBlocks();
      auto attribution = new LoopGoverningInductionVariable(loopToAnalyze,
                                                            *IV,
                                                            *sccContainingIV,
                                                            exitBlocks);
      if (attribution->isSCCContainingIVWellFormed()) {
        loopToGoverningIVAttrMap[loop] = attribution;
      } else {
        delete attribution;
      }
    }
  }

  return;
}

std::unordered_set<InductionVariable *> InductionVariableManager::
    getInductionVariables(void) const {

  /*
   * Fetch the outermost loop of @this.
   */
  auto loop = this->loop->getLoop();

  /*
   * Fetch the induction variables of the loop.
   */
  auto IVs = this->getInductionVariables(*loop);

  return IVs;
}

std::unordered_set<InductionVariable *> InductionVariableManager::
    getInductionVariables(Instruction *i) const {
  std::unordered_set<InductionVariable *> s{};

  /*
   * Iterate over every loop and their induction variables.
   */
  for (auto loopIVPair : this->loopToIVsMap) {

    /*
     * Fetch the set of induction variables of the current loop.
     */
    auto IVs = loopIVPair.second;

    /*
     * Check each induction variable.
     */
    for (auto IV : IVs) {
      auto insts = IV->getAllInstructions();
      if (insts.find(i) != insts.end()) {
        s.insert(IV);
      }
    }
  }

  return s;
}

bool InductionVariableManager::doesContributeToComputeAnInductionVariable(
    Instruction *i) const {

  /*
   * Fetch the induction variable that @i contributes to.
   */
  auto IVs = this->getInductionVariables(i);
  if (IVs.size() == 0) {
    return false;
  }

  return true;
}

InductionVariableManager::~InductionVariableManager() {
  for (auto ivAttributions : loopToGoverningIVAttrMap) {
    delete ivAttributions.second;
  }
  loopToGoverningIVAttrMap.clear();

  for (auto loopIVs : loopToIVsMap) {
    for (auto IV : loopIVs.second) {
      delete IV;
    }
  }
  loopToIVsMap.clear();

  return;
}

InductionVariable *InductionVariableManager::getInductionVariable(
    LoopStructure &LS,
    Instruction *i) const {

  /*
   * Fetch all induction variables.
   */
  auto IVs = this->getInductionVariables(LS);

  /*
   * Check each induction variable.
   */
  for (auto IV : IVs) {
    if (IV->isIVInstruction(i)) {

      /*
       * We found an induction variable that involves the instruction given as
       * input.
       */
      return IV;
    }
  }

  return nullptr;
}

std::unordered_set<InductionVariable *> InductionVariableManager::
    getInductionVariables(LoopStructure &LS) const {
  return this->loopToIVsMap.at(&LS);
}

InductionVariable *InductionVariableManager::getDerivingInductionVariable(
    LoopStructure &LS,
    Instruction *derivedInstruction) const {

  for (auto IV : this->getInductionVariables(LS)) {
    if (IV->isDerivedFromIVInstructions(derivedInstruction)) {

      /*
       * We found an induction variable that derives the instruction given as
       * input.
       */
      return IV;
    }
  }

  return nullptr;
}

LoopGoverningInductionVariable *InductionVariableManager::
    getLoopGoverningInductionVariable(void) const {

  /*
   * Fetch the loop
   */
  auto ls = this->loop->getLoop();
  assert(ls != nullptr);

  /*
   * Fetch the loop governing IV (if it exists).
   */
  auto IV = this->getLoopGoverningInductionVariable(*ls);

  return IV;
}

LoopGoverningInductionVariable *InductionVariableManager::
    getLoopGoverningInductionVariable(LoopStructure &LS) const {
  if (loopToGoverningIVAttrMap.find(&LS) == loopToGoverningIVAttrMap.end()) {
    return nullptr;
  }

  return loopToGoverningIVAttrMap.at(&LS);
}

} // namespace llvm::noelle
