/*
 * Copyright 2016 - 2024  Angelo Matni, Simone Campanoni
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
#include "arcana/noelle/core/InductionVariable.hpp"
#include "arcana/noelle/core/MemoryDependence.hpp"

namespace arcana::noelle {

InductionVariable::InductionVariable(
    LoopStructure *LS,
    InvariantManager &IVM,
    ScalarEvolution &SE,
    PHINode *loopEntryPHI,
    SCC &scc,
    LoopEnvironment &loopEnv,
    ScalarEvolutionReferentialExpander &referentialExpander,
    InductionDescriptor &ID)
  : scc{ scc },
    loopEntryPHI{ loopEntryPHI },
    stepPHIs{ std::unordered_set<PHINode *>({ loopEntryPHI }) },
    startValue{ ID.getStartValue() },
    stepSCEV{ ID.getStep() },
    singleStepValue{ ID.getConstIntStepValue() },
    stepMultiplier{ 1 },
    computationOfStepValue{},
    isComputedStepValueLoopInvariant{ false },
    loopEntryPHIType{ loopEntryPHI->getType() } {

  traverseCycleThroughLoopEntryPHIToGetAllIVInstructions(LS);
  traverseConsumersOfIVInstructionsToGetAllDerivedSCEVInstructions(LS, IVM, SE);
  collectValuesInternalAndExternalToLoopAndSCC(LS, loopEnv);

  if (ID.getKind() == InductionDescriptor::InductionKind::IK_FpInduction) {
    this->singleStepValue = cast<SCEVUnknown>(stepSCEV)->getValue();
    this->isComputedStepValueLoopInvariant = true;

  } else {
    deriveStepValue(LS, SE, referentialExpander, stepMultiplier);
  }
}

InductionVariable::InductionVariable(
    LoopStructure *LS,
    InvariantManager &IVM,
    ScalarEvolution &SE,
    int64_t stepMultiplier,
    PHINode *loopEntryPHI,
    std::unordered_set<PHINode *> stepPHIs,
    SCC &scc,
    LoopEnvironment &loopEnv,
    ScalarEvolutionReferentialExpander &referentialExpander)
  : scc{ scc },
    loopEntryPHI{ loopEntryPHI },
    stepPHIs{ stepPHIs },
    PHIs{},
    nonPHIIntermediateValues{},
    allInstructions{},
    derivedSCEVInstructions{},
    startValue{ nullptr },
    stepSCEV{ nullptr },
    singleStepValue{ nullptr },
    stepMultiplier{ stepMultiplier },
    computationOfStepValue{},
    isComputedStepValueLoopInvariant{ false },
    loopEntryPHIType{ loopEntryPHI->getType() },
    valuesToReferenceInComputingStepValue{},
    valuesInScopeOfInductionVariable{} {

  /*
   * Fetch initial value of induction variable
   */
  auto bbs = LS->getBasicBlocks();
  for (auto i = 0u; i < loopEntryPHI->getNumIncomingValues(); ++i) {
    auto incomingBB = loopEntryPHI->getIncomingBlock(i);
    if (bbs.find(incomingBB) == bbs.end()) {
      this->startValue = loopEntryPHI->getIncomingValue(i);
      break;
    }
  }

  traverseCycleThroughLoopEntryPHIToGetAllIVInstructions(LS);
  traverseConsumersOfIVInstructionsToGetAllDerivedSCEVInstructions(LS, IVM, SE);
  collectValuesInternalAndExternalToLoopAndSCC(LS, loopEnv);
  deriveStepValue(LS, SE, referentialExpander, stepMultiplier);

  return;
}

