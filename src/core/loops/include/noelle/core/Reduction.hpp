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
#include "noelle/core/SCCAttrs.hpp"
#include "noelle/core/DominatorSummary.hpp"
#include "noelle/core/AccumulatorOpInfo.hpp"

namespace llvm::noelle {

class Reduction : public SCCAttrs {
public:
  Reduction(SCC *s,
            LoopStructure *loop,
            LoopCarriedVariable *variable,
            DominatorSummary &dom);

  Reduction() = delete;

  Instruction::BinaryOps getReductionOperation(void) const;

  bool canExecuteReducibly(void) const override;

  LoopCarriedVariable *getLoopCarriedVariable(void) const override;

  Value *getInitialValue(void) const;

  Value *getIdentityValue(void) const;

  iterator_range<instruction_iterator> getAccumulators(void);

  PHINode *getPhiThatAccumulatesValuesBetweenLoopIterations(void) const;

private:
  AccumulatorOpInfo accumOpInfo;
  Instruction::BinaryOps reductionOperation;
  LoopCarriedVariable *lcVariable;
  Value *initialValue;
  PHINode *accumulator;
  Value *identity;
  std::set<Instruction *> accumulators;

  void initializeObject(Value *initialValue,
                        LoopCarriedVariable *variable,
                        DominatorSummary &dom);

  void collectAccumulators(LoopStructure &LS);
};

} // namespace llvm::noelle
