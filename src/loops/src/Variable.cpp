/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Variable.hpp"

using namespace llvm;

PDG *produceDataAndMemoryOnlyDGFromVariableDG(PDG &variableDG) {

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

Variable::Variable (
  const LoopStructure &loop,
  const LoopCarriedDependencies &LCD,
  PDG &loopDG,
  SCC &sccContainingVariable,
  PHINode *declarationPHI
) : outermostLoopOfVariable{loop}, declarationValue{declarationPHI}, isDataVariable{true}, isValid{false} {

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
  auto loopCarriedDependencies = LCD.getLoopCarriedDependenciesForLoop(loop);
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
    if (isa<CastInst>(value)) continue;
    if (isa<LoadInst>(value)) continue;

    auto variableUpdate = new VariableUpdate(cast<Instruction>(value), sccOfDataAndMemoryVariableValuesOnly);
    variableUpdates.insert(variableUpdate);

    if (loopCarriedValues.find(value) == loopCarriedValues.end()) continue;
    loopCarriedVariableUpdates.insert(variableUpdate);
  }

  this->isValid = true;
}

/*
 * TODO: Implement
 */
Variable::Variable (
  const LoopStructure &loop,
  const LoopCarriedDependencies &LCD,
  PDG &loopDG,
  SCC &variableSCC,
  Value *memoryLocation
) : outermostLoopOfVariable{loop}, declarationValue{memoryLocation}, isDataVariable{false} {
  this->isValid = false;
}

bool Variable::isEvolutionReducibleAcrossLoopIterations (void) const {

  if (!isValid) return false;

  // declarationValue->print(errs() << "Declaration: "); errs() << "\n";
  // sccOfDataAndMemoryVariableValuesOnly->printMinimal(errs() << "Data and memory SCC\n");
  // errs() << "Number of internal control values: " << controlValuesGoverningEvolution.size() << "\n";
  // for (auto controlValue : controlValuesGoverningEvolution) {
  //   controlValue->print(errs() << "Control value: "); errs() << "\n";
  // }
  // errs() << "Number of variable updates: " << variableUpdates.size() << "\n";

  /*
   * No control values internal to the variable can be involved in the evolution
   * This would prevent partial computation and then reduction as the condition results might change
   */
  for (auto controlValue : controlValuesGoverningEvolution) {
    if (sccOfVariableOnly->isInternal(controlValue)) return false;
  }

  for (auto update : variableUpdates) {

    /*
     * Overriding updates break reducibility
     */
    if (update->mayUpdateBeOverride()) return false;

    /*
     * All updates must be mutually commutative and associative
     * TODO: does transitivity of those two properties allow only checking one update against all others?
     */
    for (auto otherUpdate : variableUpdates) {
      if (!update->isTransformablyCommutativeWith(*otherUpdate)) return false;
      if (!update->isAssociativeWith(*otherUpdate)) return false;
    }

  }

  /*
   * A reducible variable cannot have produce any dependencies in the loop
   */
  for (auto externalNodePair : sccOfVariableOnly->externalNodePairs()) {
    auto value = externalNodePair.first;
    if (!isa<Instruction>(value)) continue;

    /*
     * Ignore externals outside the loop, such as live outs
     * They won't prevent reducibility
     */
    auto inst = cast<Instruction>(value);
    if (!outermostLoopOfVariable.isIncluded(inst)) continue;

    auto node = externalNodePair.second;
    for (auto edge : node->getIncomingEdges()) {
      auto producer = edge->getOutgoingT();
      if (sccOfVariableOnly->isInternal(producer)) return false;
    }
  }

  return true;
}

/************************************************************************************
 * VariableUpdate implementation
 */

VariableUpdate::VariableUpdate (Instruction *updateInstruction, SCC *dataMemoryVariableSCC)
  : updateInstruction{updateInstruction} {

  if (auto storeUpdate = dyn_cast<StoreInst>(updateInstruction)) {

    /*
     * No understanding from the StoreInst of the values used to derive this stored value
     * needs to be found. It will be found in the VariableUpdate producing the stored value.
     */
    this->newVariableValue = storeUpdate->getValueOperand();
    return;
  }
  this->newVariableValue = updateInstruction;

  for (auto &use : updateInstruction->operands()) {
    auto usedValue = use.get();

    if (dataMemoryVariableSCC->isInternal(usedValue)) {
      oldVariableValuesUsed.insert(&use);
    } else {
      externalValuesUsed.insert(&use);
    }
  }
}

bool VariableUpdate::mayUpdateBeOverride (void) const {
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
    return oldVariableValuesUsed.size() == 0;
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

bool VariableUpdate::isCommutativeWithSelf (void) const {
  if (mayUpdateBeOverride()) return false;
  return updateInstruction->isCommutative();
}

bool VariableUpdate::isAdd (void) const {
  auto op = updateInstruction->getOpcode();
  return Instruction::Add == op
    || Instruction::FAdd == op;
}

bool VariableUpdate::isMul (void) const {
  auto op = updateInstruction->getOpcode();
  return Instruction::Mul == op
    || Instruction::FMul == op;
}

bool VariableUpdate::isSub (void) const {
  auto op = updateInstruction->getOpcode();
  return Instruction::Sub == op
    || Instruction::FSub == op;
}

/*
 * LLVM cannot analyze the instruction to a point where it
 * considers subtraction by an external value equivalent to
 * addition of the negative of that external value
 */
bool VariableUpdate::isSubTransformableToAdd () const {
  if (!isSub()) return false;
  auto &useOfValueBeingSubtracted = updateInstruction->getOperandUse(1);
  return externalValuesUsed.find(&useOfValueBeingSubtracted) != externalValuesUsed.end();
}

bool VariableUpdate::isTransformablyCommutativeWithSelf (void) const {
  if (mayUpdateBeOverride()) return false;
  if (updateInstruction->isCommutative()) return true;

  return isSubTransformableToAdd();
}

bool VariableUpdate::isAssociativeWithSelf (void) const {
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

bool VariableUpdate::isTransformablyCommutativeWith (const VariableUpdate &otherUpdate) const {

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
  if (isBothUpdatesSameBitwiseLogicalOp(otherUpdate)); return true;

  return false;
}

bool VariableUpdate::isAssociativeWith (const VariableUpdate &otherUpdate) const {

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
  if (isBothUpdatesSameBitwiseLogicalOp(otherUpdate)); return true;

  return false;
}

bool VariableUpdate::isBothUpdatesAddOrSub (const VariableUpdate &otherUpdate) const {
  auto isThisAddOrSub = this->isAdd() || otherUpdate.isSub();
  auto isOtherAddOrSub = this->isAdd() || otherUpdate.isSub();
  return isThisAddOrSub && isOtherAddOrSub;
}

bool VariableUpdate::isBothUpdatesMul (const VariableUpdate &otherUpdate) const {
  return this->isMul()
    && otherUpdate.isMul();
}

bool VariableUpdate::isBothUpdatesSameBitwiseLogicalOp (const VariableUpdate &otherUpdate) const {
  auto thisOp = this->updateInstruction->getOpcode();
  auto otherOp = otherUpdate.updateInstruction->getOpcode();
  auto isThisLogicalOp = this->updateInstruction->isBitwiseLogicOp();
  auto isOtherLogicalOp = otherUpdate.updateInstruction->isBitwiseLogicOp();
  return isThisLogicalOp && isOtherLogicalOp
    && thisOp == otherOp;
}

