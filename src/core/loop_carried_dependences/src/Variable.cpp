/*
 * Copyright 2016 - 2021  Angelo Matni, Simone Campanoni, Brian Homerding
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
#include "arcana/noelle/core/Variable.hpp"
#include "arcana/noelle/core/LoopCarriedDependencies.hpp"

namespace arcana::noelle {

/************************************************************************************
 * LoopCarriedMemoryLocation implementation
 */

/*
 * TODO: Implement
 */
LoopCarriedMemoryLocation::LoopCarriedMemoryLocation(const LoopStructure &loop,
                                                     PDG &loopDG,
                                                     SCC &memoryLocationSCC,
                                                     Value *memoryLocation) {}

bool LoopCarriedMemoryLocation::isEvolutionReducibleAcrossLoopIterations(
    void) const {
  return false;
}

LoopCarriedMemoryLocation::~LoopCarriedMemoryLocation() {
  return;
}

/************************************************************************************
 * VariableUpdate implementation
 */

EvolutionUpdate::EvolutionUpdate(Instruction *updateInstruction,
                                 SCC *dataMemoryVariableSCC)
  : updateInstruction{ updateInstruction } {

  if (auto storeUpdate = dyn_cast<StoreInst>(updateInstruction)) {

    /*
     * No understanding from the StoreInst of the values used to derive this
     * stored value needs to be found. It will be found in the VariableUpdate
     * producing the stored value.
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

bool EvolutionUpdate::mayUpdateBeOverride(void) const {
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
  if (isa<CallInst>(updateInstruction))
    return true;

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
   * This defers the decision of overriding to the select instruction's
   * VariableUpdate
   */
  if (isa<CmpInst>(updateInstruction)) {
    for (auto user : updateInstruction->users()) {
      if (isa<SelectInst>(user))
        continue;
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

bool EvolutionUpdate::isCommutativeWithSelf(void) const {
  if (mayUpdateBeOverride())
    return false;
  return updateInstruction->isCommutative();
}

bool EvolutionUpdate::isAdd(void) const {
  auto op = updateInstruction->getOpcode();
  return Instruction::Add == op || Instruction::FAdd == op;
}

bool EvolutionUpdate::isMul(void) const {
  auto op = updateInstruction->getOpcode();
  return Instruction::Mul == op || Instruction::FMul == op;
}

bool EvolutionUpdate::isSub(void) const {
  auto op = updateInstruction->getOpcode();
  return Instruction::Sub == op || Instruction::FSub == op;
}

/*
 * LLVM cannot analyze the instruction to a point where it
 * considers subtraction by an external value equivalent to
 * addition of the negative of that external value
 */
bool EvolutionUpdate::isSubTransformableToAdd() const {
  if (!isSub())
    return false;
  auto &useOfValueBeingSubtracted = updateInstruction->getOperandUse(1);
  return externalValuesUsed.find(&useOfValueBeingSubtracted)
         != externalValuesUsed.end();
}

bool EvolutionUpdate::isTransformablyCommutativeWithSelf(void) const {
  if (mayUpdateBeOverride())
    return false;
  if (updateInstruction->isCommutative())
    return true;

  return isSubTransformableToAdd();
}

bool EvolutionUpdate::isAssociativeWithSelf(void) const {
  if (mayUpdateBeOverride())
    return false;
  if (updateInstruction->isAssociative())
    return true;

  /*
   * LLVM does not consider floating point operations associative
   * Strictly speaking, yes. For the sake of parallelism,
   */
  if (isAdd())
    return true;
  if (isMul())
    return true;

  return isSubTransformableToAdd();
}

bool EvolutionUpdate::isTransformablyCommutativeWith(
    const EvolutionUpdate &otherUpdate) const {

  /*
   * A pre-requisite is that both updates are commutative on their own
   */
  if (!this->isTransformablyCommutativeWithSelf()
      || !otherUpdate.isTransformablyCommutativeWithSelf())
    return false;

  /*
   * A commutative subtraction means it is just the addition of a negative
   * external, so adds and subtractions are mutually commutative
   *
   * Multiplication is not mutually commutative with any other than
   * multiplication
   *
   * Logical operators are only mutually commutative with each other
   */
  if (isBothUpdatesAddOrSub(otherUpdate))
    return true;
  if (isBothUpdatesMul(otherUpdate))
    return true;
  if (isBothUpdatesSameBitwiseLogicalOp(otherUpdate))
    return true;

  return false;
}

bool EvolutionUpdate::isAssociativeWith(
    const EvolutionUpdate &otherUpdate) const {

  /*
   * A pre-requisite is that both updates are associative on their own
   */
  if (!this->isAssociativeWithSelf() || !otherUpdate.isAssociativeWithSelf())
    return false;

  /*
   * An associative subtraction means it is just the addition of a negative
   * external, so adds and subtractions are mutually associative
   *
   * Multiplication is not mutually associative with any other than
   * multiplication
   *
   * Logical operators are only mutually associative with each other
   */
  if (isBothUpdatesAddOrSub(otherUpdate))
    return true;
  if (isBothUpdatesMul(otherUpdate))
    return true;
  if (isBothUpdatesSameBitwiseLogicalOp(otherUpdate))
    return true;

  return false;
}

bool EvolutionUpdate::isBothUpdatesAddOrSub(
    const EvolutionUpdate &otherUpdate) const {
  // FIXME understand why this is necessary
  // auto isThisAddOrSub = this->isAdd() || this->isSub();
  // auto isOtherAddOrSub = otherUpdate.isAdd() || otherUpdate.isSub();
  auto isThisAddOrSub = this->isAdd();
  auto isOtherAddOrSub = otherUpdate.isAdd();
  return isThisAddOrSub && isOtherAddOrSub;
}

bool EvolutionUpdate::isBothUpdatesMul(
    const EvolutionUpdate &otherUpdate) const {
  return this->isMul() && otherUpdate.isMul();
}

bool EvolutionUpdate::isBothUpdatesSameBitwiseLogicalOp(
    const EvolutionUpdate &otherUpdate) const {
  auto thisOp = this->updateInstruction->getOpcode();
  auto otherOp = otherUpdate.updateInstruction->getOpcode();
  auto isThisLogicalOp = this->updateInstruction->isBitwiseLogicOp();
  auto isOtherLogicalOp = otherUpdate.updateInstruction->isBitwiseLogicOp();
  return isThisLogicalOp && isOtherLogicalOp && thisOp == otherOp;
}

Instruction *EvolutionUpdate::getUpdateInstruction(void) const {
  return updateInstruction;
}

} // namespace arcana::noelle
