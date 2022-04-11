/*
 * Copyright 2019 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopStats.hpp"

using namespace llvm;
using namespace llvm::noelle;

void LoopStats::collectStatsOnNoelleIVs (Hot *profiles, LoopDependenceInfo &LDI, Stats *statsForLoop) {
  auto loopStructure = LDI.getLoopStructure();
  auto ivManager = LDI.getInductionVariableManager();
  auto ivs = ivManager->getInductionVariables(*loopStructure);

  statsForLoop->numberOfIVs = ivs.size();
  for (auto iv : ivs) {
    auto phiNode = iv->getLoopEntryPHI();
    statsForLoop->numberOfDynamicIVs += profiles->getTotalInstructions(phiNode);
  }

  auto GIV = ivManager->getLoopGoverningIVAttribution(*loopStructure);
  if (GIV != nullptr){
    statsForLoop->isGovernedByIV = 1;
    statsForLoop->numberOfDynamicGovernedIVs = profiles->getTotalInstructions(GIV->getHeaderCompareInstructionToComputeExitCondition());
  }

  return ;
}

void LoopStats::collectStatsOnLLVMIVs (Hot *profiles, ScalarEvolution &SE, Loop &llvmLoop, Stats *statsForLoop) {
  /*
   * Note: LLVM does not provide a way to collect all instructions used in computing IVs
   */

  InductionDescriptor ID = InductionDescriptor();
  for (auto &phi : llvmLoop.getHeader()->phis()) {
    bool llvmLoopValidForInductionAnalysis = phi.getBasicBlockIndex(llvmLoop.getLoopPreheader()) >= 0;
    if (llvmLoopValidForInductionAnalysis && llvmLoop.isAuxiliaryInductionVariable(phi, SE)) {
      statsForLoop->numberOfIVs++;
    } else if (llvmLoopValidForInductionAnalysis && phi.getType()->isFloatingPointTy() && InductionDescriptor::isFPInductionPHI(&phi, &llvmLoop, &SE, ID)) {
      statsForLoop->numberOfIVs++;
    }
    statsForLoop->numberOfDynamicIVs = profiles->getTotalInstructions(&phi);
  }

  auto governingIV = llvmLoop.getInductionVariable(SE);
  if (governingIV != nullptr){
    statsForLoop->isGovernedByIV = 1;
  }

  return ;
}
