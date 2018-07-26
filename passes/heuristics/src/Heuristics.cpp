#include "../include/Heuristics.hpp"

using namespace llvm;

uint64_t Heuristics::latencyPerInvocation (SCC *scc){
  return 0;
}

uint64_t Heuristics::latencyPerInvocation (Instruction *inst){

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
  auto latency = 1;

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
      calleeLatency = 10;
    }

    /*
     * Add the latency of the callee.
     */
    latency += calleeLatency;
  }

  return latency;
}