void InductionVariable::traverseCycleThroughLoopEntryPHIToGetAllIVInstructions(
    LoopStructure *LS) {

  /*
   * Collect intermediate values of the IV within the loop (by traversing its
   * strongly connected component) Traverse data dependencies the header PHI
   * has.
   */
  std::queue<DGNode<Value> *> ivIntermediateValues;
  std::set<Value *> valuesVisited;
  ivIntermediateValues.push(scc.fetchNode(loopEntryPHI));

  while (!ivIntermediateValues.empty()) {
    auto node = ivIntermediateValues.front();
    auto value = node->getT();
    ivIntermediateValues.pop();

    if (valuesVisited.find(value) != valuesVisited.end())
      continue;
    valuesVisited.insert(value);

    /*
     * Classify the encountered value as either a PHI or a non-PHI intermediate
     * instruction If it is not an instruction, skip If it is not within the
     * IV's loop, skip
     */
    if (!isa<Instruction>(value))
      continue;
    auto instruction = cast<Instruction>(value);
    if (!LS->isIncluded(instruction))
      continue;
    this->allInstructions.insert(instruction);
    if (auto phi = dyn_cast<PHINode>(instruction)) {
      this->PHIs.insert(phi);
    } else {
      this->nonPHIIntermediateValues.insert(instruction);
    }

    /*
     * Traverse all dependencies this instruction has that are internal
     * to the SCC; they are transitive dependencies of the loop entry PHI
     * and thus must be intermediate values
     */
    for (auto edge : node->getIncomingEdges()) {
      if (!isa<DataDependence<Value, Value>>(edge)) {
        continue;
      }
      if (isa<MemoryDependence<Value, Value>>(edge)) {
        continue;
      }
      auto otherNode = edge->getSrcNode();
      auto otherValue = otherNode->getT();
      if (!scc.isInternal(otherValue))
        continue;
      ivIntermediateValues.push(otherNode);
    }
  }

  /*
   * Include any casts on intermediate values
   * TODO: Determine what other instructions could still represent the induction
   * variable but not necessarily appear in the SCC for that induction variable
   */
  std::set<CastInst *> castsToAdd{};
  for (auto intermediateValue : this->allInstructions) {
    for (auto user : intermediateValue->users()) {
      if (auto castInst = dyn_cast<CastInst>(user)) {
        if (!LS->isIncluded(castInst))
          continue;
        castsToAdd.insert(castInst);
      }
    }
  }
  this->allInstructions.insert(castsToAdd.begin(), castsToAdd.end());

  return;
}

void InductionVariable::
    traverseConsumersOfIVInstructionsToGetAllDerivedSCEVInstructions(
        LoopStructure *LS,
        InvariantManager &IVM,
        ScalarEvolution &SE) {

  /*
   * Recursive search up uses of an instruction to determine if derived
   * Since we do not have the SCC that pertains to children IVs, we only
   * label acyclic dependent computation on this IV as "derived"
   */
  std::unordered_set<Instruction *> checked;
  std::function<bool(Instruction *)> checkIfDerived;
  checkIfDerived = [&](Instruction *I) -> bool {
    /*
     * Check the cache of confirmed derived values,
     * and then what we have already traversed to prevent traversing a cycle
     */
    if (derivedSCEVInstructions.find(I) != derivedSCEVInstructions.end()) {
      return true;
    }
    if (checked.find(I) != checked.end()) {
      return false;
    }
    checked.insert(I);

    /*
     * Only check SCEVable values in the loop
     */
    if (!SE.isSCEVable(I->getType()))
      return false;
    if (!LS->isIncluded(I))
      return false;

    /*
     * We only handle unary/binary operations on IV instructions.
     */
    auto scev = SE.getSCEV(I);
    if (!isa<SCEVCastExpr>(scev) && !isa<SCEVNAryExpr>(scev)
        && !isa<SCEVUDivExpr>(scev))
      return false;

    /*
     * Ensure the instruction uses the IV at least once, and only this IV,
     * apart from constants and loop invariants
     */
    bool usesAtLeastOneIVInstruction = false;
    for (auto &use : I->operands()) {
      auto usedValue = use.get();

      if (isa<ConstantInt>(usedValue))
        continue;
      if (IVM.isLoopInvariant(usedValue))
        continue;

      if (auto usedInst = dyn_cast<Instruction>(usedValue)) {
        if (!LS->isIncluded(usedInst))
          continue;
        auto isIVUse = this->isIVInstruction(usedInst);
        auto isDerivedUse = checkIfDerived(usedInst);
        if (isIVUse || isDerivedUse) {
          usesAtLeastOneIVInstruction = true;
          continue;
        }
      }

      return false;
    }

    if (!usesAtLeastOneIVInstruction)
      return false;

    /*
     * Cache the result
     */
    derivedSCEVInstructions.insert(I);

    return true;
  };

  /*
   * Queue traversal through users of IV instructions to find all derived
   * instructions
   */
  std::queue<Instruction *> intermediates;
  std::unordered_set<Instruction *> visited;
  for (auto ivInst : allInstructions) {
    intermediates.push(ivInst);
    visited.insert(ivInst);
  }

  while (!intermediates.empty()) {
    auto I = intermediates.front();
    intermediates.pop();

    for (auto user : I->users()) {
      if (auto userInst = dyn_cast<Instruction>(user)) {
        if (visited.find(userInst) != visited.end())
          continue;
        visited.insert(userInst);

        /*
         * If the user isn't derived, do not continue traversing users
         */
        if (!checkIfDerived(userInst))
          continue;
        intermediates.push(userInst);
      }
    }
  }

  return;
}

