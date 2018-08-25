#include "../include/Heuristics.hpp"

using namespace llvm;

uint64_t Heuristics::latencyPerInvocation (SCC *scc){
  uint64_t cost = 0;
  for (auto nodePair : scc->internalNodePairs()) {
    auto I = cast<Instruction>(nodePair.first);
    cost += this->latencyPerInvocation(I);
  }
  return cost;
}

uint64_t Heuristics::latencyPerInvocation (SCCDAGAttrs &sccdagAttrs, std::set<SCC *> &sccs){
  int cost = 0;
  for (auto scc : sccs) {
    auto &sccInfo = sccdagAttrs.getSCCAttrs(scc);

    /*
     * Collect scc internal information 
     */
    cost += sccInfo->internalCost;

    /*
     * Collect scc external cost (through edges)
     */
    std::set<Value *> incomingEdges;
    for (auto &sccEdgesPair : sccInfo->sccToEdgeInfo) {
      if (sccs.find(sccEdgesPair.first) != sccs.end()) continue;
      auto &edges = sccEdgesPair.second->edges;
      incomingEdges.insert(edges.begin(), edges.end());
    }

    for (auto edgeVal : incomingEdges) {
      cost += this->queueLatency(edgeVal);
    }
  }
  return cost;
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

void Heuristics::adjustParallelizationPartitionForDSWP (SCCDAGPartition &partition, SCCDAGAttrs &sccdagAttrs, uint64_t idealThreads){

  /*
   * Estimate the current latency for each subset of the current partition of the SCCDAG.
   */
  uint64_t totalCost = 0;
  std::set<SCCDAGSubset *> currentSubsets;
  for (auto &subset : partition.subsets) {
    currentSubsets.insert(subset.get());
    subset->cost = this->latencyPerInvocation(sccdagAttrs, subset->SCCs);
    totalCost += subset->cost;
  }

  /*
   * Collect all subsets of the current SCCDAG partition.
   */
  std::queue<SCCDAGSubset *> partToCheck;
  auto topLevelParts = partition.topLevelSubsets();
  for (auto part : topLevelParts) {
    partToCheck.push(part);
  }

  /*
   * Merge subsets.
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
    if (currentSubsets.find(subset) == currentSubsets.end()) {
      continue;
    }
    // subset->print(errs() << "DSWP:   CHECKING SUBSET:\n", "DSWP:   ");

    /*
     * Prioritize merge that best lowers overall cost without yielding a too costly partition
     */
    SCCDAGSubset *minSubset = nullptr;
    int32_t maxLoweredCost = 0;
    auto maxAllowedCost = totalCost / idealThreads;

    auto tryToMergeWith = [&](SCCDAGSubset *s) -> void {
      if (!partition.canMergeSubsets(subset, s)) { 
        //errs() << "DSWP:   CANNOT MERGE\n";
        return;
      }
      // part->print(errs() << "DSWP:   CAN MERGE WITH PARTITION:\n", "DSWP:   ");

      /*
       * Create an example merge of the subsets to determine its worth
       */
      auto demoMerged = partition.demoMergeSubsets(subset, s);
      auto mergedCost = this->latencyPerInvocation(sccdagAttrs, demoMerged->SCCs);
      if (mergedCost > maxAllowedCost) return ;
      // errs() << "DSWP:   Max allowed cost: " << maxAllowedCost << "\n";

      auto loweredCost = s->cost + subset->cost - mergedCost;
      // errs() << "DSWP:   Merging (cost " << subset->cost << ", " << part->cost << ") yields cost " << demoMerged->cost << "\n";
      if (loweredCost > maxLoweredCost) {
        // errs() << "DSWP:   WILL MERGE IF BEST\n";
        minSubset = s;
        maxLoweredCost = loweredCost;
      }
    };

    /*
     * Check merge criteria on dependents and depth-1 neighbors
     */
    auto dependents = partition.getDependents(subset);
    auto siblings = partition.getSiblings(subset);
    for (auto s : dependents) tryToMergeWith(s);
    for (auto s : siblings) tryToMergeWith(s);

    /*
     * Merge partition if one is found; reiterate the merge check on it
     */
    if (minSubset) {
      auto mergedSub = partition.mergeSubsets(subset, minSubset);
      totalCost -= maxLoweredCost;
      currentSubsets.erase(subset);
      currentSubsets.erase(minSubset);
      currentSubsets.insert(mergedSub);
      partToCheck.push(mergedSub);
      //mergedSub->print(errs() << "DSWP:   MERGED PART: " << partToCheck.size() << "\n", "DSWP:   ");
    }

    /*
     * Iterate the merge check on all dependent partitions
     */
    for (auto s : dependents) {
      if (minSubset == s) continue;
      partToCheck.push(s);
      //s->print(errs() << "DSWP:   WILL CHECK: " << partToCheck.size() << "\n", "DSWP:   ");
    }
  }

  return ;
}
