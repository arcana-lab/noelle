/*
 * Copyright 2019 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Invariants.hpp"

using namespace llvm;

InvariantManager::InvariantManager (
  LoopStructure *loop,
  PDG *loopDG
  ){

  /*
   * Check every instruction of the loop.
   */
  for (auto inst : loop->getInstructions()){

    /*
     * Check if it is loop invariant according to the loop structure.
     */
    if (loop->isLoopInvariant(inst)){

      /*
       * @inst is a loop invariant.
       */
      this->invariants.insert(inst);
      continue ;
    }
  }

  /*
   * Traverse the dependence graph to identify loop invariants the LoopStructure conservatively didn't identify
   */
  InvarianceChecker checker{loop, loopDG, this->invariants};

  return ;
}

bool InvariantManager::isLoopInvariant (Value *value) const {
  if (!isa<Instruction>(value)){
    return true;
  }
  auto inst = cast<Instruction>(value);
  if (this->invariants.find(inst) != this->invariants.end()){
    return true;
  }

  return false;
}

std::unordered_set<Instruction *> InvariantManager::getLoopInstructionsThatAreLoopInvariants (void) const {
  return this->invariants;
}

InvariantManager::InvarianceChecker::InvarianceChecker (
  LoopStructure *loop,
  PDG *loopDG,
  std::unordered_set<Instruction *> &invariants
) : loop{loop}, loopDG{loopDG}, invariants{invariants} {

  this->isEvolving = std::bind(&InvarianceChecker::isEvolvingValue, this, std::placeholders::_1, std::placeholders::_2);

  for (auto inst : loop->getInstructions()){

    /*
     * Since we iterate over data dependencies, we must explicitly exclude control values
     */
    if (inst->isTerminator()) continue;

    /*
     * Since we iterate over data dependencies that are loop values, and a PHI may be comprised of constants,
     * we must explicitly check that all PHI incoming values are equivalent
     */
    bool isPHI = false;
    if (auto phi = dyn_cast<PHINode>(inst)) {
      isPHI = true;
      if (!arePHIIncomingValuesEquivalent(phi)) continue;
    }

    if (this->invariants.find(inst) != this->invariants.end()) continue;
    if (this->notInvariants.find(inst) != this->notInvariants.end()) continue;

    this->dependencyValuesBeingChecked.clear();
    this->dependencyValuesBeingChecked.insert(inst);

    /*
     * If this instruction is a PHI, we claim it is invariant so should its equivalent
     * values only have a data dependency on it, they correctly claim they are invariant instead
     * of conservatively claiming variance due to the cycle
     */
    if (isPHI) {
      this->invariants.insert(inst);
    }

    auto canEvolve = loopDG->iterateOverDependencesTo(inst, false, true, true, isEvolving);

    /*
     * Check if the instruction is a call to a library function.
     */
    if (auto callInst = dyn_cast<CallInst>(inst)){
      auto callee = callInst->getCalledFunction();
      if (  true
          && (callee != nullptr)
          && (callee->empty())  ){
        if (callee->getName() == "malloc" || callee->getName() == "rand"){
          canEvolve = true;
        }
      }
    }
    if (canEvolve){
      this->invariants.erase(inst);
      this->notInvariants.insert(inst);

    } else {
      this->invariants.insert(inst);
    }
  }

}

bool InvariantManager::InvarianceChecker::isEvolvingValue (Value *toValue, DataDependenceType ddType) {

  /*
   * Check if @toValue isn't an instruction.
   */
  if (!isa<Instruction>(toValue)){
    return false;
  }
  auto toInst = cast<Instruction>(toValue);

  /*
   * If the instruction is not included in the loop, then we can skip this dependence.
   */
  if (!loop->isIncluded(toInst)){
    return false;
  }

  /*
   * If the instruction is included in the loop and this is a memory dependence, the value may evolve
   */
  if (ddType != DataDependenceType::DG_DATA_NONE){
    return true;
  }

  /*
   * Check if the values of a PHI are equivalent
   * If they are not, the PHI controls which value to use and is NOT loop invariant
   */
  bool isPHI = false;
  if (auto phi = dyn_cast<PHINode>(toInst)) {
    isPHI = true;
    if (!arePHIIncomingValuesEquivalent(phi)) return true;
  }

  /*
   * @toInst is part of the loop.
   * We need to check if @toInst is a loop invariant.
   */
  if (invariants.find(toInst) != invariants.end()){
    return false;
  }
  if (notInvariants.find(toInst) != notInvariants.end()){
    return true;
  }

  /*
   * If this instruction is a PHI, we claim it is invariant so should its equivalent
   * values only have a data dependency on it, they correctly claim they are invariant instead
   * of conservatively claiming variance due to the cycle
   */
  if (isPHI) {
    invariants.insert(toInst);
  }

  /*
   * A cycle has occurred in our dependence graph traversal. The cycle may evolve
   */
  if (this->dependencyValuesBeingChecked.find(toInst) != this->dependencyValuesBeingChecked.end()){
    return true;
  }
  this->dependencyValuesBeingChecked.insert(toInst);

  bool canEvolve = loopDG->iterateOverDependencesTo(toInst, false, true, true, isEvolving);
  if (canEvolve) {
    invariants.erase(toInst);
    notInvariants.insert(toInst);
  } else {
    invariants.insert(toInst);
  }

  return canEvolve;
}

bool InvariantManager::InvarianceChecker::arePHIIncomingValuesEquivalent (PHINode *phi) {

  std::unordered_set<Value *> incomingValues{};
  for (auto &incomingUse : phi->incoming_values()) {
    auto incomingValue = incomingUse.get();
    incomingValues.insert(incomingValue);
  }
  if (incomingValues.size() == 0) return false;

  /*
   * If all incoming values are strictly the same value, this set will be one element
   */
  if (incomingValues.size() == 1) return true;

  /*
   * Check if all incoming values are strictly equivalent
   */
  Value *singleUniqueValue = *incomingValues.begin();
  for (auto incomingValue : incomingValues) {
    if (incomingValue == singleUniqueValue) continue;
    singleUniqueValue = nullptr;
    break;
  }
  if (singleUniqueValue) return true;

  /*
   * If all incoming values are loads of the same global, we consider this equivalent
   * Whether these loads are loop invariant is up to checks on the dependence graph
   */
  GlobalValue *singleGlobalLoaded = nullptr;
  for (auto incomingValue : incomingValues) {
    if (auto load = dyn_cast<LoadInst>(incomingValue)) {
      auto loadedValue = load->getPointerOperand();
      if (auto global = dyn_cast<GlobalValue>(loadedValue)) {
        if (singleGlobalLoaded == nullptr || singleGlobalLoaded == global) {
          singleGlobalLoaded = global;
          continue;
        }
      }
    }

    singleGlobalLoaded = nullptr;
    break;
  }

  if (singleGlobalLoaded != nullptr) return true;

  return false;
}
