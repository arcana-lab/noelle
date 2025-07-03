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
#include "arcana/noelle/core/PeriodicVariableSCC.hpp"

namespace arcana::noelle {

PeriodicVariableSCC::PeriodicVariableSCC(
    SCC *s,
    LoopStructure *loop,
    const std::set<DGEdge<Value, Value> *> &loopCarriedDependences,
    DominatorSummary &dom,
    Value *initVal,
    Value *per,
    Value *st,
    Value *acc)
  : SingleAccumulatorRecomputableSCC{ SCCKind::PERIODIC_VARIABLE,
                                      s,
                                      loop,
                                      loopCarriedDependences,
                                      dom },
    initialValue{ initVal },
    period{ per },
    step{ st } {

  /*
   * We allow PeriodicVariableSCC to include cases that feature a single SCC
   * containing two phis when one of the phis is used only by the other phi. We
   * view this as a single accumulator case where the accumulator is said to be
   * the phi that has SCC-external users. Calling "getAccumulator" should
   * therefore return specifically the phi that has SCC-external users. The
   * algorithm for determining the accumulator in
   * SingleAccumulatorRecomputableSCC can't deal with picking the phi that has
   * users between the two phis of the 2-phis case. This code therefore handles
   * that special case in conjunction with the analysis code in SCCDAGAttrs.cpp,
   * so that "getAccumulator" will return the correct phi in the 2-phis case.
   */
  if (acc != nullptr) {
    this->accumulator = cast<PHINode>(acc);
  }

  return;
}

Value *PeriodicVariableSCC::getInitialValue(void) const {
  return this->initialValue;
}

Value *PeriodicVariableSCC::getPeriod(void) const {
  return this->period;
}

Value *PeriodicVariableSCC::getStepValue(void) const {
  return this->step;
}

bool PeriodicVariableSCC::classof(const GenericSCC *s) {
  return (s->getKind() == GenericSCC::SCCKind::PERIODIC_VARIABLE);
}

} // namespace arcana::noelle
