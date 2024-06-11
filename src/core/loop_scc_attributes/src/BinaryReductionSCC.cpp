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
#include "arcana/noelle/core/BinaryReductionSCC.hpp"
#include "AccumulatorOpInfo.hpp"

namespace arcana::noelle {

static AccumulatorOpInfo accumOpInfo;

BinaryReductionSCC::BinaryReductionSCC(
    SCC *s,
    LoopStructure *loop,
    const std::set<DGEdge<Value, Value> *> &loopCarriedDependences,
    LoopCarriedVariable *variable,
    DominatorSummary &dom)
  : ReductionSCC(SCCKind::BINARY_REDUCTION,
                 s,
                 loop,
                 loopCarriedDependences,
                 dom) {
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
  this->setBinaryReductionInformation(this->initialValue, dom, *loop);

  return;
}

BinaryReductionSCC::BinaryReductionSCC(
    SCC *s,
    LoopStructure *loop,
    const std::set<DGEdge<Value, Value> *> &loopCarriedDependences,
    Value *initialValue,
    Instruction::BinaryOps reductionOperation,
    PHINode *accumulator,
    Value *identity)
  : ReductionSCC(SCCKind::BINARY_REDUCTION,
                 s,
                 loop,
                 loopCarriedDependences,
                 initialValue,
                 accumulator,
                 identity),
    reductionOperation{ reductionOperation } {

  return;
}

void BinaryReductionSCC::setBinaryReductionInformation(Value *initialValue,
                                                       DominatorSummary &dom,
                                                       LoopStructure &loop) {

  /*
   * Fetch the accumulators.
   */
  auto accumulators = this->collectAccumulators(loop);

  /*
   * Set the reduction operation.
   */
  auto firstAccumI = *(accumulators.begin());
  if (firstAccumI == nullptr) {
    errs()
        << "BinaryReductionSCC: Error: A binary reduction SCC relies on an unsupported reduction operation:\n ";
    errs() << "BinaryReductionSCC: SCC =\n";
    this->scc->print(errs());
    errs() << "\n";
    errs() << "BinaryReductionSCC: Loop =\n";
    loop.print(errs());
    errs() << "\n";
    assert(firstAccumI != nullptr);
  }

  auto binOpCode = firstAccumI->getOpcode();
  this->reductionOperation =
      accumOpInfo.accumOpForType(binOpCode, this->headerAccumulator->getType());

  /*
   * Set the identity value
   */
  this->identity =
      accumOpInfo.generateIdentityFor(firstAccumI,
                                      this->accumulator->getType());

  return;
}

Instruction::BinaryOps BinaryReductionSCC::getReductionOperation(void) const {
  return this->reductionOperation;
}

std::set<Instruction *> BinaryReductionSCC::collectAccumulators(
    LoopStructure &LS) {
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
    if (isa<PHINode>(V)) {
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

bool BinaryReductionSCC::classof(const GenericSCC *s) {
  return (s->getKind() == GenericSCC::SCCKind::BINARY_REDUCTION);
}

} // namespace arcana::noelle
