/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "HELIX.hpp"
#include "llvm/IR/CFG.h"

using namespace llvm ;


/*
 * Heuristic used: push furthest outlier instructions closer to the rest of the sequential segment
 * by moving between control flow equivalent sets of basic blocks
 */
void HELIX::squeezeSequentialSegment (
  LoopDependenceInfo *LDI,
  DataFlowResult *reachabilityDFR,
  SequentialSegment *ss
  ){

  /*
   * Fetch ControlFlowEquivalence and dependence graph
   * TODO: Move this to LDI
   */
  auto loops = LDI->getLoopHierarchyStructures();
  auto rootLoop = loops.getLoopNestingTreeRoot();
  auto taskFunction = rootLoop->getHeader()->getParent();
  auto taskDG = LDI->getLoopDG();
  DominatorTree taskDT(*taskFunction);
  PostDominatorTree taskPDT(*taskFunction);
  DominatorSummary taskDS(taskDT, taskPDT);
  ControlFlowEquivalence cfe(&taskDS, &loops, rootLoop);

  /*
   * Consider all un-moved instructions in the working queue
   */
  auto ssInstructions = ss->getInstructions();
  std::queue<Instruction *> instructionsToMove{};
  for (auto I : ssInstructions) {
    instructionsToMove.push(I);
  }

  while (!instructionsToMove.empty()) {
    auto I = instructionsToMove.front();
    instructionsToMove.pop();

    /*
     * Determine whether the instruction has more produced or consumed dependencies from its SS
     * If it has more produced, push it towards the end of the loop's iteration
     * If it has more consumed, push it towards the beginning of the loop's iteration
     */
    std::unordered_set<Instruction *> producersOfI{};
    std::unordered_set<Instruction *> consumersOfI{};
    auto ssProducers = 0, ssConsumers = 0;
    auto nodeI = taskDG->fetchNode(I);
    for (auto edgeProducedByI : nodeI->getOutgoingEdges()) {
      auto consumerValue = edgeProducedByI->getIncomingT();
      auto consumerInst = cast<Instruction>(consumerValue);
      consumersOfI.insert(consumerInst);

      if (ssInstructions.find(consumerInst) == ssInstructions.end()) continue;
      ssConsumers++;
    }
    for (auto edgeConsumedByI : nodeI->getIncomingEdges()) {
      auto producerValue = edgeConsumedByI->getIncomingT();
      auto producerInst = cast<Instruction>(producerValue);
      producersOfI.insert(producerInst);

      if (ssInstructions.find(producerInst) == ssInstructions.end()) continue;
      ssProducers++;
    }


    /*
     * For each instruction to go towards, traverse CFE basic blocks
     */
    bool isMovingTowardsProducers = ssProducers >= ssConsumers;
    bool isInsertingAfterInsts = isMovingTowardsProducers;
    auto instsToPushTowards = isMovingTowardsProducers ? producersOfI : consumersOfI;
    // TODO:

  }

}

void HELIX::squeezeSequentialSegments (
  LoopDependenceInfo *LDI,
  std::vector<SequentialSegment *> *sss
  ){

  // TODO:
  return;

  /*
   * Compute reachability across a single iteration of the loop
   */
  auto reachabilityDFR = this->computeReachabilityFromInstructions(LDI);

  /*
   * Squeeze all sequential segments.
   * NOTE: Reachability does NOT need to be re-computed after each squeezing
   * as a sequential segment does not care about SS's instructions
   */
  for (auto ss : *sss){
    this->squeezeSequentialSegment(LDI, reachabilityDFR, ss);
  }

  return ;
}

void HELIX::scheduleSequentialSegments (
  LoopDependenceInfo *LDI,
  std::vector<SequentialSegment *> *sss
  ){
  //TODO

  return ;
}
