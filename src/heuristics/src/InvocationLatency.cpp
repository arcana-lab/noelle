/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "InvocationLatency.hpp"

using namespace llvm;
 
InvocationLatency::InvocationLatency (Hot *hot)
  : profiles{hot}
  {
  return ;
}

uint64_t InvocationLatency::latencyPerInvocation (SCC *scc){

  /*
   * Check if we have already computed the latency of this SCC.
   */
  if (this->sccToCost.find(scc) != this->sccToCost.end()) {
    return this->sccToCost[scc];
  }

  /*
   * Compute the latency of the SCC.
   */
  auto cost = this->profiles->getTotalInstructions(scc);
  sccToCost[scc] = cost;

  return cost;
}

/*
 * The execution time of all subsets is approx:
 *  1) The maximum internal execution of any subset +
 *  2) The time spent en/de-queueing for all subsets
 */
uint64_t InvocationLatency::latencyPerInvocation (
  SCCDAGAttrs *attrs,
  std::unordered_set<SCCSet *> &sets
) {
  uint64_t maxInternalCost = 0;
  std::set<Value *> queueValues;
  for (auto set : sets) {
    auto &sccs = set->sccs;
    std::set<SCC *> subsetSCCs(sccs.begin(), sccs.end());
    for (auto scc : sccs) {
      auto &parents = memoizeParents(attrs, scc);
      subsetSCCs.insert(parents.begin(), parents.end());
    }

    uint64_t internalCost = 0;
    for (auto scc : subsetSCCs) {
      auto &externals = memoizeExternals(attrs, scc);
      queueValues.insert(externals.begin(), externals.end());
      internalCost += this->latencyPerInvocation(scc);
    }
    if (internalCost > maxInternalCost) maxInternalCost = internalCost;
  }

  uint64_t cost = maxInternalCost;
  for (auto queueVal : queueValues) {
    cost += this->queueLatency(queueVal);
  }
  return cost;
}

uint64_t InvocationLatency::latencyPerInvocation (Instruction *inst){

  /*
   * Identify 0 latency instructions
   */
  auto isSyntacticSugar = [&](Instruction *I) -> bool {
    return isa<PHINode>(I) || isa<GetElementPtrInst>(I) || isa<CastInst>(I);
  };
  if (isSyntacticSugar(inst)) {
    return 0;
  }

  /*
   * Estimate the latency.
   */
  auto latency = this->profiles->getTotalInstructions(inst);

  return latency;
}

uint64_t InvocationLatency::queueLatency (Value *queueVal){
  // TODO(angelo): use primitive size of bits of type of value?
  return 100;
}

/*
 * Retrieve or memoize all values the SCC is dependent on.
 * This does NOT include values within clonable parents as they will be present during execution (because they are cloned).
 */
std::set<Value *> &InvocationLatency::memoizeExternals (SCCDAGAttrs *attrs, SCC *scc) {
  auto externalsIter = incomingExternals.find(scc);
  if (externalsIter != incomingExternals.end()) {
    return externalsIter->second;
  }

  for (auto edge : attrs->edgesViaClones[scc]) {
    auto parent = edge->getIncomingT();
    auto parentInfo = attrs->getSCCAttrs(parent);
    if (parentInfo->canBeCloned()) {
      continue;
    }

    for (auto subEdge : edge->getSubEdges()) {
      incomingExternals[scc].insert(subEdge->getIncomingT());
    }
  }
  return incomingExternals[scc];
}

/*
 * Retrieve or memoize all parents of this SCC that are clonable
 */
std::set<SCC *> &InvocationLatency::memoizeParents (SCCDAGAttrs *attrs, SCC *scc) {
  auto parentsIter = clonableParents.find(scc);
  if (parentsIter != clonableParents.end()) return parentsIter->second;
  for (auto parent : attrs->parentsViaClones[scc]) {
    auto parentInfo = attrs->getSCCAttrs(parent);
    if (parentInfo->canBeCloned()) {
      clonableParents[scc].insert(parent);
    }
  }
  return clonableParents[scc];
}
