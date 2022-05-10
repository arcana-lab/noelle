/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/InductionVariables.hpp"
#include "noelle/core/LoopGoverningIVAttribution.hpp"

namespace llvm::noelle{

InductionVariable::InductionVariable  (
  LoopStructure *LS,
  InvariantManager &IVM,
  ScalarEvolution &SE,
  PHINode *loopEntryPHI,
  SCC &scc,
  LoopEnvironment &loopEnv,
  ScalarEvolutionReferentialExpander &referentialExpander,
  InductionDescriptor &ID
) : scc{scc}, loopEntryPHI{loopEntryPHI}, startValue{ID.getStartValue()}, loopEntryPHIType{loopEntryPHI->getType()},
    stepSCEV{ID.getStep()}, computationOfStepValue{}, singleStepValue{ID.getConstIntStepValue()}, isComputedStepValueLoopInvariant{false} {

  traverseCycleThroughLoopEntryPHIToGetAllIVInstructions(LS);
  traverseConsumersOfIVInstructionsToGetAllDerivedSCEVInstructions(LS, IVM, SE);
  collectValuesInternalAndExternalToLoopAndSCC(LS, loopEnv);

  if (ID.getKind() == InductionDescriptor::InductionKind::IK_FpInduction) {
    this->singleStepValue = cast<SCEVUnknown>(stepSCEV)->getValue();
    this->isComputedStepValueLoopInvariant = true;
  } else {
    deriveStepValue(LS, SE, referentialExpander);
  }
}

InductionVariable::InductionVariable  (
  LoopStructure *LS,
  InvariantManager &IVM,
  ScalarEvolution &SE,
  PHINode *loopEntryPHI,
  SCC &scc,
  LoopEnvironment &loopEnv,
  ScalarEvolutionReferentialExpander &referentialExpander
) : scc{scc}, loopEntryPHI{loopEntryPHI}, startValue{nullptr}, loopEntryPHIType{loopEntryPHI->getType()},
    stepSCEV{nullptr}, computationOfStepValue{}, isComputedStepValueLoopInvariant{false} {

  /*
   * Fetch initial value of induction variable
   */
  auto bbs = LS->getBasicBlocks();
  for (auto i = 0; i < loopEntryPHI->getNumIncomingValues(); ++i) {
    auto incomingBB = loopEntryPHI->getIncomingBlock(i);
    if (bbs.find(incomingBB) == bbs.end()) {
      this->startValue = loopEntryPHI->getIncomingValue(i);
      break;
    }
  }

  traverseCycleThroughLoopEntryPHIToGetAllIVInstructions(LS);
  traverseConsumersOfIVInstructionsToGetAllDerivedSCEVInstructions(LS, IVM, SE);
  collectValuesInternalAndExternalToLoopAndSCC(LS, loopEnv);
  deriveStepValue(LS, SE, referentialExpander);

  return ;
}

void InductionVariable::traverseCycleThroughLoopEntryPHIToGetAllIVInstructions (LoopStructure *LS) {

  /*
   * Collect intermediate values of the IV within the loop (by traversing its strongly connected component)
   * Traverse data dependencies the header PHI has.
   */
  std::queue<DGNode<Value> *> ivIntermediateValues;
  std::set<Value *> valuesVisited;
  ivIntermediateValues.push(scc.fetchNode(loopEntryPHI));

  while (!ivIntermediateValues.empty()) {
    auto node = ivIntermediateValues.front();
    auto value = node->getT();
    ivIntermediateValues.pop();

    if (valuesVisited.find(value) != valuesVisited.end()) continue;
    valuesVisited.insert(value);

    /*
     * Classify the encountered value as either a PHI or a non-PHI intermediate instruction
     * If it is not an instruction, skip
     * If it is not within the IV's loop, skip
     */
    if (!isa<Instruction>(value)) continue;
    auto instruction = cast<Instruction>(value);
    if (!LS->isIncluded(instruction)) continue;
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
      if (!edge->isDataDependence() || edge->isMemoryDependence()) continue;
      auto otherNode = edge->getOutgoingNode();
      auto otherValue = otherNode->getT();
      if (!scc.isInternal(otherValue)) continue;
      ivIntermediateValues.push(otherNode);
    }
  }

