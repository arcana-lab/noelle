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

uint64_t InvocationLatency::latencyPerInvocation (std::set<std::set<SCC *> *> &subsets){
  uint64_t maxInternalCost = 0;
  std::set<Value *> queueValues;
  for (auto sccs : subsets) {
    for (auto scc : *sccs) {

      /*
       * Collect scc internal information 
       */
      auto internalCost = this->latencyPerInvocation(scc);
      if (internalCost > maxInternalCost) maxInternalCost = internalCost;

      /*
       * Collect scc external cost (through edges)
       */
      if (sccToExternals.find(scc) == sccToExternals.end()) {
        for (auto &valueNodePair : scc->externalNodePairs()) {
          sccToExternals[scc].insert(valueNodePair.first);
        }
      }
      queueValues.insert(sccToExternals[scc].begin(), sccToExternals[scc].end());
    }
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
  } else if (isa<TerminatorInst>(inst)) {
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