void InductionVariable::collectValuesInternalAndExternalToLoopAndSCC(
    LoopStructure *LS,
    LoopEnvironment &loopEnvironment) {

  auto bbs = LS->getBasicBlocks();

  /*
   * Values internal to the IV's SCC are in scope but should
   * NOT be referenced when computing the IV's step value
   */
  for (auto internalNodePair : scc.internalNodePairs()) {
    auto value = internalNodePair.first;
    valuesInScopeOfInductionVariable.insert(value);
  }

  /*
   * Values external to the IV's SCC are in scope
   *
   * HACK: they should be referenced when computing the IV's step value
   * even if they aren't loop external, but that would require a more
   * powerful way to distinguish instructions in the loop that are
   * still loop invariant, which isn't possible at this time. Therefore,
   * we force the expansion of all but live in values. In turn, the expander
   * will return that it could not expand SCEVAddRecExpr, exiting gracefully.
   */
  for (auto externalPair : scc.externalNodePairs()) {
    auto value = externalPair.first;
    valuesInScopeOfInductionVariable.insert(value);
  }

  /*
   * All live ins are in scope and should be referenced
   */
  for (auto liveIn : loopEnvironment.getProducers()) {
    valuesInScopeOfInductionVariable.insert(liveIn);
    valuesToReferenceInComputingStepValue.insert(liveIn);
  }
}

/*
 * Examine the step recurrence SCEV and either retrieve the single value
 * representing the SCEV or expand values to represent it
 */
void InductionVariable::deriveStepValue(
    LoopStructure *LS,
    ScalarEvolution &SE,
    ScalarEvolutionReferentialExpander &referentialExpander,
    int64_t multiplier) {

  /*
   * Fetch the SCEV for the step value.
   */
  if (!this->stepSCEV) {

    /*
     * Here, stepSCEV is being defined without using LLVM InductionDescriptor.
     * Note: We currently don't identify IVs that have more than one PHI/SCEV
     * involved in the calculation of its step.
     */
    assert(this->stepPHIs.size() == 1
           && "Not one PHI for step value calculation!\n");
    auto stepSCEVPHI = *(stepPHIs.begin());
    assert(SE.getSCEV(stepSCEVPHI)->getSCEVType() == SCEVTypes::scAddRecExpr);
    this->stepSCEV =
        cast<SCEVAddRecExpr>(SE.getSCEV(stepSCEVPHI))->getStepRecurrence(SE);
  }
  assert(this->stepSCEV != nullptr && "stepSCEV is nullptr!\n");

  /*
   * Analyze the SCEV about the step.
   */
  switch (stepSCEV->getSCEVType()) {

    case SCEVTypes::scConstant:
      deriveStepValueFromSCEVConstant(cast<SCEVConstant>(stepSCEV), multiplier);
      break;

    case SCEVTypes::scUnknown:
      deriveStepValueFromSCEVUnknown(cast<SCEVUnknown>(stepSCEV), LS);
      break;

    case SCEVTypes::scAddExpr:
    case SCEVTypes::scAddRecExpr:
    case SCEVTypes::scMulExpr:
    case SCEVTypes::scSignExtend:
    case SCEVTypes::scSMaxExpr:
    case SCEVTypes::scSMinExpr:
    case SCEVTypes::scTruncate:
    case SCEVTypes::scUDivExpr:
    case SCEVTypes::scUMaxExpr:
    case SCEVTypes::scUMinExpr:
    case SCEVTypes::scZeroExtend:

      /*
       * Not all composite SCEVs are handled, so if the derivation fails,
       * do not claim understanding of the step recurrence
       */
      if (!deriveStepValueFromCompositeSCEV(stepSCEV,
                                            referentialExpander,
                                            LS)) {
        this->stepSCEV = nullptr;
      }
      break;

    default:
      this->stepSCEV = nullptr;
      break;
  }

  return;
}

void InductionVariable::deriveStepValueFromSCEVConstant(
    const SCEVConstant *scev,
    int64_t multiplier) {
  if (auto CI = dyn_cast<ConstantInt>(scev->getValue())) {
    this->singleStepValue = ConstantInt::get(scev->getValue()->getType(),
                                             multiplier * CI->getSExtValue());
  } else
    this->singleStepValue = scev->getValue();
  this->isComputedStepValueLoopInvariant = true;
}

void InductionVariable::deriveStepValueFromSCEVUnknown(const SCEVUnknown *scev,
                                                       LoopStructure *LS) {
  this->singleStepValue = scev->getValue();
  this->isComputedStepValueLoopInvariant =
      LS->isLoopInvariant(this->singleStepValue);
}

