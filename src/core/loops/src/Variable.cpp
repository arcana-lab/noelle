/*
 * Copyright 2016 - 2021  Angelo Matni, Simone Campanoni, Brian Homerding
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/Variable.hpp"
#include "LoopCarriedDependencies.hpp"

namespace llvm::noelle {

bool LoopCarriedCycle::isEvolutionReducibleAcrossLoopIterations (void) const {
  return false;
}

LoopCarriedVariable::LoopCarriedVariable (
  const LoopStructure &loop,
  StayConnectedNestedLoopForestNode *loopNode,
  PDG &loopDG,
  SCCDAG &sccdag,
  SCC &sccContainingVariable,
  PHINode *declarationPHI
) : outermostLoopOfVariable{loop}, declarationValue{declarationPHI}, isValid{false} {

  assert(sccContainingVariable.isInternal(declarationPHI)
    && "Declaration PHI node is not internal to the SCC provided!");

  /*
   * Ensure the loop is in a normalized form
   */
  auto preHeaderBlock = outermostLoopOfVariable.getPreHeader();
  if (!preHeaderBlock
    || declarationPHI->getBasicBlockIndex(preHeaderBlock) == -1) return ;
  this->initialValue = declarationPHI->getIncomingValueForBlock(preHeaderBlock);

  /*
   * Collect loop carried values of this variable
   *  These are from loop carried dependencies consumed by the declaration PHI
   * Collect all other loop carried dependencies as well
   *  These will be ignored when constructing the variable's data/memory SCC
   */
  auto declarationNode = sccContainingVariable.fetchNode(declarationValue);
  auto loopCarriedDependencies = LoopCarriedDependencies::getLoopCarriedDependenciesForLoop(loop, loopNode,  sccdag);

  std::unordered_set<DGEdge<Value> *> edgesThatExist;
  std::unordered_set<DGEdge<Value> *> edgesToRemove;

  std::unordered_set<Value *> loopCarriedValues{};
  std::unordered_set<DGEdge<Value> *> loopCarriedDependenciesNotOfVariable{};

  for (auto dependency : loopCarriedDependencies) {
    auto consumer = dependency->getIncomingT();
    if (consumer == declarationValue) {
      auto producer = dependency->getOutgoingT();
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
   * This will identify control nodes that directly participate in the evolution of the variable
   */
  std::vector<Value *> allPossibleInternalValues{};
  for (auto nodePair : sccContainingVariable.internalNodePairs()) {
    auto value = nodePair.first;
    allPossibleInternalValues.push_back(value);
  }
  auto dgOfVariable = loopDG.createSubgraphFromValues(allPossibleInternalValues, true, loopCarriedDependenciesNotOfVariable);
  auto sccdagOfVariable = new SCCDAG(dgOfVariable);
  this->sccOfVariableOnly = sccdagOfVariable->sccOfValue(declarationPHI);

  /*
   * We are interested in the SCC containing data/memory values
   * with loop carried dependencies only pertaining to the variable declaration
   * thereby dropping all control cycles along with any data/memory cycles not pertaining to the variable
   * 
   * This will identify updates to and only to the variable
   */
  auto dataMemoryDGOfVariable = produceDataAndMemoryOnlyDGFromVariableDG(*dgOfVariable);
  auto dataMemorySCCDAGOfVariable = new SCCDAG(dataMemoryDGOfVariable);
  assert(dataMemorySCCDAGOfVariable->sccOfValue(declarationPHI) != nullptr
    && "Declaration PHI was not persisted in derived data and memory only SCCDAG");
  this->sccOfDataAndMemoryVariableValuesOnly = dataMemorySCCDAGOfVariable->sccOfValue(declarationPHI);

  /*
   * Identify all control values, internal or external, to the variable SCC
   */
  for (auto node : sccOfVariableOnly->getNodes()) {
    auto value = node->getT();

    if (auto selectInst = dyn_cast<SelectInst>(value)) {

      /*
       * Select instructions contain a condition that controls the evolution of the variable 
       * There is no need to check them for producing control dependencies, so we continue
       */
      this->controlValuesGoverningEvolution.insert(selectInst->getCondition());
      continue;
    }

    for (auto edge : node->getOutgoingEdges()) {
      if (!edge->isControlDependence()) continue;

      /*
       * This value produces a control dependency
       */
      this->controlValuesGoverningEvolution.insert(value);
      break;
    }
  }

  /*
   * Catalog all internal values in the variable's data/memory only SCC as VariableUpdate
   */
  for (auto dataOrMemoryNodePair : sccOfDataAndMemoryVariableValuesOnly->internalNodePairs()) {
    auto value = dataOrMemoryNodePair.first;
    if (value == declarationValue) continue;

    assert(isa<Instruction>(value));

    /*
     * Ignore instructions that have no effect on the evolution of the variable
     */
    if (isa<LoadInst>(value)) continue;
    if (auto cast = dyn_cast<CastInst>(value)) {
      castsInternalToVariableComputation.insert(cast);
      continue;
    }

    auto variableUpdate = new EvolutionUpdate(cast<Instruction>(value), sccOfDataAndMemoryVariableValuesOnly);
    variableUpdates.insert(variableUpdate);

    if (loopCarriedValues.find(value) == loopCarriedValues.end()) continue;
    loopCarriedVariableUpdates.insert(variableUpdate);
  }

  this->isValid = true;
}

LoopCarriedVariable::~LoopCarriedVariable () {
  // if (dataDGOfVariable) delete dataDGOfVariable;
	// if (sccdagOfVariable) delete sccdagOfVariable;
	// if (dataSCCDAGOfVariable) delete dataSCCDAGOfVariable;

	for (auto variableUpdate : variableUpdates) {
	  delete variableUpdate;
	}
}

bool LoopCarriedVariable::isEvolutionReducibleAcrossLoopIterations (void) const {
  if (!this->isValid) {
    return false;
  }

  /*
   * No control values internal to the variable can be involved in the evolution
   * This would prevent partial computation and then reduction as the condition results might change
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
    if (isa<PHINode>(updateInstruction) || isa<SelectInst>(updateInstruction)) continue;
    arithmeticUpdates.insert(update);
  }

  /*
   * Do not allow any casts to cause rounding error if the variable is reduced
   */
  if (hasRoundingError(arithmeticUpdates)) {
    return false;
  }

  /*
   * Ignore a value that does not evolve and is just propagated; its last execution is its current value
   */
  if (arithmeticUpdates.size() == 0) {
    return false;
  }

  /*
   * All arithmetic updates must be mutually commutative and associative
   * TODO: does transitivity of those two properties allow only checking one update against all others?
   */
  for (auto update : arithmeticUpdates) {
    for (auto otherUpdate : arithmeticUpdates) {
      if (!update->isTransformablyCommutativeWith(*otherUpdate)) return false;
      if (!update->isAssociativeWith(*otherUpdate)) return false;
    }
  }

  /*
   * All consumers must be live out intermediate values of the variable
   * that do not perform further computation within the loop, as that consumer would
   * prevent reducing the variable and collecting it outside the loop
   */
  auto consumers = getConsumersOfVariable();
  if (!areValuesPropagatingVariableIntermediatesOutsideLoop(consumers)) return false;

  return true;
}

PDG *LoopCarriedVariable::produceDataAndMemoryOnlyDGFromVariableDG(PDG &variableDG) const {

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
    if (producesControlDependency) continue;

    /*
     * If no data or memory dependency is produced, keep the node just in case
     * the variable happens to be a trivial, unevolving one
     * 
     * While non-controlling terminator instructions will still pass along,
     * they can be ignored, as it was a node merged into the SCC and will not remain
     * in the SCC containing the variable when the new SCCDAG is computed
     * 
     */
    auto value = node->getT();
    dataAndMemoryValues.push_back(value);
  }

  return variableDG.createSubgraphFromValues(dataAndMemoryValues, true);
}

