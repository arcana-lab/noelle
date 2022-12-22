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
#include "noelle/core/RecomputableSCC.hpp"

namespace llvm::noelle {

RecomputableSCC::RecomputableSCC(
    SCCKind K,
    SCC *s,
    LoopStructure *loop,
    const std::set<DGEdge<Value> *> &loopCarriedDependences,
    DominatorSummary &dom)
  : LoopCarriedSCC{ K, s, loop, loopCarriedDependences },
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
  this->accumulator = phi;

  return;
}

PHINode *RecomputableSCC::getPhiThatAccumulatesValuesBetweenLoopIterations(
    void) const {
  return this->accumulator;
}

bool RecomputableSCC::classof(const SCCAttrs *s) {
  return (s->getKind() >= SCCAttrs::SCCKind::RECOMPUTABLE)
         && (s->getKind() <= SCCAttrs::SCCKind::LAST_RECOMPUTABLE);
}

} // namespace llvm::noelle
