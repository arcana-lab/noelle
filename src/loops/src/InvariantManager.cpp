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

    /*
     * Check if @value is loop invariant according to the dependence graph.
     */
    auto checkValueFunc = [loop] (Value *toValue, DataDependenceType ddType) -> bool {

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
       * @toInst is part of the loop.
       * We need to check if @toInst is a loop invariant.
       */
      //TODO

      return true;
    };
    auto canEvolve = loopDG->iterateOverDependencesTo(inst, false, true, true, checkValueFunc);
    auto isInvariant = !canEvolve;
    if (isInvariant){
      this->invariants.insert(inst);
    }
  }

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
