/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopCarriedDependencies.hpp"

using namespace llvm;

LoopCarriedDependencies::LoopCarriedDependencies (
  const LoopsSummary &LIS,
  const DominatorSummary &DS,
  SCCDAG &sccdagForLoops
) {

  for (auto &loop : LIS.loops) {
    loopCarriedDependenciesMap[loop.get()] = Criticisms();
  }

  for (auto sccNode : sccdagForLoops.getNodes()) {
    auto scc = sccNode->getT();
    for (auto edge : scc->getEdges()) {
      auto producer = edge->getOutgoingT();
      auto consumer = edge->getIncomingT();
      if (!isa<Instruction>(producer)) continue ;
      if (!isa<Instruction>(consumer)) continue ;

      auto producerI = dyn_cast<Instruction>(producer);
      auto consumerI = dyn_cast<Instruction>(consumer);
      auto producerLoop = LIS.getLoop(*producerI);
      auto consumerLoop = LIS.getLoop(*consumerI);
      if (!producerLoop || !consumerLoop) continue;

      if (producerI == consumerI || !DS.DT.dominates(producerI, consumerI)) {
        auto producerLevel = producerLoop->getNestingLevel();
        auto consumerLevel = consumerLoop->getNestingLevel();
        assert(producerLevel >= consumerLevel
          && "Producer of loop carried dependency is NOT in the same loop or in an inner-more loop than the consumer");
        loopCarriedDependenciesMap[consumerLoop].insert(edge);
      }
    }
  }

}

Criticisms LoopCarriedDependencies::getLoopCarriedDependenciesForLoop (LoopSummary &LS) {
  assert(loopCarriedDependenciesMap.find(&LS) != loopCarriedDependenciesMap.end());
  return loopCarriedDependenciesMap.at(&LS);
}