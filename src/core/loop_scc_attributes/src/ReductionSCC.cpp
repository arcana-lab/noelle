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
#include "noelle/core/ReductionSCC.hpp"

namespace llvm::noelle {

ReductionSCC::ReductionSCC(
    SCCKind K,
    SCC *s,
    LoopStructure *loop,
    const std::set<DGEdge<Value, Value> *> &loopCarriedDependences,
    DominatorSummary &dom)
  : LoopCarriedSCC{ K, s, loop, loopCarriedDependences, true },
    initialValue{ nullptr },
    accumulator{ nullptr },
    identity{ nullptr } {

  /*
   * Initialize the object.
   */
  this->initializeObject(*loop);

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

ReductionSCC::ReductionSCC(
    SCCKind K,
    SCC *s,
    LoopStructure *loop,
    const std::set<DGEdge<Value, Value> *> &loopCarriedDependences,
    Value *initialValue,
    PHINode *accumulator,
    Value *identity)
  : LoopCarriedSCC(K, s, loop, loopCarriedDependences, true),
    initialValue{ initialValue },
    accumulator{ accumulator },
    identity{ identity } {

  /*
   * Initialize the object.
   */
  this->initializeObject(*loop);

  return;
}

void ReductionSCC::initializeObject(LoopStructure &loop) {

  /*
   * Find the PHI of the SCC.
   * An SCC must have a PHI, it must be the only PHI in the header of the loop.
   * Notice that the PHI is the only instruction that is guaranteed to have the
   * correct type of the source-level variable being updated by this IR-level
   * SCC (the accumulator IR instruction does not.)
   */
  auto header = this->loop->getHeader();
  PHINode *phiInst = nullptr;
  for (auto n : this->scc->getNodes()) {
    auto inst = cast<Instruction>(n->getT());
    if (inst->getParent() != header) {
      continue;
    }
    if (phiInst = dyn_cast<PHINode>(inst)) {
      break;
    }
  }
  if (phiInst == nullptr) {
    errs()
        << "ReductionSCC: ERROR = the PHI node could not be found in the header of the loop.\n";
    errs() << "ReductionSCC: SCC = ";
    this->scc->print(errs());
    errs() << "\n";
    errs() << "ReductionSCC: Loop = ";
    loop.print(errs());
    errs() << "\n";
    abort();
  }
  assert(phiInst != nullptr);
  this->headerAccumulator = phiInst;

  return;
}

Value *ReductionSCC::getInitialValue(void) const {
  return this->initialValue;
}

PHINode *ReductionSCC::getPhiThatAccumulatesValuesBetweenLoopIterations(
    void) const {
  return this->accumulator;
}

Value *ReductionSCC::getIdentityValue(void) const {
  return this->identity;
}

bool ReductionSCC::classof(const GenericSCC *s) {
  return (s->getKind() >= GenericSCC::SCCKind::REDUCTION)
         && (s->getKind() <= GenericSCC::SCCKind::LAST_REDUCTION);
}

} // namespace llvm::noelle
