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
   * Merge subsets.
   */
  auto modified = false;
  do {
    modified = false;

    /*
     * Estimate the current latency for traversing once the pipeline created by the current partition of the SCCDAG.
     */
    uint64_t totalCost = 0;
    uint64_t maxAllowedCost = 0;
    std::unordered_map<int, uint64_t> subsetIDToCost;
    for (auto &subset : partition.subsets) {
      auto subsetID = partition.getSubsetID(subset);
      auto cost = this->latencyPerInvocation(sccdagAttrs, subset->SCCs);
      subsetIDToCost[subsetID] = cost;
      if (maxAllowedCost == 0 || cost > maxAllowedCost) {
        maxAllowedCost = cost;
      }

      totalCost += cost;
    }

    /*
     * Collect all subsets of the current SCCDAG partition.
     */
    std::queue<int> subIDToCheck;
    std::set<int> alreadyChecked;
    auto topLevelSubIDs = partition.getSubsetIDsWithNoIncomingEdges();
    for (auto subID : topLevelSubIDs) {
      subIDToCheck.push(subID);
      alreadyChecked.insert(subID);
    }

    /*
     * Merge subsets.
     */
    while (!subIDToCheck.empty()) {

      /*
       * Fetch the current subset.
       */
      auto subID = subIDToCheck.front();
      subIDToCheck.pop();

      /*
       * Check if the current subset has been already tagged to be removed (i.e., merged).
       */
      if (partition.isValidSubset(subID)) continue ;

      /*
       * Prioritize merge that best lowers overall cost without yielding a too costly partition
       */
      int minSubsetID = -1;
      int32_t maxLoweredCost = 0;
      auto tryToMergeWith = [&](int s) -> void {
        if (!partition.canMergeSubsets(subID, s)) return ;

        /*
         * Create an example merge of the subsets to determine its worth
         */
        auto sccsOfSubsets = partition.sccsOfSubsets(subID, s);
        auto mergedCost = this->latencyPerInvocation(sccdagAttrs, sccsOfSubsets);
        if (mergedCost > maxAllowedCost || mergedCost < maxLoweredCost) return ;

        minSubsetID = s;
      };

      /*
       * Check merge criteria on dependents and depth-1 neighbors
       */
      auto dependentIDs = partition.getDependentIDs(subID);
      auto siblingIDs = partition.getSiblingIDs(subID);
      for (auto s : dependentIDs) tryToMergeWith(s);
      for (auto s : siblingIDs) tryToMergeWith(s);

      /*
       * Merge partition if one is found; reiterate the merge check on it
       */
      if (minSubsetID != -1) {
        auto mergedSubID = partition.mergeSubsets(subID, minSubsetID);
        subIDToCheck.push(mergedSubID);
        alreadyChecked.insert(mergedSubID);

        /*
         * Add dependent SCCs as well.
         */
        for (auto s : dependentIDs) {
          if (alreadyChecked.find(s) == alreadyChecked.end()){
            subIDToCheck.push(s);
            alreadyChecked.insert(s);
          }
        }

        modified = true;
      }
    }
  } while (modified);

  return ;
}
