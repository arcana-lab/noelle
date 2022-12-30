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
#include "noelle/core/LoopCarriedSCC.hpp"

namespace llvm::noelle {

LoopCarriedSCC::LoopCarriedSCC(
    SCC *s,
    LoopStructure *loop,
    const std::set<DGEdge<Value> *> &loopCarriedDependences)
  : LoopCarriedSCC(SCCAttrs::SCCKind::LOOP_CARRIED,
                   s,
                   loop,
                   loopCarriedDependences) {
  return;
}

LoopCarriedSCC::LoopCarriedSCC(
    SCCKind K,
    SCC *s,
    LoopStructure *loop,
    const std::set<DGEdge<Value> *> &loopCarriedDependences)
  : SCCAttrs(K, s, loop),
    lcDeps{ loopCarriedDependences } {
  return;
}

std::set<DGEdge<Value> *> LoopCarriedSCC::getLoopCarriedDependences(
    void) const {
  return this->lcDeps;
}

bool LoopCarriedSCC::classof(const SCCAttrs *s) {
  return (s->getKind() >= SCCAttrs::SCCKind::LOOP_CARRIED)
         && (s->getKind() <= SCCAttrs::SCCKind::LAST_LOOP_CARRIED);
}

} // namespace llvm::noelle