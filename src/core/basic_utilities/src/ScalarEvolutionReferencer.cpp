/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
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
#include "noelle/core/ScalarEvolutionReferencer.hpp"

using namespace llvm;

/*******************************
 * ScalarEvolutionReferentialExpander
 */

ScalarEvolutionReferentialExpander::ScalarEvolutionReferentialExpander(
    ScalarEvolution &SE,
    Function &F) {
  scevValueMapper = new SCEVValueMapper(SE, F);
}

ScalarEvolutionReferentialExpander::~ScalarEvolutionReferentialExpander() {
  delete scevValueMapper;
}

SCEVReference *ScalarEvolutionReferentialExpander::createReferenceTree(
    const SCEV *scev,
    std::set<Value *> valuesInScope) {
  ReferenceTreeBuilder builder(scev, *scevValueMapper, valuesInScope);
  return builder.getTree();
}

Value *ScalarEvolutionReferentialExpander::expandUsingReferenceValues(
    SCEVReference *tree,
    std::set<Value *> valuesToReferenceAndNotExpand,
    IRBuilder<> &expansionBuilder) {
  ReferenceTreeExpander expander(tree,
                                 valuesToReferenceAndNotExpand,
                                 expansionBuilder);
  return expander.getRootOfTree();
}

/*******************************
 * ReferenceTreeBuilder
 */

ReferenceTreeBuilder::ReferenceTreeBuilder(const SCEV *scev,
                                           SCEVValueMapper &scevValueMapper,
                                           std::set<Value *> &valuesInScope)
  : valuesInScope{ valuesInScope },
    scevValueMapper{ scevValueMapper } {

  tree = visit(scev);
}

SCEVReference *ReferenceTreeBuilder::getTree() {
  return tree;
}

Value *ReferenceTreeBuilder::mapToSingleInScopeValue(const SCEV *S) {
  auto values = scevValueMapper.getValuesOf(S);
  Value *singleV = nullptr;
  for (auto V : values) {
    if (valuesInScope.find(V) == valuesInScope.end())
      continue;
    if (singleV)
      return nullptr;

    singleV = V;
  }

  return singleV;
}

SCEVReference *ReferenceTreeBuilder::createReferenceOfSingleInScopeValue(
    const SCEV *S) {
  auto singleValue = mapToSingleInScopeValue(S);
  return singleValue ? new SCEVReference(singleValue, S) : nullptr;
}

SCEVReference *ReferenceTreeBuilder::createReferenceOfNArySCEV(
    const SCEVNAryExpr *S) {
  auto compositeReference = new SCEVReference(mapToSingleInScopeValue(S), S);

  for (auto opS : S->operands()) {
    auto operandReference = visit(opS);
    if (!operandReference) {
      break;
    }

    compositeReference->addChildReference(operandReference);
  }

  /*
   * If references of composed SCEVs could not be found, only return this
   * reference if its value is within scope
   */
  if (compositeReference->getNumChildReferences() != S->getNumOperands()) {
    if (!compositeReference->getValue()) {
      delete compositeReference;
      return nullptr;
    }
  }

  return compositeReference;
}

SCEVReference *ReferenceTreeBuilder::visitConstant(const SCEVConstant *S) {
  return new SCEVReference(S->getValue(), S);
}

SCEVReference *ReferenceTreeBuilder::visitUnknown(const SCEVUnknown *S) {
  auto value = S->getValue();
  return valuesInScope.find(value) == valuesInScope.end()
             ? nullptr
             : new SCEVReference(value, S);
}

SCEVReference *ReferenceTreeBuilder::visitTruncateExpr(
    const SCEVTruncateExpr *S) {
  return createReferenceOfSingleInScopeValue(S);
}

SCEVReference *ReferenceTreeBuilder::visitZeroExtendExpr(
    const SCEVZeroExtendExpr *S) {
  return createReferenceOfSingleInScopeValue(S);
}

SCEVReference *ReferenceTreeBuilder::visitSignExtendExpr(
    const SCEVSignExtendExpr *S) {
  return createReferenceOfSingleInScopeValue(S);
}

SCEVReference *ReferenceTreeBuilder::visitAddExpr(const SCEVAddExpr *S) {
  return createReferenceOfNArySCEV(S);
}

