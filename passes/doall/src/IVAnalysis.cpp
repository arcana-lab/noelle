#include "DOALL.hpp"

// TODO(angelo): Have LoopSummary scrape this info from IVs
void DOALL::reduceOriginIV (
  LoopDependenceInfoForParallelizer *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker,
  ScalarEvolution &SE
) {

  /*
   * Fetch the information about the loop induction variable that controls the loop trip count.
   */
  auto headerBr = LDI->header->getTerminator();
  auto headerSCC = LDI->loopSCCDAG->sccOfValue(headerBr);
  auto &attrs = LDI->sccdagAttrs.getSCCAttrs(headerSCC);
  assert(attrs->isSimpleIV);
  chunker->originIVAttrs = attrs.get();

  /*
   * Map from instructions of the original loop's IV to the respective cloned instructions.
   */
  chunker->cloneIV = cast<PHINode>(chunker->fetchClone(attrs->singlePHI));
  auto IVInfo = attrs->simpleIVInfo;
  chunker->cloneIVInfo.cmp = cast<CmpInst>(chunker->fetchClone(IVInfo.cmp));
  chunker->cloneIVInfo.br = cast<BranchInst>(chunker->fetchClone(IVInfo.br));

  /*
   * ============================================================================
   * Task: Determine the bounds [start, end) of the original loop's IV
   * ============================================================================
   */

  /*
   * Fetch clone of initial Value of the original loop's IV PHINode: [start, ...)
   */
  auto startClone = chunker->fetchClone(IVInfo.start);

  /*
   * Fetch clone of Value used in CmpInst of the original loop's IV
   */
  auto cmpToClone = chunker->fetchClone(IVInfo.cmpIVTo);

  /*
   * Fetch the offset from the compared to Value to the end value: [..., end)
   * cmpToValue + offset = end
   */
  auto offsetV = ConstantInt::get(IVInfo.step->getType(), IVInfo.endOffset);
  IRBuilder<> entryB(chunker->entryBlock);
  endClone = IVInfo.endOffset ? entryB.CreateAdd(endClone, offsetV) : endClone;

  /*
   * Manipulate clone IV [start, end) values to guarantee the following: 
   * 1) The PHI begins with the start value
   * 2) The PHI is incremented/decremented at the loop latch, NOT before
   * 3) The CmpInst checks that the end value is NOT reached. If it is, the loop body is NOT executed
   */
  auto oneV = ConstantInt::get(IVInfo.step->getType(), 1);
  chunker->cloneIVInfo.step = oneV;
  auto stepSize = IVInfo.step->getValue().getSExtValue();
  if (stepSize == 1) {
    chunker->cloneIVInfo.start = startClone;
    chunker->cloneIVInfo.cmpIVTo = endClone;
  } else {
    chunker->cloneIVInfo.start = entryB.CreateAdd(endClone, oneV);
    chunker->cloneIVInfo.cmpIVTo = entryB.CreateAdd(startClone, oneV);
  }
}
