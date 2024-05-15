/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/IVStepperUtility.hpp"

namespace arcana::noelle {

LoopGoverningIVUtility::LoopGoverningIVUtility(
    LoopStructure *loopOfGIV,
    InductionVariableManager &IVM,
    LoopGoverningInductionVariable &attribution)
  : loop{ loopOfGIV },
    attribution{ attribution },
    conditionValueOrderedDerivation{},
    flipOperandsToUseNonStrictPredicate{ false },
    flipBrSuccessorsToUseNonStrictPredicate{ false },
    isWhile{ false } {

  /*
   * Fetch the IV
   */
  auto IV = attribution.getInductionVariable();

  /*
   * Fetch information about the condition to exit the loop.
   *
   * Check where the IV is in the comparison (left or right).
   */
  this->condition =
      attribution.getHeaderCompareInstructionToComputeExitCondition();
  this->doesOriginalCmpInstHaveIVAsLeftOperand =
      condition->getOperand(0)
      == attribution.getValueToCompareAgainstExitConditionValue();

  /*
   * Collect the set of instructions that need to be executed to evaluate the
   * loop exit condition for the subsequent iteration.
   */
  auto conditionValueDerivationSet = attribution.getConditionValueDerivation();
  for (auto &I : *condition->getParent()) {
    if (conditionValueDerivationSet.find(&I)
        == conditionValueDerivationSet.end())
      continue;
    conditionValueOrderedDerivation.push_back(&I);
  }
  assert(IV->getSingleComputedStepValue()
         && (isa<ConstantInt>(IV->getSingleComputedStepValue())
             || isa<ConstantFP>(IV->getSingleComputedStepValue())));

  /*
   * Fetch information about the step value for the IV.
   */
  auto isStepValuePositive = IV->isStepValuePositive();

  /*
   * Fetch information about the predicate that when true the execution needs to
   * leave the loop.
   */
  auto conditionExitsOnTrue = attribution.getHeaderBrInst()->getSuccessor(0)
                              == attribution.getExitBlockFromHeader();
  auto exitPredicate = conditionExitsOnTrue ? condition->getPredicate()
                                            : condition->getInversePredicate();
  exitPredicate = doesOriginalCmpInstHaveIVAsLeftOperand
                      ? exitPredicate
                      : CmpInst::getSwappedPredicate(exitPredicate);
  this->flipOperandsToUseNonStrictPredicate =
      !doesOriginalCmpInstHaveIVAsLeftOperand;
  this->flipBrSuccessorsToUseNonStrictPredicate = !conditionExitsOnTrue;
  this->nonStrictPredicate = exitPredicate;
  this->strictPredicate = exitPredicate;
  switch (exitPredicate) {

    case CmpInst::Predicate::ICMP_NE:
    case CmpInst::Predicate::FCMP_UNE:
    case CmpInst::Predicate::FCMP_ONE:

      /*
       * This predicate is non-strict and will result in either 0 or 1
       * iteration(s)
       */
      break;

    case CmpInst::Predicate::ICMP_EQ:
    case CmpInst::Predicate::FCMP_UEQ:
    case CmpInst::Predicate::FCMP_OEQ:
      // This predicate is strict and needs to be extended to LTE/GTE to catch
      // jumping past the exiting value
      if (isStepValuePositive) {
        this->nonStrictPredicate = CmpInst::Predicate::ICMP_SGE;
        this->strictPredicate = CmpInst::Predicate::ICMP_SGT;
      } else {
        this->nonStrictPredicate = CmpInst::Predicate::ICMP_SLE;
        this->strictPredicate = CmpInst::Predicate::ICMP_SLT;
      }
      break;

    case CmpInst::Predicate::ICMP_SLE:
    case CmpInst::Predicate::ICMP_SLT:
    case CmpInst::Predicate::ICMP_ULT:
    case CmpInst::Predicate::ICMP_ULE:
    case CmpInst::Predicate::FCMP_ULT:
    case CmpInst::Predicate::FCMP_ULE:
      // This predicate is non-strict. We simply assert that the step value has
      // the expected sign

      // HACK: while it is technically correct to increment with a less than
      // exit condition, yielding 0 or 1 iteration, it would break under
      // assumptions that further recurrences of the IV can be checked on this
      // condition Our parallelization schemes make that assumption, hence the
      // assert here
      assert(!isStepValuePositive
             && "IV step value is not compatible with exit condition!");
      break;

    case CmpInst::Predicate::ICMP_UGT:
    case CmpInst::Predicate::ICMP_UGE:
    case CmpInst::Predicate::ICMP_SGT:
    case CmpInst::Predicate::ICMP_SGE:
    case CmpInst::Predicate::FCMP_UGT:
    case CmpInst::Predicate::FCMP_UGE:
      // This predicate is non-strict. We simply assert that the step value has
      // the expected sign

      // HACK: while it is technically correct to decrement with a greater than
      // exit condition, yielding 0 or 1 iteration, it would break under
      // assumptions that further recurrences of the IV can be checked on this
      // condition Our parallelization schemes make that assumption, hence the
      // assert here
      assert(isStepValuePositive
             && "IV step value is not compatible with exit condition!");
      break;

    default:
      errs() << "LoopGoverningIVUtility: we do not handle a loop predicate\n";
      abort();
  }

  /*
   * Check if the loop has a while form.
   */
  for (auto predecessorOfHeader : predecessors(this->loop->getHeader())) {

    /*
     * Fetch the next predecessor of the header that is included in the loop.
     */
    if (!this->loop->isIncluded(predecessorOfHeader)) {
      continue;
    }

    /*
     * Check if the terminator of the current predecessor is an unconditional
     * branch (necessary condition for a while loop)
     */
    auto terminatorOfPred = predecessorOfHeader->getTerminator();
    if (!isa<BranchInst>(terminatorOfPred)) {
      continue;
    }
    auto branch = cast<BranchInst>(terminatorOfPred);
    if (branch->isConditional()) {
      continue;
    }

    /*
     * We found a predecessor of the loop's header that ends with an
     * unconditional branch. Check if this basic block contains computation that
     * isn't bitcasts or IV related (necessary condition for a while loop).
     */
    auto doesItContainComputation = false;
    for (auto &inst : *predecessorOfHeader) {
      if (predecessorOfHeader->getTerminator() == &inst) {
        continue;
      }
      if (isa<BitCastInst>(&inst)) {
        continue;
      }
      if (IVM.doesContributeToComputeAnInductionVariable(&inst)) {
        continue;
      }
      doesItContainComputation = true;
      break;
    }
    if (doesItContainComputation) {

      /*
       * We found a non empty basic block.
       * Hence, the loop is a while loop.
       */
      this->isWhile = true;
      break;
    }
  }

  return;
}

void LoopGoverningIVUtility::
    updateConditionAndBranchToCatchIteratingPastExitValue(
        CmpInst *cmpToUpdate,
        BranchInst *branchInst,
        BasicBlock *exitBlock) {

  if (this->flipOperandsToUseNonStrictPredicate) {
    auto opL = cmpToUpdate->getOperand(0);
    auto opR = cmpToUpdate->getOperand(1);
    cmpToUpdate->setOperand(0, opR);
    cmpToUpdate->setOperand(1, opL);
  }
  cmpToUpdate->setPredicate(this->nonStrictPredicate);

  if (this->flipBrSuccessorsToUseNonStrictPredicate) {
    branchInst->swapSuccessors();
  }

  assert(branchInst->getSuccessor(0) == exitBlock
         && "header br inst doesn't exit on true!");
  return;
}

void LoopGoverningIVUtility::cloneConditionalCheckFor(
    Value *recurrenceOfIV,
    Value *clonedCompareValue,
    BasicBlock *continueBlock,
    BasicBlock *exitBlock,
    IRBuilder<> &cloneBuilder) {

  /*
   * Create the comparison instruction.
   */
  auto cmpInst = cloneBuilder.CreateICmp(this->nonStrictPredicate,
                                         recurrenceOfIV,
                                         clonedCompareValue);

  /*
   * Add the conditional branch
   */
  cloneBuilder.CreateCondBr(cmpInst, exitBlock, continueBlock);

  return;
}

void LoopGoverningIVUtility::updateConditionToCheckIfWeHavePastExitValue(
    CmpInst *cmpToUpdate) {

  /*
   * Check if the loop is a while one and the value used to compare against the
   * exit condition value is not an instruction of the PHI of the loop governing
   * IV.
   */
  auto IV = this->attribution.getInductionVariable();
  if (this->isWhile
      && (!IV->getAllInstructions().count(
          this->attribution.getValueToCompareAgainstExitConditionValue()))) {
    cmpToUpdate->setPredicate(this->strictPredicate);
  }

  return;
}

std::vector<Instruction *> &LoopGoverningIVUtility::getConditionValueDerivation(
    void) {
  return conditionValueOrderedDerivation;
}

Value *LoopGoverningIVUtility::generateCodeToComputeTheTripCount(
    IRBuilder<> &builder) {

  /*
   * Fetch the start and last value.
   */
  auto IV = this->attribution.getInductionVariable();
  auto startValue = IV->getStartValue();
  auto lastValue = this->attribution.getExitConditionValue();

  /*
   * Compute the delta.
   */
  Value *delta = nullptr;
  if (IV->isStepValuePositive()) {
    delta = IV->getType()->isIntegerTy()
                ? builder.CreateSub(lastValue, startValue)
                : builder.CreateFSub(lastValue, startValue);
  } else {
    delta = IV->getType()->isIntegerTy()
                ? builder.CreateSub(startValue, lastValue)
                : builder.CreateFSub(startValue, lastValue);
  }

  /*
   * Compute the number of steps to reach the delta.
   */
  auto tripCount = builder.CreateUDiv(delta, IV->getSingleComputedStepValue());

  return tripCount;
}

Value *LoopGoverningIVUtility::
    generateCodeToComputePreviousValueUsedToCompareAgainstExitConditionValue(
        IRBuilder<> &builder,
        Value *currentIterationValue,
        Value *stepValue) {

  /*
   * Assert that the builder is pointing to an instruction within the loop.
   */
  // TODO

  /*
   * Generate the value that was used to compare against the exit condition
   * value in the last iteration.
   */
  auto prevIterationValue =
      this->generateCodeToComputeValueToUseForAnIterationAgo(
          builder,
          currentIterationValue,
          stepValue);

  return prevIterationValue;
}

Value *LoopGoverningIVUtility::generateCodeToComputeValueToUseForAnIterationAgo(
    IRBuilder<> &builder,
    Value *currentIterationValue,
    Value *stepValue) {

  /*
   * Check if the value used to compare against the exit condition value is the
   * PHI of the loop governing IV.
   */
  auto IV = this->attribution.getInductionVariable();
  if (this->attribution.getValueToCompareAgainstExitConditionValue()
      == IV->getLoopEntryPHI()) {

    /*
     * The value used is the PHI.
     * Hence, we must generate code to compute the value of the previous
     * iteration.
     */
    auto prevIterationValue =
        IV->getType()->isIntegerTy()
            ? builder.CreateSub(currentIterationValue, stepValue)
            : builder.CreateFSub(currentIterationValue, stepValue);

    return prevIterationValue;
  }

  /*
   * The value used to check whether we should exit the loop is the updated
   * value. Hence, the previous value is simply the current updated one.
   */
  return currentIterationValue;
}

Value *LoopGoverningIVUtility::generateCodeToDetermineLastIterationValue(
    IRBuilder<> &builder,
    Value *currentIterationValue,
    PHINode *lastIterationFlag,
    Value *stepValue) {

  auto IV = this->attribution.getInductionVariable();

  auto prevIterationValue =
      IV->getType()->isIntegerTy()
          ? builder.CreateSub(currentIterationValue, stepValue)
          : builder.CreateFSub(currentIterationValue, stepValue);

  /*
   * Decide which iteration value is the last iteration based on the
   * PHINode flag.
   */
  auto lastIterationSelect = builder.CreateSelect(lastIterationFlag,
                                                  currentIterationValue,
                                                  prevIterationValue);
  return lastIterationSelect;
}

void LoopGoverningIVUtility::
    updateConditionToCheckIfTheLastLoopIterationWasExecuted(
        bool ivInLeftOperand,
        CmpInst *condition) {
  assert(condition != nullptr);

  /*
   * Fetch the loop governing IV.
   */
  auto IV = this->attribution.getInductionVariable();
  assert(IV->isStepValueLoopInvariant());

  /*
   * Adjust the predicate.
   *
   * For example, assume the loop exit condition is i >= 100.
   * If the loop is exited, the previous iteration was the last loop iteration
   * iff on the previous iteration, i < 100.
   */
  auto newPredicate = condition->getPredicate();
  switch (condition->getPredicate()) {
    case CmpInst::Predicate::ICMP_SGE:
    case CmpInst::Predicate::ICMP_UGE:
    case CmpInst::Predicate::ICMP_SLE:
    case CmpInst::Predicate::ICMP_ULE:
      newPredicate = ivInLeftOperand ? condition->getInversePredicate()
                                     : this->strictPredicate;
      break;

    case CmpInst::Predicate::ICMP_SGT:
    case CmpInst::Predicate::ICMP_UGT:
    case CmpInst::Predicate::ICMP_SLT:
    case CmpInst::Predicate::ICMP_ULT:
      newPredicate = ivInLeftOperand ? this->nonStrictPredicate
                                     : condition->getInversePredicate();
      break;

    case CmpInst::Predicate::ICMP_EQ:
    case CmpInst::Predicate::ICMP_NE:
      newPredicate = condition->getInversePredicate();
      break;

    default:
      break;
  }
  condition->setPredicate(newPredicate);

  return;
}

} // namespace arcana::noelle
