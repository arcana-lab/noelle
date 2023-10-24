/*
 * Copyright 2016 - 2021  Angelo Matni, Simone Campanoni, Brian Homerding
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
#include "noelle/core/LoopCarriedDependencies.hpp"

namespace llvm::noelle {

void LoopCarriedDependencies::setLoopCarriedDependencies(
    LoopTree *loopNode,
    const DominatorSummary &DS,
    PDG &dgForLoops) {
  for (auto edge : dgForLoops.getEdges()) {
    assert(!edge->isLoopCarriedDependence() && "Flag was already set");
  }

  for (auto edge : dgForLoops.getEdges()) {
    if (!LoopCarriedDependencies::isALoopCarriedDependence(loopNode,
                                                           DS,
                                                           edge)) {
      continue;
    }

    auto loop =
        LoopCarriedDependencies::isALoopCarriedDependence(loopNode, DS, edge);
    if (!loop) {
      continue;
    }
    edge->setLoopCarried(true);
  }

  return;
}

bool LoopCarriedDependencies::isALoopCarriedDependence(
    LoopTree *loopNode,
    const DominatorSummary &DS,
    DGEdge<Value, Value> *edge) {

  /*
   * Fetch the loop.
   */
  auto topLoop = loopNode->getLoop();
  assert(topLoop != nullptr);
  auto topLoopHeader = topLoop->getHeader();
  auto topLoopHeaderBranch = topLoopHeader->getTerminator();

  /*
   * Fetch the instructions involved in the dependence.
   */
  auto producer = edge->getSrc();
  auto consumer = edge->getDst();

  /*
   * Only dependences between instructions can be loop-carried.
   */
  if (!isa<Instruction>(producer)) {
    return false;
  }
  if (!isa<Instruction>(consumer)) {
    return false;
  }

  /*
   * Fetch the instructions involved in the dependence.
   */
  auto producerI = dyn_cast<Instruction>(producer);
  auto consumerI = dyn_cast<Instruction>(consumer);

  /*
   * Fetch the innermost loops that contain the two instructions.
   */
  auto producerLoop = loopNode->getInnermostLoopThatContains(producerI);
  auto consumerLoop = loopNode->getInnermostLoopThatContains(consumerI);

  /*
   * If either of the instruction does not belong to a loop, then the dependence
   * cannot be loop-carried.
   */
  if (!producerLoop || !consumerLoop) {
    return false;
  }

  /*
   * If the dependence is a control one and the two instructions belong to a
   * subloop, then this cannot be a loop-carried one for the target loop.
   */
  if (edge->isControlDependence() && (producerLoop != loopNode->getLoop())
      && (consumerLoop != loopNode->getLoop())) {
    return false;
  }

  /*
   * Check if both instructions will access the same element (variable, memory
   * location) in the same iteration
   */
  auto doTheyTouchTheSameElementInTheSameIteration = true;
  if (edge->isMemoryDependence()) {

    /*
     * Fetch the pointer of the location accessed by the producer.
     */
    Value *producerPointer = nullptr;
    if (auto load = dyn_cast<LoadInst>(producerI)) {
      producerPointer = load->getPointerOperand();
    } else if (auto store = dyn_cast<StoreInst>(producerI)) {
      producerPointer = store->getPointerOperand();
    }

    /*
     * Fetch the pointer of the location accessed by the consumer.
     */
    Value *consumerPointer = nullptr;
    if (auto load = dyn_cast<LoadInst>(consumerI)) {
      consumerPointer = load->getPointerOperand();
    } else if (auto store = dyn_cast<StoreInst>(consumerI)) {
      consumerPointer = store->getPointerOperand();
    }

    /*
     * If we cannot identify the single pointer for each instruction, then we
     * cannot use dominance to determine whether the dependence is loop-carried
     * or not.
     */
    if ((producerPointer == nullptr) || (consumerPointer == nullptr)) {
      doTheyTouchTheSameElementInTheSameIteration = false;

    } else {

      /*
       * Each instruction can only access a single memory location per
       * iteration.
       *
       * Check whether they access the same location per iteration or not.
       */
      if (producerPointer != consumerPointer) {
        doTheyTouchTheSameElementInTheSameIteration = false;

      } else {
        assert(producerPointer == consumerPointer);
        auto pointerAsInst = dyn_cast<Instruction>(producerPointer);
        if (pointerAsInst == nullptr) {
          doTheyTouchTheSameElementInTheSameIteration = false;
        } else {
          if (topLoop->isIncluded(pointerAsInst)) {
            doTheyTouchTheSameElementInTheSameIteration = false;
          }
        }
      }
    }
  }
  if (!doTheyTouchTheSameElementInTheSameIteration) {
    return true;
  }

  if (producerI == consumerI || !DS.DT.dominates(producerI, consumerI)) {

    /*
     * Check if the dependence is data and via variable.
     */
    if (!edge->isMemoryDependence() && edge->isDataDependence()) {

      /*
       * The data dependence is variable based
       *
       * If the producer cannot reach the header of the loop without reaching
       * the consumer, then the dependence cannot be loop-carried.
       */
      auto producerB = producerI->getParent();
      auto consumerB = consumerI->getParent();
      auto mustProducerReachConsumerBeforeHeader =
          !canBasicBlockReachHeaderBeforeOther(*consumerLoop,
                                               producerB,
                                               consumerB);
      if (mustProducerReachConsumerBeforeHeader) {
        return false;
      }

      /*
       * The data dependence is variable based.
       * The producer can reach the header before reaching the consumer.
       *
       * Check if the consumer will take the value from someone else when the
       * execution comes from the header rather than the producer of the
       * previous iteration
       */
      if (DS.DT.dominates(consumerI, producerI)
          && DS.DT.dominates(topLoopHeaderBranch, consumerI)) {
        if (auto phiConsumer = dyn_cast<PHINode>(consumerI)) {
          return false;
        }
      }
    }

    return true;
  }

  return false;
}

