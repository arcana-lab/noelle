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
#include "arcana/noelle/core/InductionVariables.hpp"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

namespace arcana::noelle {

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
        }
        /*
         * For a PHI that has a SCEV that is not an AddRecExpr, it may still be
         * an IV that is being updated in a subloop if the proceeding conditions
         * are met.
         */
        else if (scev->getSCEVType() != SCEVTypes::scAddRecExpr) {
          noelleDeterminedValidIV = false;
          int64_t stepMultiplier = 1;

          /*
           * 1. In the PHI's SCC, there is one PHI that has an AddRecExpr
           * SCEV and is contained in the subloop of the original loop.
           */
          auto foundOnePHI = false;
          PHINode *internalPHI = nullptr;

          sccContainingIV->iterateOverInstructions([&](Instruction *I) -> bool {
            if (isa<PHINode>(I) && I != &phi
                && SE.getSCEV(I)->getSCEVType() == SCEVTypes::scAddRecExpr
                && this->loop->isIncludedInItsSubLoops(I)) {
              if (!foundOnePHI) {
                foundOnePHI = true;
                internalPHI = cast<PHINode>(I);
              } else {
                foundOnePHI = false;
                return true;
              }
            }
            return false;
          });

          if (!foundOnePHI) {
            continue;
          }

          /*
           * 2. The subloop has only one exit condition, which compares
           * the subloop's governing IV to a constant.
           */
          auto subloop = this->loop->getInnermostLoopThatContains(internalPHI);
          if (subloop->getLoopExitBasicBlocks().size() != 1) {
            continue;
          }

          /*
           * Note: a BranchInst is expected to terminate the loop header.
           * We don't handle do-while loops at the moment.
           */
          auto subloopHeader = subloop->getHeader();
          if (subloopHeader->getUniqueSuccessor() != nullptr) {

            /*
             * This is a do-while loop.
             */
            continue;
          }
          if (auto subloopExitBr =
                  dyn_cast<BranchInst>(subloopHeader->getTerminator())) {

            /*
             * Fetch the condition.
             */
            auto subloopExitBrCondition = subloopExitBr->getCondition();
            if (!isa<CmpInst>(subloopExitBrCondition)) {
              continue;
            }
            auto subloopExitCond = cast<CmpInst>(subloopExitBrCondition);
            auto subloopExitCondL = subloopExitCond->getOperand(0);
            auto subloopExitCondR = subloopExitCond->getOperand(1);

            const SCEV *subloopIV = nullptr;
            const SCEV *subloopExitSCEV = nullptr;
            if (SE.getSCEV(subloopExitCondL)->getSCEVType()
                    == SCEVTypes::scAddRecExpr
                && SE.getSCEV(subloopExitCondR)->getSCEVType()
                       == SCEVTypes::scConstant) {
              subloopIV = SE.getSCEV(subloopExitCondL);
              subloopExitSCEV = SE.getSCEV(subloopExitCondR);
            } else if (SE.getSCEV(subloopExitCondR)->getSCEVType()
                           == SCEVTypes::scAddRecExpr
                       && SE.getSCEV(subloopExitCondL)->getSCEVType()
                              == SCEVTypes::scConstant) {
              subloopIV = SE.getSCEV(subloopExitCondR);
              subloopExitSCEV = SE.getSCEV(subloopExitCondL);
            }

            if (subloopExitSCEV == nullptr || subloopIV == nullptr) {
              continue;
            }

            /*
             * If all conditions are met, calculate the number of inner loop
             * iterations (step multiplier) and allocate the IV.
             */
            assert(subloopExitSCEV->getSCEVType() == SCEVTypes::scConstant);
            auto subloopExitConstant =
                cast<SCEVConstant>(subloopExitSCEV)->getValue()->getSExtValue();

            assert(subloopIV->getSCEVType() == SCEVTypes::scAddRecExpr);
            auto subloopIVSCEV = cast<SCEVAddRecExpr>(subloopIV);

            auto subloopExitBBs = subloop->getLoopExitBasicBlocks();
            bool exitsOnTrue = false;
            if (std::find(subloopExitBBs.begin(),
                          subloopExitBBs.end(),
                          subloopExitBr->getSuccessor(0))
                != subloopExitBBs.end()) {
              exitsOnTrue = true;
            }

            if (auto startSCEVConstant =
                    dyn_cast<SCEVConstant>(subloopIVSCEV->getStart())) {
              auto subloopStartValue =
                  startSCEVConstant->getValue()->getSExtValue();
              if (auto stepSCEVConstant = dyn_cast<SCEVConstant>(
                      subloopIVSCEV->getStepRecurrence(SE))) {
                auto subloopStepSize =
                    stepSCEVConstant->getValue()->getSExtValue();
                auto negativeStep = stepSCEVConstant->getValue()->isNegative();

                /*
                 * We ignore the combinations that don't make sense for IVs.
                 * Example: an increasing IV that exits when it is < C.
                 * In this case, if the start value is < C, the loop wouldn't
                 * execute. Otherwise, it will never be < C and run infinitely.
                 */
                auto unhandledCmp = false;
                switch (subloopExitCond->getPredicate()) {

                  case CmpInst::Predicate::ICMP_EQ:
                    if (!exitsOnTrue)
                      unhandledCmp = true;
                    break;

                  case CmpInst::Predicate::ICMP_NE:
                    if (exitsOnTrue)
                      unhandledCmp = true;
                    break;

                  case CmpInst::Predicate::ICMP_UGT:
                  case CmpInst::Predicate::ICMP_SGT:
                    if (negativeStep == exitsOnTrue)
                      unhandledCmp = true;
                    if (!negativeStep)
                      subloopExitConstant += 1;
                    break;

                  case CmpInst::Predicate::ICMP_SGE:
                  case CmpInst::Predicate::ICMP_UGE:
                    if (negativeStep == exitsOnTrue)
                      unhandledCmp = true;
                    if (negativeStep)
                      subloopExitConstant += 1;
                    break;

                  case CmpInst::Predicate::ICMP_SLT:
                  case CmpInst::Predicate::ICMP_ULT:
                    if (negativeStep != exitsOnTrue)
                      unhandledCmp = true;
                    if (negativeStep)
                      subloopExitConstant += 1;
                    break;

                  case CmpInst::Predicate::ICMP_SLE:
                  case CmpInst::Predicate::ICMP_ULE:
                    if (negativeStep != exitsOnTrue)
                      unhandledCmp = true;
                    if (!negativeStep)
                      subloopExitConstant += 1;
                    break;

                  default:
                    unhandledCmp = true;
                    break;
                }

                if (unhandledCmp) {
                  continue;
                }

                auto d = std::div(subloopExitConstant - subloopStartValue,
                                  subloopStepSize);
                stepMultiplier = d.quot + (d.rem ? 1 : 0);

                IV = new InductionVariable(
                    loop,
                    IVM,
                    SE,
                    stepMultiplier,
                    &phi,
                    std::unordered_set<PHINode *>({ internalPHI }),
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
                                   1,
                                   &phi,
                                   std::unordered_set<PHINode *>({ &phi }),
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

} // namespace arcana::noelle
