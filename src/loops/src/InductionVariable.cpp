/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "InductionVariables.hpp"
#include "LoopGoverningIVAttribution.hpp"

using namespace llvm;

InductionVariable::InductionVariable  (
  LoopSummary *LS,
  ScalarEvolution &SE,
  PHINode *loopEntryPHI,
  SCC &scc,
  LoopEnvironment &loopEnv,
  ScalarEvolutionReferentialExpander &referentialExpander
) : scc{scc}, loopEntryPHI{loopEntryPHI}, startValue{nullptr},
    stepSCEV{nullptr}, computationOfStepValue{}, isComputedStepValueLoopInvariant{false} {

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
     */
    if (!isa<Instruction>(value)) continue;
    auto instruction = cast<Instruction>(value);
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
      if (!edge->isDataDependence()) continue;
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
      if (!isa<CastInst>(user)) continue;
      castsToAdd.insert(cast<CastInst>(user));
    }
  }
  this->allInstructions.insert(castsToAdd.begin(), castsToAdd.end());

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

  collectValuesInternalAndExternalToLoopAndSCC(LS, loopEnv);
  deriveStepValue(LS, SE, referentialExpander, loopEnv);
}

void InductionVariable::collectValuesInternalAndExternalToLoopAndSCC (
  LoopSummary *LS,
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
  LoopSummary *LS,
  ScalarEvolution &SE,
  ScalarEvolutionReferentialExpander &referentialExpander,
  LoopEnvironment &loopEnv
) {

  auto loopEntrySCEV = SE.getSCEV(loopEntryPHI);
  assert(loopEntrySCEV->getSCEVType() == SCEVTypes::scAddRecExpr);
  this->stepSCEV = cast<SCEVAddRecExpr>(loopEntrySCEV)->getStepRecurrence(SE);

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

void InductionVariable::deriveStepValueFromSCEVUnknown (const SCEVUnknown *scev, LoopSummary *LS) {
  this->singleStepValue = scev->getValue();
  this->isComputedStepValueLoopInvariant = LS->isLoopInvariant(this->singleStepValue);
}

bool InductionVariable::deriveStepValueFromCompositeSCEV (
  const SCEV *scev,
  ScalarEvolutionReferentialExpander &referentialExpander,
  LoopSummary *LS
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

  finalValue->print(errs() << "Expanded final value: "); errs() << "\n";

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

std::set<PHINode *> InductionVariable::getPHIs (void) const {
  return PHIs;
}

std::set<Instruction *> InductionVariable::getNonPHIIntermediateValues (void) const {
  return nonPHIIntermediateValues;
}

std::set<Instruction *> InductionVariable::getAllInstructions(void) const {
  return allInstructions;
}

Value *InductionVariable::getStartValue (void) const {
  return startValue;
}

Value *InductionVariable::getSingleComputedStepValue (void) const {
  return singleStepValue;
}

const SCEV *InductionVariable::getStepSCEV (void) const {
  return stepSCEV;
}

std::vector<Instruction *> InductionVariable::getComputationOfStepValue(void) const {
  return computationOfStepValue;
}

bool InductionVariable::isStepValueLoopInvariant (void) const {
  return isComputedStepValueLoopInvariant;
}
