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

      //DDLOTT: the algorithm for determining the accumulator in SingleAccumulatorRecomputableSCC is not
      //sufficient for 2-phis case PVSCC. The representation of SARSCC is kind of fine, since only 1 phi is
      //allowed to have non-phi users, but the algorithm can't deal with picking the phi that has users.
      //errs() << "PV constructor\n";
      if(acc != nullptr) {
        //DD: responsibility for this being a PHINode lies with the SCCDAGAttrs isPeriodic analysis.
        //errs()  << *acc << "\n" << this->accumulator << "\n";
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
