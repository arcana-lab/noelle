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

using namespace llvm;
using namespace llvm::noelle;

uint64_t Hot::getSelfInstructions (Function *f) const {
  auto insts = this->functionSelfInstructions.at(f);

  return insts;
}
      
uint64_t Hot::getInvocations (Function *f) const {
  auto invs = this->functionInvocations.at(f);

  return invs;
}

void Hot::setFunctionTotalInstructions (Function *f, uint64_t totalInstructions) {
  this->functionTotalInstructions[f] = totalInstructions;
    
  return ;
}

bool Hot::isFunctionTotalInstructionsAvailable (Function &F) const {
  if (this->functionTotalInstructions.find(&F) == this->functionTotalInstructions.end()){
    return false;
  }
  return true;
}

uint64_t Hot::getTotalInstructions (Function *f) const {
  if (!this->isFunctionTotalInstructionsAvailable(*f)){
    return 0;
  }
  auto t = this->functionTotalInstructions.at(f);

  return t;
}

bool Hot::hasBeenExecuted (Function *f) const {
  if (this->getInvocations(f) == 0){
    return false;
  }

  return true;
}

double Hot::getDynamicTotalInstructionCoverage (Function *f) const {
  auto mInsts = this->getTotalInstructions();
  auto lInsts = this->getTotalInstructions(f);
  auto hotness = ((double)lInsts) / ((double)mInsts);

  return hotness;
}