SCEVReference *ReferenceTreeBuilder::visitMulExpr(const SCEVMulExpr *S) {
  return createReferenceOfNArySCEV(S);
}

SCEVReference *ReferenceTreeBuilder::visitUDivExpr(const SCEVUDivExpr *S) {
  auto LHS = visit(S->getLHS());
  auto RHS = visit(S->getRHS());
  auto selfValue = mapToSingleInScopeValue(S);
  auto uDivReference = new SCEVReference(selfValue, S);

  if (LHS && RHS) {
    uDivReference->addChildReference(LHS);
    uDivReference->addChildReference(RHS);
    return uDivReference;
  }

  /*
   * References of composed SCEVs could not be found
   * Only return this reference if its value is within scope
   */
  if (LHS)
    delete LHS;
  if (RHS)
    delete RHS;
  if (!selfValue) {
    delete uDivReference;
    return nullptr;
  }
  return uDivReference;
}

SCEVReference *ReferenceTreeBuilder::visitAddRecExpr(const SCEVAddRecExpr *S) {
  return createReferenceOfNArySCEV(S);
}

SCEVReference *ReferenceTreeBuilder::visitSMaxExpr(const SCEVSMaxExpr *S) {
  return createReferenceOfNArySCEV(S);
}

SCEVReference *ReferenceTreeBuilder::visitUMaxExpr(const SCEVUMaxExpr *S) {
  return createReferenceOfNArySCEV(S);
}

SCEVReference *ReferenceTreeBuilder::visitSMinExpr(const SCEVSMinExpr *S) {
  return createReferenceOfNArySCEV(S);
}

SCEVReference *ReferenceTreeBuilder::visitUMinExpr(const SCEVUMinExpr *S) {
  return createReferenceOfNArySCEV(S);
}

SCEVReference *visitCouldNotCompute(const SCEVCouldNotCompute *S) {
  return nullptr;
}

/*******************************
 * SCEVValueMapper
 */

SCEVValueMapper::SCEVValueMapper(ScalarEvolution &SE, Function &F) {
  for (auto &A : F.args()) {
    if (!SE.isSCEVable(A.getType()))
      continue;
    auto scev = SE.getSCEV(&A);
    scevToValues[scev].insert(&A);
    valueToSCEV[&A] = scev;
  }

  for (auto &B : F) {
    for (auto &I : B) {
      if (!SE.isSCEVable(I.getType()))
        continue;
      auto scev = SE.getSCEV(&I);
      scevToValues[scev].insert(&I);
      valueToSCEV[&I] = scev;
    }
  }
}

Value *SCEVValueMapper::getSingleValueOf(const SCEV *scev) const {
  auto values = getValuesOf(scev);
  return (values.size() != 1) ? nullptr : *values.begin();
}

const std::set<Value *> SCEVValueMapper::getValuesOf(const SCEV *scev) const {
  return scevToValues.find(scev) != scevToValues.end() ? scevToValues.at(scev)
                                                       : std::set<Value *>{};
}

const SCEV *SCEVValueMapper::getSCEVOf(Value *value) const {
  return valueToSCEV.find(value) != valueToSCEV.end() ? valueToSCEV.at(value)
                                                      : nullptr;
}

/*******************************
 * SCEVReference
 */

SCEVReference::SCEVReference(Value *v, const SCEV *scev)
  : value{ v },
    scev{ scev },
    childReferences{} {}

SCEVReference::~SCEVReference() {
  for (auto child : childReferences)
    delete child;
}

Value *SCEVReference::getValue() const {
  return value;
}

const SCEV *SCEVReference::getSCEV() const {
  return scev;
}

iterator_range<std::vector<SCEVReference *>::iterator> SCEVReference::
    getChildReferences() {
  return make_range(childReferences.begin(), childReferences.end());
}

SCEVReference *SCEVReference::getChildReference(int32_t idx) {
  return childReferences.at(idx);
}

int32_t SCEVReference::getNumChildReferences() {
  return childReferences.size();
}

void SCEVReference::addChildReference(SCEVReference *scevReference) {
  childReferences.push_back(scevReference);
}

std::set<SCEVReference *> SCEVReference::collectAllReferences() {
  std::set<SCEVReference *> references;
  references.insert(this);
  for (auto child : childReferences) {
    auto childReferences = child->collectAllReferences();
    references.insert(childReferences.begin(), childReferences.end());
  }

  return references;
}
