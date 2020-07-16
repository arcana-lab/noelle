/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "InductionVariables.hpp"
#include "LoopGoverningIVAttribution.hpp"

using namespace llvm;

InductionVariableManager::InductionVariableManager (
  LoopsSummary &LIS,
  InvariantManager &IVM,
  ScalarEvolution &SE,
  SCCDAG &sccdag,
  LoopEnvironment &loopEnv
) : loopToIVsMap{}, loopToGoverningIVMap{} {

  Function &F = *LIS.getLoopNestingTreeRoot()->getHeader()->getParent();
  ScalarEvolutionReferentialExpander referentialExpander(SE, F);

  for (auto &loop : LIS.loops) {
    loopToIVsMap[loop.get()] = std::unordered_set<InductionVariable *>();

    /*
     * Fetch the loop header.
     */
    auto header = loop->getHeader();

    /*
     * Iterate over all phis within the loop header.
     */
    for (auto &phi : header->phis()) {
      // phi.print(errs() << "Checking PHI: "); errs() << "\n";
      auto scev = SE.getSCEV(&phi);
      if (!scev || scev->getSCEVType() != SCEVTypes::scAddRecExpr) continue;

      auto sccContainingIV = sccdag.sccOfValue(&phi);
      auto IV = new InductionVariable(loop.get(), IVM, SE, &phi, *sccContainingIV, loopEnv, referentialExpander); 

      /*
       * Only save IVs for which the step size is understood
       */
      if (!IV->getStepSCEV()) {
        delete IV;
        continue;
      }

      loopToIVsMap[loop.get()].insert(IV);
      auto exitBlocks = LIS.getLoop(phi)->getLoopExitBasicBlocks();
      LoopGoverningIVAttribution attribution(*IV, *sccContainingIV, exitBlocks);
      if (attribution.isSCCContainingIVWellFormed()) {
        loopToGoverningIVMap[loop.get()] = IV;
      }
    }
  }
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
  for (auto loopIVs : loopToIVsMap) {
    for (auto IV : loopIVs.second) {
      delete IV;
    }
  }

  loopToIVsMap.clear();
  loopToGoverningIVMap.clear();

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
    auto insts = IV->getAllInstructions();
    if (insts.find(i) != insts.end()){

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
) {

  for (auto IV : this->getInductionVariables(LS)){
    auto insts = IV->getDerivedSCEVInstructions();
    if (insts.find(derivedInstruction) != insts.end()){

      /*
       * We found an induction variable that derives the instruction given as input.
       */
      return IV;
    }
  }

  return nullptr;
}

InductionVariable * InductionVariableManager::getLoopGoverningInductionVariable (LoopStructure &LS) const {
  if (loopToGoverningIVMap.find(&LS) == loopToGoverningIVMap.end()) return nullptr;
  return loopToGoverningIVMap.at(&LS);
}
