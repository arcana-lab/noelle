/*
 * Copyright 2022  Simone Campanoni
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
#include "noelle/core/Reduction.hpp"

namespace llvm::noelle {

Reduction::Reduction(SCC *s,
                     AccumulatorOpInfo &opInfo,
                     LoopStructure *loop,
                     LoopCarriedVariable *variable)
  : SCCAttrs(s, opInfo, loop),
    lcVariable{ variable } {
  assert(s != nullptr);
  assert(loop != nullptr);
  assert(this->lcVariable != nullptr);

  /*
   * Set the reduction operation.
   */
  auto firstAccumI = *(this->getAccumulators().begin());
  auto binOpCode = firstAccumI->getOpcode();
  this->reductionOperation =
      opInfo.accumOpForType(binOpCode, firstAccumI->getType());

  return;
}

Instruction::BinaryOps Reduction::getReductionOperation(void) const {
  return this->reductionOperation;
}

bool Reduction::canExecuteReducibly(void) const {
  return true;
}

LoopCarriedVariable *Reduction::getLoopCarriedVariable(void) const {
  return this->lcVariable;
}

} // namespace llvm::noelle
