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
#include "noelle/core/Dominators.hpp"
#include "noelle/core/LoopCarriedSCC.hpp"

namespace llvm::noelle {

class ReductionSCC : public LoopCarriedSCC {
public:
  ReductionSCC() = delete;

  Value *getInitialValue(void) const;

  Value *getIdentityValue(void) const;

  PHINode *getPhiThatAccumulatesValuesBetweenLoopIterations(void) const;

  static bool classof(const SCCAttrs *s);

protected:
  Value *initialValue;
  PHINode *accumulator;
  Value *identity;
  PHINode *headerAccumulator;

  ReductionSCC(SCCKind K,
               SCC *s,
               LoopStructure *loop,
               const std::set<DGEdge<Value> *> &loopCarriedDependences,
               DominatorSummary &dom);

  ReductionSCC(SCCKind K,
               SCC *s,
               LoopStructure *loop,
               const std::set<DGEdge<Value> *> &loopCarriedDependences,
               Value *initialValue,
               PHINode *accumulator,
               Value *identity);

  void initializeObject(LoopStructure &loop);
};

} // namespace llvm::noelle
