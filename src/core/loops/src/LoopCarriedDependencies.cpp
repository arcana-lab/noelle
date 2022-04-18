/*
 * Copyright 2016 - 2021  Angelo Matni, Simone Campanoni, Brian Homerding
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopCarriedDependencies.hpp"

namespace llvm::noelle {

void LoopCarriedDependencies::setLoopCarriedDependencies (
  StayConnectedNestedLoopForestNode *loopNode,
  const DominatorSummary &DS,
  PDG &dgForLoops
) {
  for (auto edge : dgForLoops.getEdges()) {
    assert(!edge->isLoopCarriedDependence() && "Flag was already set");
  }

  for (auto edge : dgForLoops.getEdges()) {
    auto loop = getLoopOfLCD(loopNode, DS, edge);
    if (!loop) {
      continue;
    }
    edge->setLoopCarried(true);
  }

  return ;
}

LoopStructure * LoopCarriedDependencies::getLoopOfLCD(StayConnectedNestedLoopForestNode *loopNode, const DominatorSummary &DS, DGEdge<Value> *edge) {

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
  auto producer = edge->getOutgoingT();
  auto consumer = edge->getIncomingT();

  /*
   * Only dependences between instructions can be loop-carried.
   */
  if (!isa<Instruction>(producer)) {
    return nullptr ;
  }
  if (!isa<Instruction>(consumer)) {
    return nullptr ;
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
   * If either of the instruction does not belong to a loop, then the dependence cannot be loop-carried.
   */
  if (!producerLoop || !consumerLoop) {
    return nullptr ;
  }

  if (producerI == consumerI || !DS.DT.dominates(producerI, consumerI)) {
    auto producerLevel = producerLoop->getNestingLevel();
    auto consumerLevel = consumerLoop->getNestingLevel();
    auto isMemoryDependenceThusCanCrossLoops = edge->isMemoryDependence();
    auto isControlDependence = edge->isControlDependence();

    /*
     * Check if the dependence is data and via variable.
     */
    if (!edge->isMemoryDependence() && edge->isDataDependence()) {

      /*
       * The data dependence is variable based
       *
       * If the producer cannot reach the header of the loop without reaching the consumer, then the dependence cannot be loop-carried.
       */
      auto producerB = producerI->getParent();
      auto consumerB = consumerI->getParent();
      auto mustProducerReachConsumerBeforeHeader = !canBasicBlockReachHeaderBeforeOther(*consumerLoop, producerB, consumerB);
      if (mustProducerReachConsumerBeforeHeader) {
        return nullptr ;
      }

      /*
       * The data dependence is variable based.
       * The producer can reach the header before reaching the consumer.
       * 
       * Check if the consumer will take the value from someone else when the execution comes from the header rather than the producer of the previous iteration
       */
      if (  true
            && DS.DT.dominates(consumerI, producerI)
            && DS.DT.dominates(topLoopHeaderBranch, consumerI)
        ){
        if (auto phiConsumer = dyn_cast<PHINode>(consumerI)){
          //errs() << "AAAA: producer = " << *producerI << "\n";
          //errs() << "AAAA: consumer = " << *consumerI << "\n";
          //errs() << "AAAA: Loop = " << *producerLoop->getHeader()->getFirstNonPHIOrDbg() << "\n\n";
          return nullptr;
        }
      }
    }

    return consumerLoop;
  }

  return nullptr ;
}

std::set<DGEdge<Value> *> LoopCarriedDependencies::getLoopCarriedDependenciesForLoop (
  const LoopStructure &LS, 
  StayConnectedNestedLoopForestNode *loopNode,
  PDG &LoopDG
  ) {
  
  std::set<DGEdge<Value> *> LCEdges;
  for (auto edge : LoopDG.getEdges()) {
    if (!edge->isLoopCarriedDependence()) { 
      continue; 
    }

    auto consumer = edge->getIncomingT();
    auto consumerI = cast<Instruction>(consumer);
    auto consumerLoop = loopNode->getInnermostLoopThatContains(consumerI);
    if (consumerLoop != &LS) {
      continue;
    }

    LCEdges.insert(edge);
  }

  return LCEdges;
}

std::set<DGEdge<Value> *> LoopCarriedDependencies::getLoopCarriedDependenciesForLoop (
  const LoopStructure &LS, 
  StayConnectedNestedLoopForestNode *loopNode,
  SCCDAG &sccdag
  ) {

  std::set<DGEdge<Value> *> LCEdges;

  for (auto sccNode : sccdag.getNodes()) {
    auto scc = sccNode->getT();
    for (auto edge : scc->getEdges()) {
      if (!edge->isLoopCarriedDependence()) {
        continue;
      }

      auto consumer = edge->getIncomingT();
      auto consumerI = cast<Instruction>(consumer);
      auto consumerLoop = loopNode->getInnermostLoopThatContains(consumerI);
      if (consumerLoop != &LS) {
        continue;
      }

      auto producer = edge->getOutgoingT();
      auto producerI = dyn_cast<Instruction>(producer);
      if(producerI == NULL) { continue; }

      auto producerLoop = loopNode->getInnermostLoopThatContains(producerI);
      if(!producerLoop) {continue;}
      LCEdges.insert(edge);
    }
  }
  return LCEdges;
}

bool LoopCarriedDependencies::canBasicBlockReachHeaderBeforeOther (
  const LoopStructure &LS,
  BasicBlock *I,
  BasicBlock *J
) {

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
    if (B == header) return true;
    if (exits.find(B) != exits.end()) continue;
    if (B == J) {
      isJReached = true;
      continue;
    }

    for (auto succIter = succ_begin(B); succIter != succ_end(B); ++succIter) {
      auto succ = *succIter;

      /*
       * Do not re-traverse enqueued blocks
       */
      if (enqueued.find(succ) != enqueued.end()) continue;
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

}
