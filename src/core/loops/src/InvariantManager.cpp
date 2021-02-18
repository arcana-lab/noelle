/*
 * Copyright 2019 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Invariants.hpp"
#include "PDGAnalysis.hpp"

using namespace llvm;
using namespace llvm::noelle;

bool arePHIIncomingValuesEquivalent (PHINode *phi) {

  std::unordered_set<Value *> incomingValues{};
  for (auto &incomingUse : phi->incoming_values()) {
    auto incomingValue = incomingUse.get();
    incomingValues.insert(incomingValue);
//    errs() << "\nBRIAN 3: " << *incomingUse << '\n';
//    errs() << "BRIAN 4: " << *incomingValue << '\n';
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

InvariantManager::InvariantManager (
  LoopStructure *loop,
  PDG *loopDG,
  TalkDown *talkdown
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

  // get exit con
  // if one of the cmp op is induction var
  // the other is invariant if we have the meta data 
//  if (talkdown) {
    TalkDownChecker talkdownChecker{loop, loopDG, this->invariants, talkdown};
//  }
  return ;
}

bool InvariantManager::isLoopInvariant (Value *value) const {
  if (!isa<Instruction>(value)){
    return true;
  }
  auto inst = cast<Instruction>(value);
//  errs() << "BRIAN 1: " << *inst << '\n';
  if (auto phi = dyn_cast<PHINode>(inst)) {
  //    errs() << "phi Incoming values are equivalent?: " << arePHIIncomingValuesEquivalent(phi) << '\n';
  }
  for (auto &i : this->invariants) {
//    errs() << "BRIAN 2, our invariants: " << *i << '\n';
  }
  if (this->invariants.find(inst) != this->invariants.end()){
    return true;
  }

  return false;
}

std::unordered_set<Instruction *> InvariantManager::getLoopInstructionsThatAreLoopInvariants (void) const {
  return this->invariants;
}

bool InvariantManager::TalkDownChecker::checkBranchOps(Value *op, LoopStructure *l) {
  if (auto I = dyn_cast<Instruction>(op)) {
    if (!l->isIncluded(I)) {
      return true;
    }
  }

  if (isa<LoadInst>(op)) {
    return true;
  }

  if (auto phi = dyn_cast<PHINode>(op)) {
    auto v0 = phi->getIncomingValue(0);
    if (checkBranchOps(v0, l)) {
        return true;
    }
    auto v1 = phi->getIncomingValue(1);
    if (checkBranchOps(v1, l)) {
        return true;
    }
  }

  return false;

}

InvariantManager::TalkDownChecker::TalkDownChecker (
  LoopStructure *loop,
  PDG *loopDG,
  std::unordered_set<Instruction *> &invariants,
  TalkDown *talkdown
) : loop{loop}, loopDG{loopDG}, invariants{invariants} {

//  errs() << "BRIAN 9, I'm in TalkDownChecker!\n";
//  errs() << "BRIAN 9, Loop: " << '\n';
  loop->print(errs());
//  errs() << "BRIAN 9, Loop Header: " << *(loop->getHeader()) << '\n';
  auto Ftree = talkdown->findTreeForFunction(loop->getHeader()->getParent());
  bool independent = false;
  for(auto &I : *(loop->getHeader())) {
    auto annot = Ftree->getAnnotationsForInst(&I);
    for (auto &an : annot) {
//      errs() << "Annotation key = " << an.getKey() << ", value = " << an.getValue() << '\n';
      if (an.getKey() == "independent" && an.getValue() == "1") {
  //      errs() << "BRIAN 9, Found an independent loop\n";
    //    errs() << "BRIAN 9, Loop = \n";
        for (auto bb : loop->getBasicBlocks()) {
          errs() << *bb << '\n';
        }
        independent = true;
      }
    }
    if (independent) {
      break;
    }
  }

    for (auto bb :  loop->getLoopExitEdges() ) {
//      errs() << "BRIAN 9, loop exit block:\n" << *(bb.first) << '\n';
      if (auto BI = dyn_cast<BranchInst> (bb.first->getTerminator())) {
  //      errs() << "BRIAN 9, branch inst = " << *BI << '\n';

        if (independent) {
        if(BI->isConditional()) {
          if (auto cmp = dyn_cast<CmpInst>(BI->getCondition())) {
            auto op0 = cmp->getOperand(0);
            bool op0Invariant = false;
            bool op1Invariant = false;
            if (checkBranchOps(op0, loop)) {
              if (auto I = dyn_cast<Instruction>(op0)) {
                this->invariants.insert(I);
                op0Invariant = true;
              }
            }

            auto op1 = cmp->getOperand(1);
            if (checkBranchOps(op1, loop)) {
              if (auto I = dyn_cast<Instruction>(op1)) {
                this->invariants.insert(I);
                op1Invariant = true;
              }
            }

            if (op0Invariant && op1Invariant) {
//              errs() << "BRIAN 9: NOOOO, BOTH ops showed invariant";
            }

//            errs() << "BRIAN 8 cmp operands: \n" << *op0 << '\n' << *op1 << '\n'; 
          }
        }
      }
    }
  }

}

InvariantManager::InvarianceChecker::InvarianceChecker (
  LoopStructure *loop,
  PDG *loopDG,
  std::unordered_set<Instruction *> &invariants
) : loop{loop}, loopDG{loopDG}, invariants{invariants} {

  /*
   * Check all instructions.
   */
  for (auto inst : loop->getInstructions()){

    /*
     * Since we will rely on data dependencies to identify loop invariants, we exclude instructions that are involved in control dependencies.
     * This means we will never identify loop invariant branches. 
     * This limitation can be avoided by generalizing the next algorithm.
     */
    if (inst->isTerminator()) {
      continue;
    }

    /*
     * Since we iterate over data dependencies that are loop values, and a PHI may be comprised of constants, we must explicitly check that all PHI incoming values are equivalent.
     */
//    errs() << "BRIAN 5: Invariant Manager " << *inst << '\n';
    bool isPHI = false;
    if (auto phi = dyn_cast<PHINode>(inst)) {
      isPHI = true;
      if (!arePHIIncomingValuesEquivalent(phi)) {
  //      errs() << "BRIAN 5: PHI incoming ar not Equiv\n";
        continue;
      }
    }

    /*
     * Skip instructions that have already been analyzed and categorized.
     */
    if (this->invariants.find(inst) != this->invariants.end()) {
      //  errs() << "BRIAN 5: Already in Invariants\n";
      continue;
    }
    if (this->notInvariants.find(inst) != this->notInvariants.end()) {
//      errs() << "BRIAN 5: Already in NOT Invariants\n";
      continue;
    }

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

    auto isEvolving = [this](Value *toValue, DGEdge<Value> *dep){
      return this->isEvolvingValue(toValue, dep);
    };
    auto canEvolve = loopDG->iterateOverDependencesTo(inst, false, true, true, isEvolving);

    /*
     * Check if the instruction is a call to a library function.
     */
    if (auto callInst = dyn_cast<CallInst>(inst)){
      auto callee = callInst->getCalledFunction();
      if (  true
          && (callee != nullptr)
          && (callee->empty())  ){

        /*
         * The instruction is a call to a library function.
         * Check if the function is pure.
         */
        if (!PDGAnalysis::isTheLibraryFunctionPure(callee)){
          canEvolve = true;
        }
      }
    }

    /*
     * Categorize the instruction.
     */
    if (canEvolve){
      this->invariants.erase(inst);
      this->notInvariants.insert(inst);

    } else {
      this->invariants.insert(inst);
    }
  }

  return ;
}

