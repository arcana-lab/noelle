/*
 * Copyright 2019 - 2020  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopInvariantCodeMotion.hpp"

using namespace llvm;

bool LoopInvariantCodeMotion::hoistInvariantValues (
  LoopDependenceInfo const &LDI
) {

  bool modified = false;

  auto invariantManager = LDI.getInvariantManager();
  auto loopStructure = LDI.getLoopStructure();
  auto header = loopStructure->getHeader();
  auto preHeader = loopStructure->getPreHeader();

  std::vector<Instruction *> instructionsToHoistToPreheader{};
  std::unordered_set<PHINode *> phisToRemove{};
  for (auto B : loopStructure->getBasicBlocks()) {
    for (auto &I : *B) {
      if (!invariantManager->isLoopInvariant(&I)) {
        continue;
      }

      /*
       * There is no benefit to hoisting GEPs, and it seems that
       * other normalizing transformations bring GEPs next to their usages
       */
      if (isa<GetElementPtrInst>(&I)) {
        continue;
      }
      modified = true;

      auto phi = dyn_cast<PHINode>(&I);
      if (!phi) {
        instructionsToHoistToPreheader.push_back(&I);
        continue;
      }

      /*
       * All PHI invariants are equivalent, so choose any to replace the PHI
       */
      auto valueToReplacePHI = phi->getIncomingValue(0);

      /*
       * Note, the users are modified, so we must cache them first
       */
      std::unordered_set<User *> users(phi->user_begin(), phi->user_end());
      for (auto user : users) {
        user->replaceUsesOfWith(phi, valueToReplacePHI);
      }
      phisToRemove.insert(phi);

      /*
       * If the replacement is an Instruction, it needs to be hoisted
       */
      if (auto instToReplacePHI = dyn_cast<Instruction>(valueToReplacePHI)) {
        instructionsToHoistToPreheader.push_back(instToReplacePHI);
      }
    }
  }

  for (auto phi : phisToRemove) {
    phi->eraseFromParent();
  }

  /*
   * Sort invariants to hoist in order of dominance to preserve execution order
   */
  DominatorTree DT(*header->getParent());
  PostDominatorTree PDT(*header->getParent());
  DominatorSummary DS(DT, PDT);
  auto dominanceCmpFunc = [&DS] (Instruction *I, Instruction *J) -> bool {
    return DS.DT.dominates(I, J);
  };
  std::sort (instructionsToHoistToPreheader.begin(), instructionsToHoistToPreheader.end(), dominanceCmpFunc);

  /*
   * Hoist each instruction into the preheader
   */
  IRBuilder<> preHeaderBuilder(preHeader->getTerminator());
  for (auto I : instructionsToHoistToPreheader) {
    I->removeFromParent();
    preHeaderBuilder.Insert(I);
  }

  return modified;
}
