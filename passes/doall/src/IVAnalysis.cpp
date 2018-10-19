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
   * Map the original instructions related to the loop induction variable to the cloned ones in the parallelized loop.
   */
  chunker->cloneIV = cast<PHINode>(chunker->fetchClone(attrs->singlePHI));
  auto IVInfo = attrs->simpleIVInfo;
  chunker->cloneIVInfo.cmp = cast<CmpInst>(chunker->fetchClone(IVInfo.cmp));
  chunker->cloneIVInfo.br = cast<BranchInst>(chunker->fetchClone(IVInfo.br));

  auto startClone = chunker->fetchClone(IVInfo.start);
  auto endClone = chunker->fetchClone(IVInfo.cmpIVTo);
  auto offsetV = ConstantInt::get(IVInfo.step->getType(), IVInfo.endOffset);
  IRBuilder<> entryB(chunker->entryBlock);
  endClone = IVInfo.endOffset ? entryB.CreateAdd(endClone, offsetV) : endClone;

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
