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
    const std::set<DGEdge<Value, Value> *> &loopCarriedDependences,
    const std::set<Instruction *> &vs,
    bool commutative)
  : LoopCarriedSCC{ K, s, loop, loopCarriedDependences, commutative },
    values{ vs } {
  return;
}

RecomputableSCC::RecomputableSCC(
    SCCKind K,
    SCC *s,
    LoopStructure *loop,
    const std::set<DGEdge<Value, Value> *> &loopCarriedDependences,
    bool commutative)
  : RecomputableSCC{ K, s, loop, loopCarriedDependences, {}, commutative } {
  return;
}

void RecomputableSCC::addValue(Instruction *v) {
  this->values.insert(v);

  return;
}

std::set<Instruction *> RecomputableSCC::
    getValuesToPropagateAcrossLoopIterations(void) const {
  assert(this->values.size() > 0);
  return this->values;
}

bool RecomputableSCC::classof(const GenericSCC *s) {
  return (s->getKind() >= GenericSCC::SCCKind::RECOMPUTABLE)
         && (s->getKind() <= GenericSCC::SCCKind::LAST_RECOMPUTABLE);
}

} // namespace llvm::noelle