std::unordered_set<Value *> LoopCarriedVariable::getConsumersOfVariable (void) const {
  std::unordered_set<Value *> consumers;

  for (auto externalNodePair : sccOfVariableOnly->externalNodePairs()) {
    auto value = externalNodePair.first;
    if (!isa<Instruction>(value)) continue;

    /*
     * Ignore loop externals outside the loop, such as live outs
     */
    auto consumer = cast<Instruction>(value);
    if (!outermostLoopOfVariable.isIncluded(consumer)) continue;

    auto node = externalNodePair.second;
    for (auto edge : node->getIncomingEdges()) {
      auto producer = edge->getOutgoingT();
      if (sccOfVariableOnly->isExternal(producer)) continue;

      /*
       * This is a loop internal consumer of the variable
       */
      consumers.insert(consumer);
    }
  }

  return consumers;
}

bool LoopCarriedVariable::areValuesPropagatingVariableIntermediatesOutsideLoop (std::unordered_set<Value *> values) const {

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
     * Ensure the value propagates an intermediate value of the variable or is contained within 
     */
    if (auto cast = dyn_cast<CastInst>(value)) {
      auto valueToCast = cast->getOperand(0);
      if (sccOfDataAndMemoryVariableValuesOnly->isExternal(valueToCast)) return false;

    } else if (auto phi = dyn_cast<PHINode>(value)) {

      /*
       * The PHI cannot encapsulate any control. Therefore, the PHI must be in the header
       * and all incoming values must be the same (except for the pre header incoming value)
       */
      if (loopHeader != phi->getParent()) return false;

      Value *singleIncomingValue = nullptr;
      for (int32_t i = 0; i < phi->getNumIncomingValues(); ++i) {
        auto incomingBlock = phi->getIncomingBlock(i);
        auto incomingValue = phi->getIncomingValue(i);
        // incomingValue->print(errs() << "Checking incoming value: "); errs() << "\n";

        if (incomingBlock == loopPreheader) continue;
        if (singleIncomingValue == nullptr || incomingValue == singleIncomingValue) {
          singleIncomingValue = incomingValue;
          continue;
        }

        singleIncomingValue = nullptr;
        break;
      }

      if (!singleIncomingValue) return false;
      if (sccOfDataAndMemoryVariableValuesOnly->isExternal(singleIncomingValue)) return false;

    } else return false;

    /*
     * Ensure the value isn't used inside the loop for further computation
     */
    for (auto user : value->users()) {
      if (!isa<Instruction>(user)) continue;
      auto userI = cast<Instruction>(user);
      auto userBlock = userI->getParent();

      if (!outermostLoopOfVariable.isIncluded(userBlock)) continue;
      if (valuesChecked.find(userI) != valuesChecked.end()) continue;
      valuesToCheck.push(userI);
      valuesChecked.insert(userI);
    }

  }

  return true;
}

