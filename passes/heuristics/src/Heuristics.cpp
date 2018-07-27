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

uint64_t Heuristics::queueLatency (Value *queueVal){
  // TODO(angelo): use primitive size of bits of type of value?
  return 100;
}

void Heuristics::adjustParallelizationPartitionForDSWP (SCCDAGPartition &partition){

  /*
   * Collect all top level partitions
   */
  std::queue<SCCDAGSubset *> partToCheck;
  auto topLevelParts = partition.topLevelSubsets();
  for (auto part : topLevelParts) {
    partToCheck.push(part);
  }

  /*
   * Merge partitions.
   */
  while (!partToCheck.empty()) {

    /*
     * Fetch the current subset.
     */
    auto subset = partToCheck.front();
    partToCheck.pop();

    /*
     * Check if the current subset has been already tagged to be removed (i.e., merged).
     */
    if (!partition.isValidSubset(subset)) {
      continue;
    }
    // subset->print(errs() << "DSWP:   CHECKING SUBSET:\n", "DSWP:   ");

    /*
     * Prioritize merge that best lowers overall cost without yielding a too costly partition
     */
    SCCDAGSubset *minSubset = nullptr;
    int32_t maxLoweredCost = 0;
    auto maxAllowedCost = partition.maxSubsetCost();

    auto checkMergeWith = [&](SCCDAGSubset *part) -> void {
      if (!partition.canMergeSubsets(subset, part)) { errs() << "DSWP:   CANNOT MERGE\n"; return; }
      // part->print(errs() << "DSWP:   CAN MERGE WITH PARTITION:\n", "DSWP:   ");

      auto demoMerged = partition.demoMergeSubsets(subset, part);
      if (demoMerged->cost > maxAllowedCost) return ;
      // errs() << "DSWP:   Max allowed cost: " << maxAllowedCost << "\n";

      auto loweredCost = part->cost + subset->cost - demoMerged->cost;
      // errs() << "DSWP:   Merging (cost " << subset->cost << ", " << part->cost << ") yields cost " << demoMerged->cost << "\n";
      if (loweredCost > maxLoweredCost) {
        // errs() << "DSWP:   WILL MERGE IF BEST\n";
        minSubset = part;
        maxLoweredCost = loweredCost;
      }
    };

    /*
     * Check merge criteria on dependents and depth-1 neighbors
     */
    auto dependents = partition.getDependents(subset);
    auto cousins = partition.getCousins(subset);
    for (auto part : dependents) checkMergeWith(part);
    for (auto part : cousins) checkMergeWith(part);

    /*
     * Merge partition if one is found; reiterate the merge check on it
     */
    if (minSubset) {
      auto mergedPart = partition.mergeSubsets(subset, minSubset);
      partToCheck.push(mergedPart);
      mergedPart->print(errs() << "DSWP:   MERGED PART: " << partToCheck.size() << "\n", "DSWP:   ");
    }

    /*
     * Iterate the merge check on all dependent partitions
     */
    for (auto part : dependents) {
      if (minSubset == part) continue;
      partToCheck.push(part);
      part->print(errs() << "DSWP:   WILL CHECK: " << partToCheck.size() << "\n", "DSWP:   ");
    }
  }

  return ;
}
