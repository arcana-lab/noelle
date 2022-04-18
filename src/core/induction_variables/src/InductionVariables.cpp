/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/InductionVariables.hpp"
#include "noelle/core/LoopGoverningIVAttribution.hpp"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

namespace llvm::noelle {

InductionVariableManager::InductionVariableManager (
  StayConnectedNestedLoopForestNode *loopNode,
  InvariantManager &IVM,
  ScalarEvolution &SE,
  SCCDAG &sccdag,
  LoopEnvironment &loopEnv,
  Loop &LLVMLoop
) : loop{loopNode}, 
    loopToIVsMap{}, 
    loopToGoverningIVAttrMap{} 
  {
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
     * Fetch the loop header.
     */
    auto header = loop->getHeader();

    /*
     * Iterate over all phis within the loop header.
     */
    for (auto &phi : header->phis()) {

      /*
       * Check if LLVM considers this PHI to be an induction variable
       */
      InductionDescriptor ID = InductionDescriptor();
      bool llvmDeterminedValidIV;
      bool llvmLoopValidForInductionAnalysis = phi.getBasicBlockIndex(LLVMLoop.getLoopPreheader()) >= 0;
      
      if (llvmLoopValidForInductionAnalysis && InductionDescriptor::isInductionPHI(&phi, &LLVMLoop, &SE, ID)) {
        llvmDeterminedValidIV = true;
      } else if (phi.getType()->isFloatingPointTy() && InductionDescriptor::isFPInductionPHI(&phi, &LLVMLoop, &SE, ID)) {
        llvmDeterminedValidIV = true;
      } else {
        llvmDeterminedValidIV = false;
      }

      bool noelleDeterminedValidIV = true;
      /*
       * Check if the PHI node can be analyzed by the SCEV analysis.
       */
      if (!SE.isSCEVable(phi.getType())) {
        noelleDeterminedValidIV = false;
      } else {
        /*
         * Fetch the SCEV and check if it suggests this is an induction variable.
         */
        auto scev = SE.getSCEV(&phi);
        if (!scev || scev->getSCEVType() != SCEVTypes::scAddRecExpr) {
          noelleDeterminedValidIV = false;
        }
      }

      /*
       * Allocate the induction variable.
       */
      InductionVariable* IV = nullptr;
      auto sccContainingIV = sccdag.sccOfValue(&phi);
      if (noelleDeterminedValidIV) {
        IV = new InductionVariable(loop, IVM, SE, &phi, *sccContainingIV, loopEnv, referentialExpander);
      } else if (llvmDeterminedValidIV) {
        // Construct from LLVM abstraction
        IV = new InductionVariable(loop, IVM, SE, &phi, *sccContainingIV, loopEnv, referentialExpander, ID);
      } else {
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
      auto attribution = new LoopGoverningIVAttribution(*IV, *sccContainingIV, exitBlocks);
      if (attribution->isSCCContainingIVWellFormed()) {
        loopToGoverningIVAttrMap[loop] = attribution;
      } else {
        delete attribution;
      }
    }
  }

  return ;
}

std::unordered_set<InductionVariable *> InductionVariableManager::getInductionVariables (void) const {
  
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

std::unordered_set<InductionVariable *> InductionVariableManager::getInductionVariables (Instruction *i) const {
  std::unordered_set<InductionVariable *> s{};

  /*
   * Iterate over every loop and their induction variables.
   */
  for (auto loopIVPair : this->loopToIVsMap){

    /*
     * Fetch the set of induction variables of the current loop.
     */
    auto IVs = loopIVPair.second;

    /*
     * Check each induction variable.
     */
    for (auto IV : IVs){
      auto insts = IV->getAllInstructions();
      if (insts.find(i) != insts.end()){
        s.insert(IV);
      }
    }
  }

  return s;
}

bool InductionVariableManager::doesContributeToComputeAnInductionVariable (Instruction *i) const {

  /*
   * Fetch the induction variable that @i contributes to.
   */
  auto IVs = this->getInductionVariables(i);
  if (IVs.size() == 0){
    return false;
  }

  return true;
}

InductionVariableManager::~InductionVariableManager () {
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

  return ;
}

InductionVariable * InductionVariableManager::getInductionVariable (LoopStructure &LS, Instruction *i) const {

  /*
   * Fetch all induction variables.
   */
  auto IVs = this->getInductionVariables(LS);

  /*
   * Check each induction variable.
   */
  for (auto IV : IVs){
    if (IV->isIVInstruction(i)){

      /*
       * We found an induction variable that involves the instruction given as input.
       */
      return IV;
    }
  }

  return nullptr;
}

std::unordered_set<InductionVariable *> InductionVariableManager::getInductionVariables (LoopStructure &LS) const {
  return this->loopToIVsMap.at(&LS);
}

InductionVariable * InductionVariableManager::getDerivingInductionVariable (
  LoopStructure &LS,
  Instruction *derivedInstruction
) const {

  for (auto IV : this->getInductionVariables(LS)){
    if (IV->isDerivedFromIVInstructions(derivedInstruction)){

      /*
       * We found an induction variable that derives the instruction given as input.
       */
      return IV;
    }
  }

  return nullptr;
}

InductionVariable * InductionVariableManager::getLoopGoverningInductionVariable (LoopStructure &LS) const {

  /*
   * Check if the loop has the governing IV.
   */
  if (loopToGoverningIVAttrMap.find(&LS) == loopToGoverningIVAttrMap.end()) {
    return nullptr;
  }

  /*
   * Fetch the governing IV.
   */
  auto attribution = loopToGoverningIVAttrMap.at(&LS);

  return &attribution->getInductionVariable();
}

LoopGoverningIVAttribution * InductionVariableManager::getLoopGoverningIVAttribution (LoopStructure &LS) const {
  if (loopToGoverningIVAttrMap.find(&LS) == loopToGoverningIVAttrMap.end()) {
    return nullptr;
  }

  return loopToGoverningIVAttrMap.at(&LS);
}

}
