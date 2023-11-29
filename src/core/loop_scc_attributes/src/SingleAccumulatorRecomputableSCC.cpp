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
#include "noelle/core/SingleAccumulatorRecomputableSCC.hpp"

namespace arcana::noelle {

SingleAccumulatorRecomputableSCC::SingleAccumulatorRecomputableSCC(
    SCCKind K,
    SCC *s,
    LoopStructure *loop,
    const std::set<DGEdge<Value, Value> *> &loopCarriedDependences,
    DominatorSummary &dom)
  : RecomputableSCC{ K, s, loop, loopCarriedDependences, true },
    accumulator{ nullptr } {

  /*
   * Fetch the accumulator
   */
  PHINode *phi = nullptr;
  for (auto phiCandidate : this->getPHIs()) {
    auto found = true;
    for (auto currentPhi : this->getPHIs()) {
      if (!dom.DT.dominates(phiCandidate, currentPhi)) {
        found = false;
        break;
      }
    }
    if (found) {
      phi = phiCandidate;
      break;
    }
  }
  if (phi == nullptr) {
    abort();
  }
  this->addValue(phi);
  this->accumulator = phi;

  return;
}

PHINode *SingleAccumulatorRecomputableSCC::
    getPhiThatAccumulatesValuesBetweenLoopIterations(void) const {
  assert(this->accumulator != nullptr);
  return this->accumulator;
}

bool SingleAccumulatorRecomputableSCC::classof(const GenericSCC *s) {
  return (s->getKind() >= GenericSCC::SCCKind::SINGLE_ACCUMULATOR_RECOMPUTABLE)
         && (s->getKind()
             <= GenericSCC::SCCKind::LAST_SINGLE_ACCUMULATOR_RECOMPUTABLE);
}

} // namespace arcana::noelle
