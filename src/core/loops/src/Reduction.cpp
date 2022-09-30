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
#include "AccumulatorOpInfo.hpp"

namespace llvm::noelle {

static AccumulatorOpInfo accumOpInfo;

Reduction::Reduction(SCC *s,
                     LoopStructure *loop,
                     LoopCarriedVariable *variable,
                     DominatorSummary &dom)
  : SCCAttrs(s, loop),
    accumulator{ nullptr },
    identity{ nullptr } {
  assert(s != nullptr);
  assert(loop != nullptr);
  assert(variable != nullptr);

  /*
   * Fetch the initial value of the reduced variable.
   * This is the value the variable has just before jumping into the loop.
   */
  this->initialValue = variable->getInitialValue();
  assert(this->initialValue != nullptr);

  /*
   * Initialize the reduction object.
   */
  this->initializeObject(this->initialValue, dom, *loop);

  return;
}

void Reduction::initializeObject(Value *initialValue,
                                 DominatorSummary &dom,
                                 LoopStructure &loop) {

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
   * Fetch the accumulators.
   */
  auto accumulators = this->collectAccumulators(loop);

  /*
   * Set the reduction operation.
   */
  auto firstAccumI = *(accumulators().begin());
  auto binOpCode = firstAccumI->getOpcode();
  this->reductionOperation =
      accumOpInfo.accumOpForType(binOpCode, phiInst->getType());

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
      accumOpInfo.generateIdentityFor(firstAccumI,
                                      this->accumulator->getType());

  return;
}

Instruction::BinaryOps Reduction::getReductionOperation(void) const {
  return this->reductionOperation;
}

bool Reduction::canExecuteReducibly(void) const {
  return true;
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

std::set<Instruction *> Reduction::collectAccumulators(LoopStructure &LS) {
  std::set<Instruction *> accumulators;

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
        accumulators.insert(I);
        continue;
      }
    }
  }

  return accumulators;
}

} // namespace llvm::noelle
