/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"
#include "Hot.hpp"

using namespace llvm ;

uint64_t Hot::getStaticInstructions (Instruction *i) const {
  return 1;
}

uint64_t Hot::getInvocations (Instruction *i) const {
  auto bb = i->getParent();

  auto inv = this->getInvocations(bb);

  return inv;
}

uint64_t Hot::getTotalInstructions (Instruction *i) const {
  if (this->instructionTotalInstructions.find(i) == this->instructionTotalInstructions.end()){

    /*
     * This is not a call instruction.
     * Hence, self instructions is equal to total instructions.
     */
    return this->getInvocations(i);
  }

  /*
   * This is a call instruction.
   */
  return this->instructionTotalInstructions.at(i);
}
      
uint64_t Hot::getSelfInstructions (Instruction *i) const {
  return this->getInvocations(i);
}

bool Hot::hasBeenExecuted (Instruction *i) const {
  if (this->getInvocations(i) == 0){
    return false;
  }

  return true;
}