  /*
   * Include any casts on intermediate values
   * TODO: Determine what other instructions could still represent the induction variable
   * but not necessarily appear in the SCC for that induction variable
   */
  std::set<CastInst *> castsToAdd{};
  for (auto intermediateValue : this->allInstructions) {
    for (auto user : intermediateValue->users()) {
      if (auto castInst = dyn_cast<CastInst>(user)) {
        if (!LS->isIncluded(castInst)) continue;
        castsToAdd.insert(castInst);
      }
    }
  }
  this->allInstructions.insert(castsToAdd.begin(), castsToAdd.end());

  return;
}

void InductionVariable::traverseConsumersOfIVInstructionsToGetAllDerivedSCEVInstructions (
  LoopStructure *LS,
  InvariantManager &IVM,
  ScalarEvolution &SE
) {

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
    if (!SE.isSCEVable(I->getType())) return false;
    if (!LS->isIncluded(I)) return false;

    /*
     * We only handle unary/binary operations on IV instructions.
     */
    auto scev = SE.getSCEV(I);
    if (!isa<SCEVCastExpr>(scev) && !isa<SCEVNAryExpr>(scev) && !isa<SCEVUDivExpr>(scev)) return false;

    /*
     * Ensure the instruction uses the IV at least once, and only this IV,
     * apart from constants and loop invariants
     */
    bool usesAtLeastOneIVInstruction = false;
    for (auto &use : I->operands()) {
      auto usedValue = use.get();

      if (isa<ConstantInt>(usedValue)) continue;
      if (IVM.isLoopInvariant(usedValue)) continue;

      if (auto usedInst = dyn_cast<Instruction>(usedValue)) {
        if (!LS->isIncluded(usedInst)) continue;
        auto isIVUse = this->isIVInstruction(usedInst);
        auto isDerivedUse = checkIfDerived(usedInst);
        if (isIVUse || isDerivedUse) {
          usesAtLeastOneIVInstruction = true;
          continue;
        }
      }

      return false;
    }

    if (!usesAtLeastOneIVInstruction) return false;

    /*
     * Cache the result
     */
    derivedSCEVInstructions.insert(I);

    return true;
  };

  /*
   * Queue traversal through users of IV instructions to find all derived instructions
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
        if (visited.find(userInst) != visited.end()) continue;
        visited.insert(userInst);

        /*
         * If the user isn't derived, do not continue traversing users
         */
        if (!checkIfDerived(userInst)) continue;
        intermediates.push(userInst);
      }
    }
  }

  return;
}

