#include "DOALL.hpp"

// TODO(angelo): Have LoopSummary scrape this info from IVs
void DOALL::simplifyOriginalLoopIV (
  LoopDependenceInfoForParallelizer *LDI
) {
  auto task = (DOALLTaskExecution *)tasks[0];

  /*
   * Fetch information about the loop induction variable controlling the loop trip count.
   */
  auto headerBr = LDI->header->getTerminator();
  auto headerSCC = LDI->loopSCCDAG->sccOfValue(headerBr);
  auto &attrs = LDI->sccdagAttrs.getSCCAttrs(headerSCC);
  assert(attrs->isSimpleIV);
  task->originalIVAttrs = attrs.get();

  /*
   * Identify clones of the PHI, Cmp, and Branch instructions that govern the loop IV.
   */
  auto &iClones = task->instructionClones;
  task->originalIVClone = cast<PHINode>(iClones[attrs->singlePHI]);
  auto IVInfo = attrs->simpleIVInfo;
  task->clonedIVInfo.cmp = cast<CmpInst>(iClones[IVInfo.cmp]);
  task->clonedIVInfo.br = cast<BranchInst>(iClones[IVInfo.br]);

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
  auto startClone = fetchClone(IVInfo.start);

  /*
   * Fetch clone of Value used in CmpInst of the original loop's IV
   */
  auto cmpToClone = fetchClone(IVInfo.cmpIVTo);

  /*
   * Fetch the offset from the compared to Value to the end value: [..., end)
   * cmpToValue + offset = end
   */
  auto offsetV = ConstantInt::get(IVInfo.step->getType(), IVInfo.endOffset);
  IRBuilder<> entryBuilder(task->entryBlock);
  auto endClone = IVInfo.endOffset ? entryBuilder.CreateAdd(cmpToClone, offsetV) : cmpToClone;

  /*
   * Manipulate clone IV [start, end) values to guarantee the following: 
   * 1) The PHI begins with the start value
   * 2) The PHI is incremented/decremented at the loop latch, NOT before
   * 3) The CmpInst checks that the end value is NOT reached. If it is, the loop body is NOT executed
   */
  auto oneV = ConstantInt::get(IVInfo.step->getType(), 1);
  task->clonedIVInfo.step = oneV;
  auto stepSize = IVInfo.step->getValue().getSExtValue();
  if (stepSize == 1) {
    task->clonedIVInfo.start = startClone;
    task->clonedIVInfo.cmpIVTo = endClone;
  } else {
    task->clonedIVInfo.start = entryBuilder.CreateAdd(endClone, oneV);
    task->clonedIVInfo.cmpIVTo = entryBuilder.CreateAdd(startClone, oneV);
  }
}
