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
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/Reduction.hpp"
#include "noelle/core/DominatorSummary.hpp"

namespace llvm::noelle {

class BinaryReduction : public Reduction {
public:
  BinaryReduction(SCC *s,
                  LoopStructure *loop,
                  LoopCarriedVariable *variable,
                  DominatorSummary &dom);

  BinaryReduction(SCC *s,
                  LoopStructure *loop,
                  Value *initialValue,
                  Instruction::BinaryOps reductionOperation,
                  PHINode *accumulator,
                  Value *identity);

  BinaryReduction() = delete;

  Instruction::BinaryOps getReductionOperation(void) const;

protected:
  Instruction::BinaryOps reductionOperation;

  void setBinaryReductionInformation(Value *initialValue,
                                     DominatorSummary &dom,
                                     LoopStructure &loop);

  std::set<Instruction *> collectAccumulators(LoopStructure &LS);

  iterator_range<instruction_iterator> getAccumulators(void);
};

} // namespace llvm::noelle
