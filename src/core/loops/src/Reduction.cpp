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
#include "noelle/core/Reduction.hpp"

namespace llvm::noelle {

Reduction::Reduction(SCC *s,
                     LoopStructure *loop,
                     LoopCarriedVariable *variable,
                     DominatorSummary &dom)
  : SCCAttrs(s, loop),
    accumOpInfo{},
    lcVariable{ variable },
    accumulator{ nullptr },
    identity{ nullptr },
    accumulators{} {
  assert(s != nullptr);
  assert(loop != nullptr);
  assert(this->lcVariable != nullptr);

  /*
   * Fetch the initial value of the reduced variable.
   * This is the value the variable has just before jumping into the loop.
   */
  this->initialValue = this->lcVariable->getInitialValue();
  assert(this->initialValue != nullptr);

  /*
   * Collect the accumulators included in the SCC.
   */
  this->collectAccumulators(*loop);

  /*
   * Initialize the reduction object.
   */
  this->initializeObject(this->initialValue, this->lcVariable, dom);

  return;
}

void Reduction::initializeObject(Value *initialValue,
                                 LoopCarriedVariable *variable,
                                 DominatorSummary &dom) {

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
        << "Reduction: ERROR = the PHI node could not be found in the header of the loop.\n";
    errs() << "Reduction: SCC = ";
    this->scc->print(errs());
    errs() << "\n";
    errs() << "Reduction: Loop = ";
    loop->print(errs());
    errs() << "\n";
    abort();
  }
  assert(phiInst != nullptr);

  /*
   * Set the reduction operation.
   */
  auto firstAccumI = *(this->getAccumulators().begin());
  auto binOpCode = firstAccumI->getOpcode();
  this->reductionOperation =
      this->accumOpInfo.accumOpForType(binOpCode, phiInst->getType());

  /*
   * Fetch the PHI
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

  /*
   * Set the identity value
   */
  this->identity =
      this->accumOpInfo.generateIdentityFor(firstAccumI,
                                            this->accumulator->getType());

  return;
}

Instruction::BinaryOps Reduction::getReductionOperation(void) const {
  return this->reductionOperation;
}

bool Reduction::canExecuteReducibly(void) const {
  return true;
}

LoopCarriedVariable *Reduction::getLoopCarriedVariable(void) const {
  return this->lcVariable;
}

Value *Reduction::getInitialValue(void) const {
  return this->initialValue;
}

PHINode *Reduction::getPhiThatAccumulatesValuesBetweenLoopIterations(
    void) const {
  return this->accumulator;
}

Value *Reduction::getIdentityValue(void) const {
  return this->identity;
}

iterator_range<SCCAttrs::instruction_iterator> Reduction::getAccumulators(
    void) {
  return make_range(this->accumulators.begin(), this->accumulators.end());
}

void Reduction::collectAccumulators(LoopStructure &LS) {

  /*
   * Iterate over elements of the SCC to collect PHIs and accumulators.
   */
  for (auto iNodePair : this->scc->internalNodePairs()) {

    /*
     * Fetch the current element of the SCC.
     */
    auto V = iNodePair.first;

    /*
     * Check if it is a PHI.
     */
    if (auto phi = dyn_cast<PHINode>(V)) {
      continue;
    }

    /*
     * Check if it is an accumulator.
     */
    if (auto I = dyn_cast<Instruction>(V)) {

      /*
       * Fetch the opcode.
       */
      auto binOp = I->getOpcode();

      /*
       * Check if this is an opcode we handle.
       */
      if (accumOpInfo.accumOps.find(binOp) != accumOpInfo.accumOps.end()) {
        this->accumulators.insert(I);
        continue;
      }
    }
  }

  return;
}

} // namespace llvm::noelle