PHINode *LoopCarriedVariable::getLoopEntryPHIForValueOfVariable (Value *value) const {

  if (!sccOfVariableOnly->isInGraph(value)) return nullptr;

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

bool LoopCarriedVariable::hasRoundingError (std::unordered_set<EvolutionUpdate *> &arithmeticUpdates) const {

  /*
   * Casting might change the rounding error.
   * Hence, if casts to and from different precision types are present, further analysis is needed
   */
  auto isIntegerTypedCast = false;
  auto isFloatingPointTypedCast = false;
  for (auto cast : this->castsInternalToVariableComputation) {
    auto castTy = cast->getType();
    isIntegerTypedCast |= castTy->isIntegerTy();
    isFloatingPointTypedCast |= castTy->isFloatingPointTy();

    /*
     * Check if we are converting between floating point values.
     * If we do, then we need an extra analysis to try to exclude the possibility of having rounding errors.
     * We currently don't have such analysis, and therefore we need to be conservative and assume rounding errors will happen.
     */
    auto srcType = cast->getSrcTy();
    if (  true
          && (castTy->isFloatingPointTy())
          && (srcType->isFloatingPointTy())
       ){
      return true;
    }
  }
  if (!isIntegerTypedCast || !isFloatingPointTypedCast) {
    return false;
  }

  /*
   * Handle the simple case where the accumulation is additive and cast up from integer to floating point
   * Rounding from floating point to integer in this case is only impacting the value added
   * each iteration and is not propagated iteration to iteration
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

/************************************************************************************
 * LoopCarriedMemoryLocation implementation
 */

/*
 * TODO: Implement
 */
LoopCarriedMemoryLocation::LoopCarriedMemoryLocation (
  const LoopStructure &loop,
  PDG &loopDG,
  SCC &memoryLocationSCC,
  Value *memoryLocation
) {}

bool LoopCarriedMemoryLocation::isEvolutionReducibleAcrossLoopIterations (void) const {
  return false;
}

/************************************************************************************
 * VariableUpdate implementation
 */

EvolutionUpdate::EvolutionUpdate (Instruction *updateInstruction, SCC *dataMemoryVariableSCC)
  : updateInstruction{updateInstruction} {

  if (auto storeUpdate = dyn_cast<StoreInst>(updateInstruction)) {

    /*
     * No understanding from the StoreInst of the values used to derive this stored value
     * needs to be found. It will be found in the VariableUpdate producing the stored value.
     */
    this->newValue = storeUpdate->getValueOperand();
    return;
  }
  this->newValue = updateInstruction;

  for (auto &use : updateInstruction->operands()) {
    auto usedValue = use.get();

    if (dataMemoryVariableSCC->isInternal(usedValue)) {
      internalValuesUsed.insert(&use);
    } else {
      externalValuesUsed.insert(&use);
    }
  }
}

bool EvolutionUpdate::mayUpdateBeOverride (void) const {
  if (isa<SelectInst>(updateInstruction) || isa<PHINode>(updateInstruction)) {

    /*
     * If any operand in the select or phi instruction is external,
     * then the instruction can possibly override the variable
     */
    return externalValuesUsed.size() > 0;
  }

  /*
   * Without further analysis of the called function,
   * assume the caller can override the value
   */
  if (isa<CallInst>(updateInstruction)) return true;

  /*
   * If the instruction is a binary operator and at least
   * one operand is derived from a previous value of the variable,
   * this update isn't overriding
   */
  if (updateInstruction->isBinaryOp()) {
    return internalValuesUsed.size() == 0;
  }

  /*
   * Comparisons are not considered overriding as long as they
   * are immediately used by select instructions only
   * 
   * This defers the decision of overriding to the select instruction's VariableUpdate
   */
  if (isa<CmpInst>(updateInstruction)) {
    for (auto user : updateInstruction->users()) {
      if (isa<SelectInst>(user)) continue;
      return true;
    }
    return false;
  }

  /*
   * Conservatively report that the update may override
   * TODO: StoreInst are not handled yet: we only handle data variables so far
   */
  return true;
}

bool EvolutionUpdate::isCommutativeWithSelf (void) const {
  if (mayUpdateBeOverride()) return false;
  return updateInstruction->isCommutative();
}

bool EvolutionUpdate::isAdd (void) const {
  auto op = updateInstruction->getOpcode();
  return Instruction::Add == op
    || Instruction::FAdd == op;
}

bool EvolutionUpdate::isMul (void) const {
  auto op = updateInstruction->getOpcode();
  return Instruction::Mul == op
    || Instruction::FMul == op;
}

bool EvolutionUpdate::isSub (void) const {
  auto op = updateInstruction->getOpcode();
  return Instruction::Sub == op
    || Instruction::FSub == op;
}

/*
 * LLVM cannot analyze the instruction to a point where it
 * considers subtraction by an external value equivalent to
 * addition of the negative of that external value
 */
bool EvolutionUpdate::isSubTransformableToAdd () const {
  if (!isSub()) return false;
  auto &useOfValueBeingSubtracted = updateInstruction->getOperandUse(1);
  return externalValuesUsed.find(&useOfValueBeingSubtracted) != externalValuesUsed.end();
}

bool EvolutionUpdate::isTransformablyCommutativeWithSelf (void) const {
  if (mayUpdateBeOverride()) return false;
  if (updateInstruction->isCommutative()) return true;

  return isSubTransformableToAdd();
}

bool EvolutionUpdate::isAssociativeWithSelf (void) const {
  if (mayUpdateBeOverride()) return false;
  if (updateInstruction->isAssociative()) return true;

  /*
   * LLVM does not consider floating point operations associative
   * Strictly speaking, yes. For the sake of parallelism,
   */
  if (isAdd()) return true;
  if (isMul()) return true;

  return isSubTransformableToAdd();
}

bool EvolutionUpdate::isTransformablyCommutativeWith (const EvolutionUpdate &otherUpdate) const {

  /*
   * A pre-requisite is that both updates are commutative on their own
   */
  if (!this->isTransformablyCommutativeWithSelf()
    || !otherUpdate.isTransformablyCommutativeWithSelf()) return false;

  /*
   * A commutative subtraction means it is just the addition of a negative external,
   * so adds and subtractions are mutually commutative
   * 
   * Multiplication is not mutually commutative with any other than multiplication
   * 
   * Logical operators are only mutually commutative with each other
   */
  if (isBothUpdatesAddOrSub(otherUpdate)) return true;
  if (isBothUpdatesMul(otherUpdate)) return true;
  if (isBothUpdatesSameBitwiseLogicalOp(otherUpdate)) return true;

  return false;
}

bool EvolutionUpdate::isAssociativeWith (const EvolutionUpdate &otherUpdate) const {

  /*
   * A pre-requisite is that both updates are associative on their own
   */
  if (!this->isAssociativeWithSelf()
    || !otherUpdate.isAssociativeWithSelf()) return false;

  /*
   * An associative subtraction means it is just the addition of a negative external,
   * so adds and subtractions are mutually associative
   * 
   * Multiplication is not mutually associative with any other than multiplication
   * 
   * Logical operators are only mutually associative with each other
   */
  if (isBothUpdatesAddOrSub(otherUpdate)) return true;
  if (isBothUpdatesMul(otherUpdate)) return true;
  if (isBothUpdatesSameBitwiseLogicalOp(otherUpdate)) return true;

  return false;
}

bool EvolutionUpdate::isBothUpdatesAddOrSub (const EvolutionUpdate &otherUpdate) const {
  //FIXME understand why this is necessary
  //auto isThisAddOrSub = this->isAdd() || this->isSub();
  //auto isOtherAddOrSub = otherUpdate.isAdd() || otherUpdate.isSub();
  auto isThisAddOrSub = this->isAdd();
  auto isOtherAddOrSub = otherUpdate.isAdd();
  return isThisAddOrSub && isOtherAddOrSub;
}

bool EvolutionUpdate::isBothUpdatesMul (const EvolutionUpdate &otherUpdate) const {
  return this->isMul()
    && otherUpdate.isMul();
}

bool EvolutionUpdate::isBothUpdatesSameBitwiseLogicalOp (const EvolutionUpdate &otherUpdate) const {
  auto thisOp = this->updateInstruction->getOpcode();
  auto otherOp = otherUpdate.updateInstruction->getOpcode();
  auto isThisLogicalOp = this->updateInstruction->isBitwiseLogicOp();
  auto isOtherLogicalOp = otherUpdate.updateInstruction->isBitwiseLogicOp();
  return isThisLogicalOp && isOtherLogicalOp
    && thisOp == otherOp;
}

Instruction *EvolutionUpdate::getUpdateInstruction (void) const {
  return updateInstruction;
}

}