bool InductionVariable::deriveStepValueFromCompositeSCEV(
    const SCEV *scev,
    ScalarEvolutionReferentialExpander &referentialExpander,
    LoopStructure *LS) {

  // auto M = headerPHI->getFunction()->getParent();
  // DataLayout DL(M);
  // const char name = 'a';
  // SCEVExpander *expander = new SCEVExpander(SE, DL, &name);

  // stepSCEV->print(errs() << "Referencing: "); errs() << "\n";
  auto stepSizeReferenceTree =
      referentialExpander.createReferenceTree(scev,
                                              valuesInScopeOfInductionVariable);
  if (!stepSizeReferenceTree) {
    return false;
  }

  // stepSizeReferenceTree->getSCEV()->print(errs() << "Expanding: "); errs() <<
  // "\n";
  auto tempBlock = BasicBlock::Create(loopEntryPHI->getContext(),
                                      "temp_basic_block",
                                      LS->getFunction());
  IRBuilder<> tempBuilder(tempBlock);
  auto finalValue = referentialExpander.expandUsingReferenceValues(
      stepSizeReferenceTree,
      valuesToReferenceInComputingStepValue,
      tempBuilder);
  if (!finalValue) {
    tempBlock->eraseFromParent();
    return false;
  }

  this->isComputedStepValueLoopInvariant = true;
  auto references = stepSizeReferenceTree->collectAllReferences();
  // TODO: Only check leaf reference values
  for (auto reference : references) {
    auto referenceValue = reference->getValue();
    if (!referenceValue) {
      this->isComputedStepValueLoopInvariant = false;
      break;
    }
    if (!LS->isLoopInvariant(referenceValue)) {
      this->isComputedStepValueLoopInvariant = false;
      break;
    }
  }

  // finalValue->print(errs() << "Expanded final value: "); errs() << "\n";

  /*
   * If no instruction was expanded (where a value is referenced instead)
   * OR
   * if only one instruction was expanded to represent the step recurrence
   * then save that single value
   */
  if (tempBlock->size() < 2) {
    this->singleStepValue = finalValue;
  }

  /*
   * Save expanded values that compute the step recurrence
   */
  for (auto &I : *tempBlock) {
    computationOfStepValue.push_back(&I);
  }

  /*
   * Free the memory is no longer needed.
   */
  tempBlock->eraseFromParent();

  return true;
}

InductionVariable::~InductionVariable() {
  BasicBlock *tempBlock = nullptr;
  if (tempBlock) {
    tempBlock->deleteValue();
  }
}

SCC *InductionVariable::getSCC(void) const {
  return &scc;
}

PHINode *InductionVariable::getLoopEntryPHI(void) const {
  return loopEntryPHI;
}

std::unordered_set<PHINode *> InductionVariable::
    getPHIsInvolvedInComputingIVStep(void) const {
  return stepPHIs;
}

std::unordered_set<PHINode *> InductionVariable::getPHIs(void) const {
  return PHIs;
}

std::unordered_set<Instruction *> InductionVariable::
    getNonPHIIntermediateValues(void) const {
  return nonPHIIntermediateValues;
}

std::unordered_set<Instruction *> InductionVariable::getAllInstructions(
    void) const {
  return allInstructions;
}

std::unordered_set<Instruction *> InductionVariable::getDerivedSCEVInstructions(
    void) const {
  return derivedSCEVInstructions;
}

Value *InductionVariable::getStartValue(void) const {
  return startValue;
}

Value *InductionVariable::getSingleComputedStepValue(void) const {
  return singleStepValue;
}

const SCEV *InductionVariable::getStepSCEV(void) const {
  return stepSCEV;
}

std::vector<Instruction *> InductionVariable::getComputationOfStepValue(
    void) const {
  return computationOfStepValue;
}

bool InductionVariable::isStepValueLoopInvariant(void) const {
  return isComputedStepValueLoopInvariant;
}

bool InductionVariable::isIVInstruction(Instruction *I) const {
  return allInstructions.find(I) != allInstructions.end();
}

bool InductionVariable::isDerivedFromIVInstructions(Instruction *I) const {
  return derivedSCEVInstructions.find(I) != derivedSCEVInstructions.end();
}

bool InductionVariable::isStepValuePositive(void) const {

  /*
   * Fetch the step value.
   */
  assert(this->isComputedStepValueLoopInvariant);
  auto stepValue = this->getSingleComputedStepValue();

  /*
   * Check if the step value is positive
   *
   * Check if the value is an integer
   */
  if (this->loopEntryPHIType->isIntegerTy()) {
    auto constant = cast<ConstantInt>(stepValue);
    auto constantValue = constant->getValue();
    auto isPositive = constantValue.isStrictlyPositive();
    return isPositive;
  }

  /*
   * The value is a floating point one
   */
  assert(this->loopEntryPHIType->isFloatingPointTy());
  auto fpValue = cast<ConstantFP>(stepValue)->getValueAPF();
  return fpValue.isNonZero() && !fpValue.isNegative();
}

Type *InductionVariable::getType(void) const {
  return loopEntryPHIType;
}

} // namespace arcana::noelle
