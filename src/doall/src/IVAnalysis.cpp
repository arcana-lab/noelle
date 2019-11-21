/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "DOALL.hpp"
#include "DOALLTask.hpp"

// TODO(angelo): Have LoopSummary scrape this info from IVs
void DOALL::simplifyOriginalLoopIV (
  LoopDependenceInfo *LDI
) {
  auto task = (DOALLTask *)tasks[0];

  /*
   * Fetch information about the loop induction variable controlling the loop trip count.
   */
  auto headerBr = LDI->header->getTerminator();
  auto headerSCC = LDI->sccdagAttrs.getSCCDAG()->sccOfValue(headerBr);
  auto attrs = LDI->sccdagAttrs.getSCCAttrs(headerSCC);
  task->originalIVAttrs = attrs;
  auto &IVBounds = *LDI->sccdagAttrs.sccIVBounds[headerSCC];

  /*
   * Identify clone of the PHI, CmpInst, and BranchInst that govern the loop IV.
   */
  auto &iClones = task->instructionClones;
  task->cloneOfOriginalIV = cast<PHINode>(iClones[attrs->getSinglePHI()]);
  assert(task->originalIVAttrs->controlPairs.size() == 1);
  auto controlPair = *task->originalIVAttrs->controlPairs.begin();
  task->cloneOfOriginalCmp = cast<CmpInst>(iClones[(Instruction*)controlPair.first]);
  task->cloneOfOriginalBr = cast<BranchInst>(iClones[controlPair.second]);

  /*
   * ============================================================================
   * Task: Determine the bounds [start, end) of the original loop's IV
   * ============================================================================
   */

  auto fetchClone = [&](Value *V) -> Value * {
    if (isa<ConstantData>(V)) return V;
    if (task->liveInClones.find(V) != task->liveInClones.end()) {
      return task->liveInClones[V];
    }
    assert(isa<Instruction>(V));
    auto iCloneIter = iClones.find((Instruction *)V);
    assert(iCloneIter != iClones.end());
    return iCloneIter->second;
  };

  /*
   * Fetch clone of initial Value of the original loop's IV PHINode: [start, ...)
   */
  auto startClone = fetchClone(IVBounds.start);

  /*
   * Fetch clone of Value used in CmpInst of the original loop's IV
   * If an instruction, hoist to the entry block for further manipulation
   */
  auto cmpToClone = fetchClone(IVBounds.cmpIVTo);

  IRBuilder<> entryBuilder(task->entryBlock);
  if (isa<Instruction>(cmpToClone)) {
    auto cmpI = (Instruction*)cmpToClone;
    cmpI->removeFromParent();
    entryBuilder.Insert(cmpI);

    for (auto I : IVBounds.cmpToDerivation) {
      Value *cloneI = fetchClone(I);
      assert(isa<Instruction>(cloneI));
      task->clonedIVBounds.cmpToDerivation.push_back((Instruction*)cloneI);
    }
  }

  /*
   * Fetch the offset from the compared to Value to the end value: [..., end)
   * cmpToValue + offset = end
   */
  auto offsetV = ConstantInt::get(IVBounds.step->getType(), IVBounds.endOffset);
  auto endClone = IVBounds.endOffset ? entryBuilder.CreateAdd(cmpToClone, offsetV) : cmpToClone;

  /*
   * Manipulate clone IV [start, end) values to guarantee the following: 
   * 1) The PHI begins with the start value
   * 2) The PHI is incremented/decremented at the loop latch, NOT before
   * 3) The CmpInst checks that the end value is NOT reached. If it is, the loop body is NOT executed
   */
  auto oneV = ConstantInt::get(IVBounds.step->getType(), 1);
  task->clonedIVBounds.step = oneV;
  auto stepSize = IVBounds.step->getValue().getSExtValue();
  if (stepSize == 1) {
    task->clonedIVBounds.start = startClone;
    task->clonedIVBounds.cmpIVTo = endClone;
  } else {
    task->clonedIVBounds.start = entryBuilder.CreateAdd(endClone, oneV);
    task->clonedIVBounds.cmpIVTo = entryBuilder.CreateAdd(startClone, oneV);
  }
}