void InductionVariable::collectValuesInternalAndExternalToLoopAndSCC (
  LoopStructure *LS,
  LoopEnvironment &loopEnvironment
) {

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
void InductionVariable::deriveStepValue (
  LoopStructure *LS,
  ScalarEvolution &SE,
  ScalarEvolutionReferentialExpander &referentialExpander
) {

  /*
   * Fetch the SCEV for the step value.
   */
  if (!this->stepSCEV) {
    auto loopEntrySCEV = SE.getSCEV(loopEntryPHI);
    assert(loopEntrySCEV->getSCEVType() == SCEVTypes::scAddRecExpr);
    this->stepSCEV = cast<SCEVAddRecExpr>(loopEntrySCEV)->getStepRecurrence(SE);
  }

  switch (stepSCEV->getSCEVType()) {
    case SCEVTypes::scConstant:
      deriveStepValueFromSCEVConstant(cast<SCEVConstant>(stepSCEV));
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
      if (!deriveStepValueFromCompositeSCEV(stepSCEV, referentialExpander, LS)) {
        this->stepSCEV = nullptr;
      }
      break;
    case SCEVTypes::scCouldNotCompute:
      break;
  }

}

void InductionVariable::deriveStepValueFromSCEVConstant (const SCEVConstant *scev) {
  this->singleStepValue = scev->getValue();
  this->isComputedStepValueLoopInvariant = true;
}

void InductionVariable::deriveStepValueFromSCEVUnknown (const SCEVUnknown *scev, LoopStructure *LS) {
  this->singleStepValue = scev->getValue();
  this->isComputedStepValueLoopInvariant = LS->isLoopInvariant(this->singleStepValue);
}

bool InductionVariable::deriveStepValueFromCompositeSCEV (
  const SCEV *scev,
  ScalarEvolutionReferentialExpander &referentialExpander,
  LoopStructure *LS
) {

  // auto M = headerPHI->getFunction()->getParent();
  // DataLayout DL(M);
  // const char name = 'a';
  // SCEVExpander *expander = new SCEVExpander(SE, DL, &name);

  // stepSCEV->print(errs() << "Referencing: "); errs() << "\n";
  auto stepSizeReferenceTree = referentialExpander.createReferenceTree(scev, valuesInScopeOfInductionVariable);
  if (!stepSizeReferenceTree) return false;

  // stepSizeReferenceTree->getSCEV()->print(errs() << "Expanding: "); errs() << "\n";
  auto tempBlock = BasicBlock::Create(loopEntryPHI->getContext());
  IRBuilder<> tempBuilder(tempBlock);
  auto finalValue = referentialExpander.expandUsingReferenceValues(
    stepSizeReferenceTree,
    valuesToReferenceInComputingStepValue,
    tempBuilder
  );
  if (!finalValue) return false;

  this->isComputedStepValueLoopInvariant = true;
  auto references = stepSizeReferenceTree->collectAllReferences();
  // TODO: Only check leaf reference values
  for (auto reference : references) {
    if (reference->getValue() && LS->isLoopInvariant(reference->getValue())) {
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
    singleStepValue = finalValue;
  }

  /*
    * Save expanded values that compute the step recurrence
    */
  for (auto &I : *tempBlock) {
    computationOfStepValue.push_back(&I);
  }

  return true;
}

InductionVariable::~InductionVariable () {
  BasicBlock *tempBlock = nullptr;
  if (tempBlock) {
    tempBlock->deleteValue();
  }
}

SCC *InductionVariable::getSCC (void) const {
  return &scc;
}

PHINode * InductionVariable::getLoopEntryPHI (void) const {
  return loopEntryPHI;
}

std::unordered_set<PHINode *> InductionVariable::getPHIs (void) const {
  return PHIs;
}

std::unordered_set<Instruction *> InductionVariable::getNonPHIIntermediateValues (void) const {
  return nonPHIIntermediateValues;
}

std::unordered_set<Instruction *> InductionVariable::getAllInstructions(void) const {
  return allInstructions;
}

std::unordered_set<Instruction *> InductionVariable::getDerivedSCEVInstructions(void) const {
  return derivedSCEVInstructions;
}

Value *InductionVariable::getStartValue (void) const {
  return startValue;
}

Value *InductionVariable::getSingleComputedStepValue (void) const {
  return singleStepValue;
}

const SCEV * InductionVariable::getStepSCEV (void) const {
  return stepSCEV;
}

std::vector<Instruction *> InductionVariable::getComputationOfStepValue(void) const {
  return computationOfStepValue;
}

bool InductionVariable::isStepValueLoopInvariant (void) const {
  return isComputedStepValueLoopInvariant;
}

bool InductionVariable::isIVInstruction (Instruction *I) const {
  return allInstructions.find(I) != allInstructions.end();
}

bool InductionVariable::isDerivedFromIVInstructions (Instruction *I) const {
  return derivedSCEVInstructions.find(I) != derivedSCEVInstructions.end();
}

bool InductionVariable::isStepValuePositive (void) const {

  /*
   * Fetch the step value.
   */
  assert(this->isComputedStepValueLoopInvariant);
  auto stepValue = this->getSingleComputedStepValue();

  /*
   * Check if the step value is positive
   */
  if (this->loopEntryPHIType->isIntegerTy()) {
    return cast<ConstantInt>(stepValue)->getValue().isStrictlyPositive();
  } else {
    assert(this->loopEntryPHIType->isFloatingPointTy());
    auto fpValue = cast<ConstantFP>(stepValue)->getValueAPF();
    return fpValue.isNonZero() && !fpValue.isNegative();
  }

}

Type * InductionVariable::getIVType (void) const {
  return loopEntryPHIType;
}

}
