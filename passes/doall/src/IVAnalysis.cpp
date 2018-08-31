#include "DOALL.hpp"

// TODO(angelo): Have LoopSummary scrape this info from IVs
void DOALL::collectOriginIVValues (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  std::unique_ptr<ChunkerInfo> &chunker,
  ScalarEvolution &SE
) {

  /*
   * Find origin loop's induction variable
   */
  auto originHeaderBr = LDI->header->getTerminator();
  assert(isa<BranchInst>(originHeaderBr));
  chunker->originHeaderBr = cast<BranchInst>(originHeaderBr);
  auto originCond = chunker->originHeaderBr->getCondition();
  assert(isa<CmpInst>(originCond));
  chunker->originCmp = cast<CmpInst>(originCond);

  int opIndex = 0;
  for (auto &condOp : cast<User>(chunker->originCmp)->operands()) {
    bool isPHI = isa<PHINode>(condOp);
    if (isPHI) {
      chunker->originCmpPHIIndex = opIndex;
      chunker->originIV = cast<PHINode>(condOp);
    } else {
      chunker->originCmpMaxIndex = opIndex;
      chunker->maxIV = condOp;
    }
    opIndex++;
  }
  assert(chunker->originIV != nullptr && chunker->maxIV != nullptr);

  // HACK: Make the condition stronger so that chunks don't skip over the equality condition
  auto strictMaxIVPredicate = chunker->originCmp->getPredicate();
  if (strictMaxIVPredicate == CmpInst::Predicate::ICMP_EQ) {
    strictMaxIVPredicate = chunker->originCmpPHIIndex == 0 
      ? CmpInst::Predicate::ICMP_UGE
      : CmpInst::Predicate::ICMP_ULE;
  }
  chunker->strictPredicate = strictMaxIVPredicate;

  /*
   * Determine step size of induction variable
   */
  for (auto user : chunker->originIV->users()) {
    auto scev = SE.getSCEV((Value *)user);
    switch (scev->getSCEVType()) {
    case scAddExpr:
    case scAddRecExpr:
      chunker->stepperIV = (Value *)user;
      Value *lhs = user->getOperand(0);
      Value *rhs = user->getOperand(1);
      if (isa<ConstantInt>(lhs)) {
        chunker->originStepSize = cast<ConstantInt>(lhs);
        chunker->stepSizeIVIndex = 0;
      } else if (isa<ConstantInt>(rhs)) {
        chunker->originStepSize = cast<ConstantInt>(rhs);
        chunker->stepSizeIVIndex = 1;
      } else continue;
      break;
    }
  }
  assert(chunker->stepperIV != nullptr && chunker->originStepSize != nullptr);

  /*
   * Get start value off of original loop IV
   */
  chunker->startValIVIndex = chunker->originIV->getBasicBlockIndex(LDI->preHeader);
  chunker->starterIV = chunker->originIV->getIncomingValue(chunker->startValIVIndex);
  if (!isa<ConstantInt>(chunker->starterIV)) {
    chunker->starterIV = chunker->innerValMap[(Instruction*)chunker->starterIV];
  }
}

