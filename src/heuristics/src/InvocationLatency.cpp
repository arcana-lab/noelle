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

uint64_t InvocationLatency::latencyPerInvocation (SCC *scc){
  if (sccToCost.find(scc) != sccToCost.end()) {
    return sccToCost[scc];
  }

  uint64_t cost = 0;
  for (auto nodePair : scc->internalNodePairs()) {
    auto I = cast<Instruction>(nodePair.first);
    cost += this->latencyPerInvocation(I);
  }

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
  std::set<std::set<SCC *> *> &subsets
) {
  uint64_t maxInternalCost = 0;
  std::set<Value *> queueValues;
  std::set<SCC *> allSCCs;
  for (auto sccs : subsets) {
    std::set<SCC *> subsetSCCs(sccs->begin(), sccs->end());
    for (auto scc : *sccs) {
      auto &parents = memoizeParents(attrs, scc);
      subsetSCCs.insert(parents.begin(), parents.end());
    }
    allSCCs.insert(subsetSCCs.begin(), subsetSCCs.end());

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
   * Estimate the latency of the instruction.
   */
  uint64_t latency;
  if (isa<StoreInst>(inst) || isa<LoadInst>(inst)) {
    latency = 10;
  } else if (inst->isTerminator()) {
    latency = 5;
  } else {
    latency = 1;
  }

  /*
   * Handle call instructions.
   * For them, we have to add the estimate of the latency of the callee.
   */
  if (auto call = dyn_cast<CallInst>(inst)) {

    /*
     * Check if we know the callee.
     */
    auto F = call->getCalledFunction();
    auto calleeLatency = 0;
    if (  (F != nullptr)  &&
          (!F->empty())   ){

      /*
       * Compute the latency of the callee without checking its callees recursively.
       *
       * Check if we have already computed it.
       */
      if (funcToCost.find(F) == funcToCost.end()) {

        /*
         * Compute the latency.
         */
        for (auto &B : *F) {
          for (auto &J : B) {
            if (!isSyntacticSugar(&J)) {
              calleeLatency++;
            }
          }
        }
        funcToCost[F] = calleeLatency;

      } else {

        /*
         * Fetch the latency we have computed it during past invocations.
         */
        calleeLatency = funcToCost[F];
      }

    } else {
      calleeLatency = 50;
    }

    /*
     * Add the latency of the callee.
     */
    latency += calleeLatency;
  }

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