bool InvariantManager::InvarianceChecker::isEvolvingValue (Value *toValue, DGEdge<Value> *dep) {

  /*
   * Check if @toValue isn't an instruction.
   */
  if (!isa<Instruction>(toValue)){
    return false;
  }
  auto toInst = cast<Instruction>(toValue);


  if (isa<StoreInst>(toValue)) {
    return true;
  }
  /*
   * Store instructions may produce side effects
   * Currently conservative
   */
  if (isa<StoreInst>(toValue)) {
    return true;
  }

  /*
   * If the instruction is not included in the loop, then we can skip this dependence.
   */
  if (!loop->isIncluded(toInst)){
    return false;
  }

//  if (auto gep = dyn_cast<GetElementPtrInst>(toInst)) {
//    errs() << "BRIAN 21: It's a GEP: " << *gep << '\n';
  //  if (gep->hasAllConstantIndices()) {
  //    errs() << "it has all constant indices\n";
    //  errs() << "its operand is " << *(gep->getPointerOperand()) << "\n";
    //  if (auto ptrI = dyn_cast<Instruction>(gep->getPointerOperand())) {
      //  errs() << "  It's ptr operand is " << *ptrI << '\n';
        //if (loop->isIncluded(ptrI)) {
  //  //      errs() << "BRIAN 22: returning false on isEvolvignValue";
        //  return false;
        //}
     // } else {
//        errs() << "BRIAN 22: the gep ptr op is not an instruction, cannot be in our loop\n";
//        return false;
  //    }
    //}
//  }

  /*
   * The instruction is included in the loop.
   *
   * If the instruction is a memory dependence, the value may evolve.
   */
  if (dep->isMemoryDependence()){
    return true;
  }

  /*
   * Check if the values of a PHI are equivalent
   * If they are not, the PHI controls which value to use and is NOT loop invariant
   */
  auto isPHI = false;
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

  auto isEvolving = [this](Value *toValue, DGEdge<Value> *dep){
    return this->isEvolvingValue(toValue, dep);
  };
  auto canEvolve = loopDG->iterateOverDependencesTo(toInst, false, true, true, isEvolving);
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
