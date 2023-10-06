/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni, Brian Homerding
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
#include "noelle/core/Variable.hpp"
#include "noelle/core/LoopCarriedDependencies.hpp"

namespace llvm::noelle {

LoopCarriedVariable::LoopCarriedVariable(const LoopStructure &loop,
                                         LoopTree *loopNode,
                                         PDG &loopDG,
                                         SCCDAG &sccdag,
                                         SCC &sccContainingVariable,
                                         PHINode *declarationPHI)
  : outermostLoopOfVariable{ loop },
    declarationValue{ declarationPHI },
    isValid{ false } {

  assert(sccContainingVariable.isInternal(declarationPHI)
         && "Declaration PHI node is not internal to the SCC provided!");

  /*
   * Ensure the loop is in a normalized form
   */
  auto preHeaderBlock = outermostLoopOfVariable.getPreHeader();
  if (!preHeaderBlock
      || declarationPHI->getBasicBlockIndex(preHeaderBlock) == -1)
    return;
  this->initialValue = declarationPHI->getIncomingValueForBlock(preHeaderBlock);

  /*
   * Collect loop carried values of this variable
   *  These are from loop carried dependencies consumed by the declaration PHI
   * Collect all other loop carried dependencies as well
   *  These will be ignored when constructing the variable's data/memory SCC
   */
  auto declarationNode = sccContainingVariable.fetchNode(declarationValue);
  auto loopCarriedDependencies =
      LoopCarriedDependencies::getLoopCarriedDependenciesForLoop(loop,
                                                                 loopNode,
                                                                 sccdag);

  std::unordered_set<DGEdge<Value, Value> *> edgesThatExist;
  std::unordered_set<DGEdge<Value, Value> *> edgesToRemove;

  std::unordered_set<Value *> loopCarriedValues{};
  std::unordered_set<DGEdge<Value, Value> *>
      loopCarriedDependenciesNotOfVariable{};

  for (auto dependency : loopCarriedDependencies) {
    auto consumer = dependency->getDst();
    if (consumer == declarationValue) {
      auto producer = dependency->getSrc();
      loopCarriedValues.insert(producer);
    } else {
      loopCarriedDependenciesNotOfVariable.insert(dependency);
    }
  }

  /*
   * We are interested in the SCC containing data/memory/control values
   * with loop carried dependencies only pertaining to the variable definition
   * thereby dropping any cycles within the SCC not pertaining to the variable
   *
   * This will identify control nodes that directly participate in the evolution
   * of the variable
   */
  std::vector<Value *> allPossibleInternalValues{};
  for (auto nodePair : sccContainingVariable.internalNodePairs()) {
    auto value = nodePair.first;
    allPossibleInternalValues.push_back(value);
  }
  auto dgOfVariable =
      loopDG.createSubgraphFromValues(allPossibleInternalValues,
                                      true,
                                      loopCarriedDependenciesNotOfVariable);
  auto sccdagOfVariable = new SCCDAG(dgOfVariable);
  this->sccOfVariableOnly = sccdagOfVariable->sccOfValue(declarationPHI);

  /*
   * We are interested in the SCC containing data/memory values
   * with loop carried dependencies only pertaining to the variable declaration
   * thereby dropping all control cycles along with any data/memory cycles not
   * pertaining to the variable
   *
   * This will identify updates to and only to the variable
   */
  auto dataMemoryDGOfVariable =
      produceDataAndMemoryOnlyDGFromVariableDG(*dgOfVariable);
  auto dataMemorySCCDAGOfVariable = new SCCDAG(dataMemoryDGOfVariable);
  assert(
      dataMemorySCCDAGOfVariable->sccOfValue(declarationPHI) != nullptr
      && "Declaration PHI was not persisted in derived data and memory only SCCDAG");
  this->sccOfDataAndMemoryVariableValuesOnly =
      dataMemorySCCDAGOfVariable->sccOfValue(declarationPHI);

  /*
   * Identify all control values, internal or external, to the variable SCC
   */
  for (auto node : sccOfVariableOnly->getNodes()) {
    auto value = node->getT();

    if (auto selectInst = dyn_cast<SelectInst>(value)) {

      /*
       * Select instructions contain a condition that controls the evolution of
       * the variable There is no need to check them for producing control
       * dependencies, so we continue
       */
      this->controlValuesGoverningEvolution.insert(selectInst->getCondition());
      continue;
    }

    for (auto edge : node->getOutgoingEdges()) {
      if (!edge->isControlDependence())
        continue;

      /*
       * This value produces a control dependency
       */
      this->controlValuesGoverningEvolution.insert(value);
      break;
    }
  }

  /*
   * Catalog all internal values in the variable's data/memory only SCC as
   * VariableUpdate
   */
  for (auto dataOrMemoryNodePair :
       sccOfDataAndMemoryVariableValuesOnly->internalNodePairs()) {
    auto value = dataOrMemoryNodePair.first;
    if (value == declarationValue)
      continue;

    assert(isa<Instruction>(value));

    /*
     * Ignore instructions that have no effect on the evolution of the variable
     */
    if (isa<LoadInst>(value))
      continue;
    if (auto cast = dyn_cast<CastInst>(value)) {
      castsInternalToVariableComputation.insert(cast);
      continue;
    }

    auto variableUpdate =
        new EvolutionUpdate(cast<Instruction>(value),
                            sccOfDataAndMemoryVariableValuesOnly);
    variableUpdates.insert(variableUpdate);

    if (loopCarriedValues.find(value) == loopCarriedValues.end())
      continue;
    loopCarriedVariableUpdates.insert(variableUpdate);
  }

  this->isValid = true;
}

LoopCarriedVariable::~LoopCarriedVariable() {
  // if (dataDGOfVariable) delete dataDGOfVariable;
  // if (sccdagOfVariable) delete sccdagOfVariable;
  // if (dataSCCDAGOfVariable) delete dataSCCDAGOfVariable;

  for (auto variableUpdate : variableUpdates) {
    delete variableUpdate;
  }
}

bool LoopCarriedVariable::isEvolutionReducibleAcrossLoopIterations(void) const {
  if (!this->isValid) {
    return false;
  }

  /*
   * No control values internal to the variable can be involved in the evolution
   * This would prevent partial computation and then reduction as the condition
   * results might change
   */
  for (auto controlValue : this->controlValuesGoverningEvolution) {
    if (sccOfVariableOnly->isInternal(controlValue)) {
      return false;
    }
  }

  /*
   * Collect updates that do not just propagate other updates
   */
  std::unordered_set<EvolutionUpdate *> arithmeticUpdates;
  for (auto update : this->variableUpdates) {

    /*
     * Overriding updates break reducibility
     */
    if (update->mayUpdateBeOverride()) {
      return false;
    }

    auto updateInstruction = update->getUpdateInstruction();
    if (isa<PHINode>(updateInstruction) || isa<SelectInst>(updateInstruction))
      continue;
    arithmeticUpdates.insert(update);
  }

  /*
   * Do not allow any casts to cause rounding error if the variable is reduced
   */
  if (hasRoundingError(arithmeticUpdates)) {
    return false;
  }

  /*
   * Ignore a value that does not evolve and is just propagated; its last
   * execution is its current value
   */
  if (arithmeticUpdates.size() == 0) {
    return false;
  }

  /*
   * All arithmetic updates must be mutually commutative and associative
   * TODO: does transitivity of those two properties allow only checking one
   * update against all others?
   */
  for (auto update : arithmeticUpdates) {
    for (auto otherUpdate : arithmeticUpdates) {
      if (!update->isTransformablyCommutativeWith(*otherUpdate))
        return false;
      if (!update->isAssociativeWith(*otherUpdate))
        return false;
    }
  }

  /*
   * All consumers must be live out intermediate values of the variable
   * that do not perform further computation within the loop, as that consumer
   * would prevent reducing the variable and collecting it outside the loop
   */
  auto consumers = getConsumersOfVariable();
  if (!areValuesPropagatingVariableIntermediatesOutsideLoop(consumers))
    return false;

  return true;
}

PDG *LoopCarriedVariable::produceDataAndMemoryOnlyDGFromVariableDG(
    PDG &variableDG) const {

  /*
   * Collect values that do NOT produce a control dependency
   */
  std::vector<Value *> dataAndMemoryValues{};
  for (auto nodePair : variableDG.internalNodePairs()) {

    auto node = nodePair.second;
    auto producesControlDependency = false;
    for (auto edge : node->getOutgoingEdges()) {
      if (edge->isControlDependence()) {
        producesControlDependency = true;
        break;
      }
    }

    /*
     * Ignore control nodes altogether
     */
    if (producesControlDependency)
      continue;

    /*
     * If no data or memory dependency is produced, keep the node just in case
     * the variable happens to be a trivial, unevolving one
     *
     * While non-controlling terminator instructions will still pass along,
     * they can be ignored, as it was a node merged into the SCC and will not
     * remain in the SCC containing the variable when the new SCCDAG is computed
     *
     */
    auto value = node->getT();
    dataAndMemoryValues.push_back(value);
  }

  return variableDG.createSubgraphFromValues(dataAndMemoryValues, true);
}

std::unordered_set<Value *> LoopCarriedVariable::getConsumersOfVariable(
    void) const {
  std::unordered_set<Value *> consumers;

  for (auto externalNodePair : sccOfVariableOnly->externalNodePairs()) {
    auto value = externalNodePair.first;
    if (!isa<Instruction>(value))
      continue;

    /*
     * Ignore loop externals outside the loop, such as live outs
     */
    auto consumer = cast<Instruction>(value);
    if (!outermostLoopOfVariable.isIncluded(consumer))
      continue;

    auto node = externalNodePair.second;
    for (auto edge : node->getIncomingEdges()) {
      auto producer = edge->getSrc();
      if (sccOfVariableOnly->isExternal(producer))
        continue;

      /*
       * This is a loop internal consumer of the variable
       */
      consumers.insert(consumer);
    }
  }

  return consumers;
}

bool LoopCarriedVariable::areValuesPropagatingVariableIntermediatesOutsideLoop(
    std::unordered_set<Value *> values) const {

  auto loopHeader = outermostLoopOfVariable.getHeader();
  auto loopPreheader = outermostLoopOfVariable.getPreHeader();
  std::queue<Value *> valuesToCheck;
  std::unordered_set<Value *> valuesChecked;
  for (auto value : values) {
    valuesToCheck.push(value);
    valuesChecked.insert(value);
  }

  while (!valuesToCheck.empty()) {
    auto value = valuesToCheck.front();
    valuesToCheck.pop();

    // value->print(errs() << "Checking value: "); errs() << "\n";

    /*
     * Ensure the value propagates an intermediate value of the variable or is
     * contained within
     */
    if (auto cast = dyn_cast<CastInst>(value)) {
      auto valueToCast = cast->getOperand(0);
      if (sccOfDataAndMemoryVariableValuesOnly->isExternal(valueToCast))
        return false;

    } else if (auto phi = dyn_cast<PHINode>(value)) {

      /*
       * The PHI cannot encapsulate any control. Therefore, the PHI must be in
       * the header and all incoming values must be the same (except for the pre
       * header incoming value)
       */
      if (loopHeader != phi->getParent())
        return false;

      Value *singleIncomingValue = nullptr;
      for (int32_t i = 0; i < phi->getNumIncomingValues(); ++i) {
        auto incomingBlock = phi->getIncomingBlock(i);
        auto incomingValue = phi->getIncomingValue(i);
        // incomingValue->print(errs() << "Checking incoming value: "); errs()
        // << "\n";

        if (incomingBlock == loopPreheader)
          continue;
        if (singleIncomingValue == nullptr
            || incomingValue == singleIncomingValue) {
          singleIncomingValue = incomingValue;
          continue;
        }

        singleIncomingValue = nullptr;
        break;
      }

      if (!singleIncomingValue)
        return false;
      if (sccOfDataAndMemoryVariableValuesOnly->isExternal(singleIncomingValue))
        return false;

    } else
      return false;

    /*
     * Ensure the value isn't used inside the loop for further computation
     */
    for (auto user : value->users()) {
      if (!isa<Instruction>(user))
        continue;
      auto userI = cast<Instruction>(user);
      auto userBlock = userI->getParent();

      if (!outermostLoopOfVariable.isIncluded(userBlock))
        continue;
      if (valuesChecked.find(userI) != valuesChecked.end())
        continue;
      valuesToCheck.push(userI);
      valuesChecked.insert(userI);
    }
  }

  return true;
}

PHINode *LoopCarriedVariable::getLoopEntryPHIForValueOfVariable(
    Value *value) const {

  if (!sccOfVariableOnly->isInGraph(value))
    return nullptr;

  /*
   * If the value is external, fetch the loop entry PHI of the variable
   * that is consumed by this value
   */
  if (sccOfVariableOnly->isExternal(value)) {
    if (auto phi = dyn_cast<PHINode>(value)) {
      return phi;
    }

    // TODO: Implement graph traversal from value up to producing PHI
    return nullptr;
  }

  assert(isa<PHINode>(declarationValue));
  return cast<PHINode>(declarationValue);
}

bool LoopCarriedVariable::hasRoundingError(
    std::unordered_set<EvolutionUpdate *> &arithmeticUpdates) const {

  /*
   * Casting might change the rounding error.
   * Hence, if casts to and from different precision types are present, further
   * analysis is needed
   */
  auto isIntegerTypedCast = false;
  auto isFloatingPointTypedCast = false;
  for (auto cast : this->castsInternalToVariableComputation) {
    auto castTy = cast->getType();
    isIntegerTypedCast |= castTy->isIntegerTy();
    isFloatingPointTypedCast |= castTy->isFloatingPointTy();

    /*
     * Check if we are converting between floating point values.
     * If we do, then we need an extra analysis to try to exclude the
     * possibility of having rounding errors. We currently don't have such
     * analysis, and therefore we need to be conservative and assume rounding
     * errors will happen.
     */
    auto srcType = cast->getSrcTy();
    if (true && (castTy->isFloatingPointTy())
        && (srcType->isFloatingPointTy())) {
      return true;
    }
  }
  if (!isIntegerTypedCast || !isFloatingPointTypedCast) {
    return false;
  }

  /*
   * Handle the simple case where the accumulation is additive and cast up from
   * integer to floating point Rounding from floating point to integer in this
   * case is only impacting the value added each iteration and is not propagated
   * iteration to iteration
   */
  auto accumulationType = declarationValue->getType();
  auto onlyAddition = true;
  for (auto update : arithmeticUpdates) {
    onlyAddition &= update->isAdd() || update->isSubTransformableToAdd();
  }
  if (accumulationType->isIntegerTy() && onlyAddition) {
    return false;
  }

  /*
   * TODO: have a better understanding of what constitutes rounding error
   */
  return true;
}

Value *LoopCarriedVariable::getInitialValue(void) const {
  return this->initialValue;
}

} // namespace llvm::noelle
