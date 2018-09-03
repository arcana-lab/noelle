#include "DOALL.hpp"

// TODO(angelo): Have LoopSummary scrape this info from IVs
void DOALL::reduceOriginIV (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker
) {

  auto headerBr = LDI->header->getTerminator();
  auto headerSCC = LDI->loopSCCDAG->sccOfValue(headerBr);
  auto &attrs = LDI->sccdagAttrs.getSCCAttrs(headerSCC);
  assert(attrs->isSimpleIV);
  chunker->originIVAttrs = attrs.get();

  chunker->cloneIV = cast<PHINode>(chunker->fetchClone(attrs->singlePHI));
  auto IVInfo = attrs->simpleIVInfo;
  chunker->cloneIVInfo.cmp = cast<CmpInst>(chunker->fetchClone(IVInfo.cmp));
  chunker->cloneIVInfo.br = cast<BranchInst>(chunker->fetchClone(IVInfo.br));
  IVInfo.cmp->print(errs() << "Original compare:\t"); errs() << "\n";
  IVInfo.br->print(errs() << "Original branch:\t"); errs() << "\n";
  chunker->cloneIVInfo.cmp->print(errs() << "New compare:\t"); errs() << "\n";
  chunker->cloneIVInfo.br->print(errs() << "New branch:\t"); errs() << "\n";

  auto startClone = chunker->fetchClone(IVInfo.start);
  auto endClone = chunker->fetchClone(IVInfo.end);
  IRBuilder<> entryB(chunker->entryBlock);
  auto oneV = ConstantInt::get(IVInfo.step->getType(), 1);

  auto stepSize = IVInfo.step->getValue();
  if (stepSize == 1) {
    chunker->cloneIVInfo.start = startClone;
    chunker->cloneIVInfo.step = IVInfo.step;
    chunker->cloneIVInfo.end = endClone;
  } else if (stepSize == -1) {
    chunker->cloneIVInfo.start = entryB.CreateAdd(endClone, oneV);
    chunker->cloneIVInfo.step = oneV;
    chunker->cloneIVInfo.end = entryB.CreateAdd(startClone, oneV);
  } else {
    errs() << "ERROR: DOALL was not given a loop IV with step value of 1 or -1\n";
    abort();
  }
}