std::set<DGEdge<Value, Value> *> LoopCarriedDependencies::
    getLoopCarriedDependenciesForLoop(const LoopStructure &LS,
                                      LoopTree *loopNode,
                                      PDG &LoopDG) {

  std::set<DGEdge<Value, Value> *> LCEdges;
  for (auto edge : LoopDG.getEdges()) {
    if (!edge->isLoopCarriedDependence()) {
      continue;
    }

    auto consumer = edge->getDst();
    auto consumerI = cast<Instruction>(consumer);
    auto consumerLoop = loopNode->getInnermostLoopThatContains(consumerI);
    if (consumerLoop != &LS) {
      continue;
    }

    LCEdges.insert(edge);
  }

  return LCEdges;
}

std::set<DGEdge<Value, Value> *> LoopCarriedDependencies::
    getLoopCarriedDependenciesForLoop(const LoopStructure &LS,
                                      LoopTree *loopNode,
                                      SCCDAG &sccdag) {

  std::set<DGEdge<Value, Value> *> LCEdges;

  for (auto sccNode : sccdag.getNodes()) {
    auto scc = sccNode->getT();
    for (auto edge : scc->getEdges()) {
      if (!edge->isLoopCarriedDependence()) {
        continue;
      }

      auto consumer = edge->getDst();
      auto consumerI = cast<Instruction>(consumer);
      auto consumerLoop = loopNode->getInnermostLoopThatContains(consumerI);
      if (consumerLoop != &LS) {
        continue;
      }

      auto producer = edge->getSrc();
      auto producerI = dyn_cast<Instruction>(producer);
      if (producerI == NULL) {
        continue;
      }

      auto producerLoop = loopNode->getInnermostLoopThatContains(producerI);
      if (!producerLoop) {
        continue;
      }
      LCEdges.insert(edge);
    }
  }
  return LCEdges;
}

bool LoopCarriedDependencies::canBasicBlockReachHeaderBeforeOther(
    const LoopStructure &LS,
    BasicBlock *I,
    BasicBlock *J) {

  assert(LS.isIncluded(I) && LS.isIncluded(J));

  // I->print(errs() << "Source:\n");
  // J->print(errs() << "Destination:\n");

  /*
   * If the source is the destination, the loop must be at a later iteration
   */
  if (I == J) {
    return true;
  }

  auto header = LS.getHeader();
  auto exitsVector = LS.getLoopExitBasicBlocks();
  std::set<BasicBlock *> exits(exitsVector.begin(), exitsVector.end());
  std::queue<BasicBlock *> queue;
  std::unordered_set<BasicBlock *> enqueued;
  queue.push(I);
  enqueued.insert(I);
  bool isJReached = false;

  while (!queue.empty()) {
    auto B = queue.front();
    queue.pop();

    /*
     * Check if the successor is the header block
     * Check if the successor is an exit block; if so, do not traverse further
     * Check if the destination is reached; if so, do not traverse further
     */
    if (B == header)
      return true;
    if (exits.find(B) != exits.end())
      continue;
    if (B == J) {
      isJReached = true;
      continue;
    }

    for (auto succIter = succ_begin(B); succIter != succ_end(B); ++succIter) {
      auto succ = *succIter;

      /*
       * Do not re-traverse enqueued blocks
       */
      if (enqueued.find(succ) != enqueued.end())
        continue;
      queue.push(succ);
      enqueued.insert(succ);
    }
  }

  /*
   * The header was never reached
   */
  assert(isJReached);
  return false;
}

} // namespace llvm::noelle
