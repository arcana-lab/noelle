/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "InductionVariables.hpp"

using namespace llvm;

InductionVariables::InductionVariables (LoopsSummary &LIS, ScalarEvolution &SE, SCCDAG &sccdag, LoopEnvironment &loopEnv)
  : loopToIVsMap{}, loopToGoverningIVMap{} {

  Function &F = *LIS.getLoopNestingTreeRoot()->getHeader()->getParent();
  ScalarEvolutionReferentialExpander referentialExpander(SE, F);

  for (auto &loop : LIS.loops) {
    loopToIVsMap[loop.get()] = std::set<InductionVariable *>();

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
      auto IV = new InductionVariable(loop.get(), SE, &phi, *sccContainingIV, loopEnv, referentialExpander); 
      if (!IV->getSimpleValueOfStepSize() && !IV->getComposableStepSize()) {
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

InductionVariables::~InductionVariables () {
  for (auto loopIVs : loopToIVsMap) {
    for (auto IV : loopIVs.second) {
      delete IV;
    }
  }
  loopToIVsMap.clear();
  loopToGoverningIVMap.clear();
}

std::set<InductionVariable *> &InductionVariables::getInductionVariables(LoopSummary &LS) {
  return loopToIVsMap.at(&LS);
}

InductionVariable *InductionVariables::getLoopGoverningInductionVariable (LoopSummary &LS) {
  if (loopToGoverningIVMap.find(&LS) == loopToGoverningIVMap.end()) return nullptr;
  return loopToGoverningIVMap.at(&LS);
}

InductionVariable::InductionVariable  (
  LoopSummary *LS,
  ScalarEvolution &SE,
  PHINode *headerPHI,
  SCC &scc,
  LoopEnvironment &loopEnv,
  ScalarEvolutionReferentialExpander &referentialExpander
) : scc{scc}, headerPHI{headerPHI}, startValue{nullptr},
    stepSize{nullptr}, compositeStepSize{nullptr}, expansionOfCompositeStepSize{},
    isStepLoopInvariant{false} {

  /*
   * Collect intermediate values of the IV within the loop (by traversing its strongly connected component)
   */
  std::queue<DGNode<Value> *> ivIntermediateValues;
  std::set<Value *> valuesVisited;
  ivIntermediateValues.push(scc.fetchNode(headerPHI));
  while (!ivIntermediateValues.empty()) {
    auto node = ivIntermediateValues.front();
    auto value = node->getT();
    ivIntermediateValues.pop();

    if (valuesVisited.find(value) != valuesVisited.end()) continue;
    valuesVisited.insert(value);

    if (auto phi = dyn_cast<PHINode>(value)) {
      this->PHIs.insert(phi);
      this->allInstructions.insert(cast<Instruction>(phi));
    } else if (auto I = dyn_cast<Instruction>(value)) {
      this->accumulators.insert(I);
      this->allInstructions.insert(I);
    }

    for (auto edge : node->getIncomingEdges()) {
      if (!edge->isDataDependence()) continue;
      if (!scc.isInternal(edge->getOutgoingT())) continue;
      ivIntermediateValues.push(edge->getOutgoingNode());
    }
  }

  /*
   * Fetch initial value of induction variable
   */
  auto bbs = LS->getBasicBlocks();
  for (auto i = 0; i < headerPHI->getNumIncomingValues(); ++i) {
    auto incomingBB = headerPHI->getIncomingBlock(i);
    if (bbs.find(incomingBB) == bbs.end()) {
      this->startValue = headerPHI->getIncomingValue(i);
      break;
    }
  }

  auto headerSCEV = SE.getSCEV(headerPHI);
  assert(headerSCEV->getSCEVType() == SCEVTypes::scAddRecExpr);
  auto stepSCEV = cast<SCEVAddRecExpr>(headerSCEV)->getStepRecurrence(SE);

  // auto M = headerPHI->getFunction()->getParent();
  // DataLayout DL(M);
  // const char name = 'a';
  // SCEVExpander *expander = new SCEVExpander(SE, DL, &name);

  std::set<Value *> valuesInScope{}; 
  std::set<Value *> valuesToReferenceAndNotExpand{};
  std::set<Value *> valuesInternalToLoop{};
  for (auto internalNodePair : scc.internalNodePairs()) {
    auto value = internalNodePair.first;
    valuesInScope.insert(value);
    valuesInternalToLoop.insert(value);
  }
  for (auto externalPair : scc.externalNodePairs()) {
    auto value = externalPair.first;
    valuesInScope.insert(value);
    valuesToReferenceAndNotExpand.insert(value);
    if (!isa<Instruction>(value) || bbs.find(cast<Instruction>(value)->getParent()) == bbs.end()) continue;
    valuesInternalToLoop.insert(value);
  }
  for (auto liveIn : loopEnv.getProducers()) {
    valuesInScope.insert(liveIn);
    valuesToReferenceAndNotExpand.insert(liveIn);
  }

  switch (stepSCEV->getSCEVType()) {
    case SCEVTypes::scConstant:
      this->stepSize = cast<SCEVConstant>(stepSCEV)->getValue();
      this->isStepLoopInvariant = true;
      break;
    case SCEVTypes::scUnknown:
      this->stepSize = cast<SCEVUnknown>(stepSCEV)->getValue();
      this->isStepLoopInvariant = true;
      if (valuesToReferenceAndNotExpand.find(this->stepSize) == valuesToReferenceAndNotExpand.end()) {
        this->stepSize = nullptr;
        this->isStepLoopInvariant = false;
      }
      break;
    default:

      // stepSCEV->print(errs() << "Referencing: "); errs() << "\n";
      auto stepSizeReferenceTree = referentialExpander.createReferenceTree(stepSCEV, valuesInScope);
      if (stepSizeReferenceTree) {
        // stepSizeReferenceTree->getSCEV()->print(errs() << "Expanding: "); errs() << "\n";
        auto tempBlock = BasicBlock::Create(headerPHI->getContext());
        IRBuilder<> tempBuilder(tempBlock);
        auto finalValue = referentialExpander.expandUsingReferenceValues(
          stepSizeReferenceTree,
          valuesToReferenceAndNotExpand,
          tempBuilder
        );
        if (finalValue) {
          this->isStepLoopInvariant = true;
          auto references = stepSizeReferenceTree->collectAllReferences();
          for (auto reference : references) {
            if (isa<SCEVAddRecExpr>(reference->getSCEV())) {
              if (!reference->getValue() || valuesInternalToLoop.find(reference->getValue()) != valuesInternalToLoop.end()) {
                this->isStepLoopInvariant = false;
                break;
              }
            }
          }

          this->compositeStepSize = stepSCEV;
          // finalValue->print(errs() << "Expanded final value: "); errs() << "\n";
          for (auto &I : *tempBlock) {
            expansionOfCompositeStepSize.push_back(&I);
          }
        }
      }
      break;
  }

  // TODO: Determine why this seg faults on the destructor of a ValueHandleBase
  // delete expander;
}

bool InductionVariable::isStepSizeLoopInvariant () {
  return isStepLoopInvariant;
}

InductionVariable::~InductionVariable () {
  BasicBlock *tempBlock = nullptr;
  // for (auto expandedInst : expansionOfCompositeStepSize) {
  //   tempBlock = expandedInst->getParent();
  //   expandedInst->eraseFromParent();
  // }
  if (tempBlock) {
    tempBlock->deleteValue();
  }
}

PHINode *InductionVariable::getHeaderPHI () {
  return headerPHI;
}

std::set<PHINode *> &InductionVariable::getPHIs() {
  return PHIs;
}

std::set<Instruction *> &InductionVariable::getAccumulators() {
  return accumulators;
}

std::set<Instruction *> &InductionVariable::getAllInstructions() {
  return allInstructions;
}

Value *InductionVariable::getStartAtHeader () {
  return startValue;
}

Value *InductionVariable::getSimpleValueOfStepSize () {
  return stepSize;
}

const SCEV *InductionVariable::getComposableStepSize () {
  return compositeStepSize;
}

std::vector<Instruction *> InductionVariable::getExpansionOfCompositeStepSize() {
  return expansionOfCompositeStepSize;
}

/*
 * LoopGoverningIVUtility implementation
 */

LoopGoverningIVAttribution::LoopGoverningIVAttribution (InductionVariable &iv, SCC &scc, std::vector<BasicBlock *> &exitBlocks)
  : IV{iv}, scc{scc}, headerCmp{nullptr}, conditionValueDerivation{}, isWellFormed{false} {

  /*
   * To understand how to transform the loop governing condition, it is far simpler to
   * know the sign of the step size at compile time. Extra overhead is necessary if this
   * is only known at runtime, and that enhancement has yet to be made
   */
  if (!iv.getSimpleValueOfStepSize() || !isa<ConstantInt>(iv.getSimpleValueOfStepSize())) return;
  // iv.getHeaderPHI()->print(errs() << "Has step size: "); errs() << "\n";

  auto headerPHI = iv.getHeaderPHI();
  auto &ivInstructions = iv.getAllInstructions();

  /*
   * Fetch the header terminator.
   */
  auto headerTerminator = headerPHI->getParent()->getTerminator();
  if (!isa<BranchInst>(headerTerminator)) return;
  this->headerBr = cast<BranchInst>(headerTerminator);
  // iv.getHeaderPHI()->print(errs() << "Has branch: "); errs() << "\n";

  /*
   * Check this is a conditional branch.
   * If it isn't, then we don't handle this type of loops (e.g., do-while).
   */
  if (!headerBr->isConditional()){
    return ;
  }

  /*
   * Fetch the condition of the conditional branch
   */
  auto headerCondition = headerBr->getCondition();
  if (!isa<CmpInst>(headerCondition)) return;

  this->headerCmp = cast<CmpInst>(headerCondition);
  auto opL = headerCmp->getOperand(0), opR = headerCmp->getOperand(1);
  if (!(opL == headerPHI ^ opR == headerPHI)) return;
  this->conditionValue = opL == headerPHI ? opR : opL;
  // iv.getHeaderPHI()->print(errs() << "Has condition: "); errs() << "\n";

  std::set<BasicBlock *> exitBlockSet(exitBlocks.begin(), exitBlocks.end());
  if (exitBlockSet.find(headerBr->getSuccessor(0)) != exitBlockSet.end()) {
    this->exitBlock = headerBr->getSuccessor(0);
  } else if (exitBlockSet.find(headerBr->getSuccessor(1)) != exitBlockSet.end()) {
    this->exitBlock = headerBr->getSuccessor(1);
  } else return ;
  // iv.getHeaderPHI()->print(errs() << "Has one exit: "); errs() << "\n";

  if (scc.isInternal(conditionValue)) {
    std::queue<Instruction *> conditionDerivation;
    assert(isa<Instruction>(conditionValue)
      && "An internal value to an IV's SCC must be an instruction!");
    conditionDerivation.push(cast<Instruction>(conditionValue));

    while (!conditionDerivation.empty()) {
      auto value = conditionDerivation.front();
      conditionDerivation.pop();

      for (auto edge : scc.fetchNode(value)->getIncomingEdges()) {
        if (!edge->isDataDependence()) continue;

        auto outgoingValue = edge->getOutgoingT();
        if (scc.isInternal(outgoingValue)) {
          assert(isa<Instruction>(outgoingValue)
            && "An internal value to an IV's SCC must be an instruction!");
          auto outgoingInst = cast<Instruction>(outgoingValue);

          /*
           * The exit condition value cannot be itself derived from the induction variable 
           */
          if (ivInstructions.find(outgoingInst) != ivInstructions.end()) {
            // outgoingInst->print(errs() << "Exit condition depends on IV: "); errs() << "\n";
            return;
          }

          conditionValueDerivation.insert(outgoingInst);
          conditionDerivation.push(outgoingInst);
        }
      }
    }
  }

  // iv.getHeaderPHI()->print(errs() << "Is well formed: "); errs() << "\n";
  isWellFormed = true;
}

InductionVariable &LoopGoverningIVAttribution::getInductionVariable() {
  return IV;
}

CmpInst *LoopGoverningIVAttribution::getHeaderCmpInst() {
  return headerCmp;
}

Value *LoopGoverningIVAttribution::getHeaderCmpInstConditionValue() {
  return conditionValue;
}

BranchInst *LoopGoverningIVAttribution::getHeaderBrInst() {
  return headerBr;
}

BasicBlock *LoopGoverningIVAttribution::getExitBlockFromHeader() {
  return exitBlock;
}

bool LoopGoverningIVAttribution::isSCCContainingIVWellFormed() {
  return isWellFormed;
}

std::set<Instruction *> &LoopGoverningIVAttribution::getConditionValueDerivation() { 
  return conditionValueDerivation;
}

/*
 * IVUtility implementation
 */

PHINode *IVUtility::createChunkPHI (BasicBlock *preheaderB, BasicBlock *headerB, Type *chunkPHIType, Value *chunkSize) {

  // TODO: Add asserts to ensure the basic blocks/terminators are well formed

  std::vector<BasicBlock *> headerPreds(pred_begin(headerB), pred_end(headerB));
  IRBuilder<> headerBuilder(headerB->getFirstNonPHIOrDbgOrLifetime());
  auto chunkPHI = headerBuilder.CreatePHI(chunkPHIType, headerPreds.size());
  auto zeroValueForChunking = ConstantInt::get(chunkPHIType, 0);
  auto onesValueForChunking = ConstantInt::get(chunkPHIType, 1);

  for (auto B : headerPreds) {
    IRBuilder<> latchBuilder(B->getTerminator());

    if (preheaderB == B) {
      chunkPHI->addIncoming(zeroValueForChunking, B);
    } else {
      auto chunkIncrement = latchBuilder.CreateAdd(chunkPHI, onesValueForChunking);
      auto isChunkCompleted = latchBuilder.CreateICmp(CmpInst::Predicate::ICMP_EQ, chunkIncrement, chunkSize);
      auto chunkWrap = latchBuilder.CreateSelect(isChunkCompleted, zeroValueForChunking, chunkIncrement, "chunkWrap");
      chunkPHI->addIncoming(chunkWrap, B);
    }
  }

  return chunkPHI;
}

void IVUtility::chunkInductionVariablePHI(
  BasicBlock *preheaderBlock,
  PHINode *ivPHI,
  PHINode *chunkPHI,
  Value *chunkStepSize) {

  for (auto i = 0; i < ivPHI->getNumIncomingValues(); ++i) {
    auto B = ivPHI->getIncomingBlock(i);
    IRBuilder<> latchBuilder(B->getTerminator());
    if (preheaderBlock == B) continue;

    auto chunkIncomingIdx = chunkPHI->getBasicBlockIndex(B);
    Value *isChunkCompleted = cast<SelectInst>(chunkPHI->getIncomingValue(chunkIncomingIdx))->getCondition();

    /*
      * Iterate to next chunk if necessary
      */
    ivPHI->setIncomingValue(i, latchBuilder.CreateSelect(
      isChunkCompleted,
      latchBuilder.CreateAdd(ivPHI->getIncomingValue(i), chunkStepSize),
      ivPHI->getIncomingValue(i),
      "nextStepOrNextChunk"
    ));

  }
}

/*
 * LoopGoverningIVUtility implementation
 */

LoopGoverningIVUtility::LoopGoverningIVUtility (InductionVariable &IV, LoopGoverningIVAttribution &attribution)
  : attribution{attribution}, conditionValueOrderedDerivation{}, flipOperandsToUseNonStrictPredicate{false} {

  condition = attribution.getHeaderCmpInst();
  this->doesOriginalCmpInstHaveIVAsLeftOperand = condition->getOperand(0) == IV.getHeaderPHI();

  auto &conditionValueDerivationSet = attribution.getConditionValueDerivation();
  for (auto &I : *condition->getParent()) {
    if (conditionValueDerivationSet.find(&I) == conditionValueDerivationSet.end()) continue;
    conditionValueOrderedDerivation.push_back(&I);
  }

  assert(IV.getSimpleValueOfStepSize() && isa<ConstantInt>(IV.getSimpleValueOfStepSize()));
  bool isStepValuePositive = cast<ConstantInt>(IV.getSimpleValueOfStepSize())->getValue().isStrictlyPositive();
  bool conditionExitsOnTrue = attribution.getHeaderBrInst()->getSuccessor(0) == attribution.getExitBlockFromHeader();
  auto exitPredicate = conditionExitsOnTrue ? condition->getPredicate() : condition->getInversePredicate();
  // errs() << "Exit predicate before operand check: " << exitPredicate << "\n";
  exitPredicate = doesOriginalCmpInstHaveIVAsLeftOperand ? exitPredicate : CmpInst::getSwappedPredicate(exitPredicate);
  this->flipOperandsToUseNonStrictPredicate = !doesOriginalCmpInstHaveIVAsLeftOperand;
  // errs() << "Exit predicate after: " << exitPredicate << "\n";

  // condition->print(errs() << "Condition (exits on true: " << conditionExitsOnTrue << "): "); errs() << "\n";

  switch (exitPredicate) {
    case CmpInst::Predicate::ICMP_NE:
      // This predicate is non-strict and will result in either 0 or 1 iteration(s)
      this->nonStrictPredicate = exitPredicate;
      break;
    case CmpInst::Predicate::ICMP_EQ:
      // This predicate is strict and needs to be extended to LTE/GTE to catch jumping past the exiting value
      this->nonStrictPredicate = isStepValuePositive
        ? CmpInst::Predicate::ICMP_UGE : CmpInst::Predicate::ICMP_ULE;
      break;
    case CmpInst::Predicate::ICMP_SLE:
    case CmpInst::Predicate::ICMP_SLT:
    case CmpInst::Predicate::ICMP_ULT:
    case CmpInst::Predicate::ICMP_ULE:
      // This predicate is non-strict. We simply assert that the step value has the expected sign

      // HACK: while it is technically correct to increment with a less than exit condition, yielding 0 or 1 iteration,
      // it would break under assumptions that further recurrences of the IV can be checked on this condition
      // Our parallelization schemes make that assumption, hence the assert here
      assert(!isStepValuePositive && "IV step value is not compatible with exit condition!");
      this->nonStrictPredicate = exitPredicate;
      break;
    case CmpInst::Predicate::ICMP_UGT:
    case CmpInst::Predicate::ICMP_UGE:
    case CmpInst::Predicate::ICMP_SGT:
    case CmpInst::Predicate::ICMP_SGE:
      // This predicate is non-strict. We simply assert that the step value has the expected sign

      // HACK: while it is technically correct to decrement with a greater than exit condition, yielding 0 or 1 iteration,
      // it would break under assumptions that further recurrences of the IV can be checked on this condition
      // Our parallelization schemes make that assumption, hence the assert here
      assert(isStepValuePositive && "IV step value is not compatible with exit condition!");
      this->nonStrictPredicate = exitPredicate;
      break;
  }

}

void LoopGoverningIVUtility::updateConditionAndBranchToCatchIteratingPastExitValue(
  CmpInst *cmpToUpdate,
  BranchInst *branchInst,
  BasicBlock *exitBlock) {

  if (flipOperandsToUseNonStrictPredicate) {
    auto opL = cmpToUpdate->getOperand(0);
    auto opR = cmpToUpdate->getOperand(1);
    cmpToUpdate->setOperand(0, opR);
    cmpToUpdate->setOperand(1, opL);
  }
  cmpToUpdate->setPredicate(nonStrictPredicate);

  if (branchInst->getSuccessor(0) != exitBlock) {
    branchInst->setSuccessor(1, branchInst->getSuccessor(0));
    branchInst->setSuccessor(0, exitBlock);
  }
}

void LoopGoverningIVUtility::cloneConditionalCheckFor(
  Value *recurrenceOfIV,
  Value *clonedCompareValue,
  BasicBlock *continueBlock,
  BasicBlock *exitBlock,
  IRBuilder<> &cloneBuilder) {

  Value *cmpInst;
  cmpInst = cloneBuilder.CreateICmp(nonStrictPredicate, recurrenceOfIV, clonedCompareValue);
  cloneBuilder.CreateCondBr(cmpInst, exitBlock, continueBlock);
}

std::vector<Instruction *> &LoopGoverningIVUtility::getConditionValueDerivation () {
  return conditionValueOrderedDerivation;
}