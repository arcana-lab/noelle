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

uint64_t Heuristics::latencyPerInvocation (SCCDAGAttrs &sccdagAttrs, std::set<std::set<SCC *> *> &subsets){
  uint64_t maxInternalCost = 0;
  std::set<Value *> queueValues;
  for (auto sccs : subsets) {
    for (auto scc : *sccs) {
      auto &sccInfo = sccdagAttrs.getSCCAttrs(scc);

      /*
       * Collect scc internal information 
       */
      auto internalCost = sccInfo->internalCost;
      if (internalCost > maxInternalCost) maxInternalCost = sccInfo->internalCost;

      /*
       * Collect scc external cost (through edges)
       */
      for (auto &sccEdgesPair : sccInfo->sccToEdgeInfo) {
        if (sccs->find(sccEdgesPair.first) != sccs->end()) continue;
        auto &edges = sccEdgesPair.second->edges;
        queueValues.insert(edges.begin(), edges.end());
      }
    }
  }

  uint64_t cost = maxInternalCost;
  for (auto queueVal : queueValues) {
    cost += this->queueLatency(queueVal);
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
    uint64_t totalInstCount = 0;
    std::unordered_map<int, uint64_t> subsetIDToCost;
    std::unordered_map<int, uint64_t> subsetIDToInstCount;
    for (auto &subset : partition.subsets) {
      auto subsetID = partition.getSubsetID(subset);

      uint64_t instCount = 0;
      for (auto scc : subset->SCCs) instCount += scc->numInternalNodes();
      std::set<std::set<SCC *> *> subsets = { &subset->SCCs };
      uint64_t cost = this->latencyPerInvocation(sccdagAttrs, subsets);

      subsetIDToInstCount[subsetID] = instCount;
      subsetIDToCost[subsetID] = cost;

      totalInstCount += instCount;
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
     * Prioritize merge that best lowers overall cost without yielding a too costly partition
     */
    int minSubsetAID = -1;
    int minSubsetBID = -1;
    uint64_t maxLoweredCost = 0;
    uint64_t minInstCount = totalInstCount;
    uint64_t chosenMergeCost = 0;
    auto checkIfShouldMerge = [&](int sA, int sB) -> void {
      // errs() << "Checking to see if can merge " << sA << " with " << sB << "\n";
      if (!partition.canMergeSubsets(sA, sB)) return ;
      // errs() << "Trying to merge " << sA << " with " << sB << "\n";

      /*
       * Determine cost of merge
       */
      auto currentCost = subsetIDToCost[sA] + subsetIDToCost[sB];
      auto instCount = subsetIDToInstCount[sA] + subsetIDToInstCount[sB];
      std::set<std::set<SCC *> *> subsets = {
        &(partition.subsetOfID(sA)->SCCs),
        &(partition.subsetOfID(sB)->SCCs)
      };
      uint64_t mergeCost = this->latencyPerInvocation(sccdagAttrs, subsets);
      uint64_t loweredCost = currentCost - mergeCost;

      /*
       * Only merge if it doesn't yield an SCC costing more than half the total cost
       * TODO(angelo): Determine fractional limit based on number of cores available
       */
      if (mergeCost > totalCost / 1 || partition.subsets.size() == 2) return ;

      /*
       * Only merge if it best lowers cost
       */
      if (loweredCost < maxLoweredCost) return ;

      /*
       * Only merge if it is the smallest of equally cost effective merges
       */
      if (loweredCost == maxLoweredCost && minInstCount < instCount) return ;

      // errs() << "\twill lower merge by " << loweredCost << "\n";
      minSubsetAID = sA;
      minSubsetBID = sB;
      maxLoweredCost = loweredCost;
      minInstCount = instCount;
      chosenMergeCost = mergeCost;
    };

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
      if (!partition.isValidSubset(subID)) continue ;

      // errs() << "\nTraversing " << subID << "\n";

      /*
       * Check merge criteria on dependents and depth-1 neighbors
       */
      auto dependentIDs = partition.getDependentIDs(subID);
      auto siblingIDs = partition.getSiblingIDs(subID);
      for (auto s : dependentIDs) checkIfShouldMerge(subID, s);
      for (auto s : siblingIDs) checkIfShouldMerge(subID, s);

      /*
       * Add dependent SCCs as well.
       */
      for (auto s : dependentIDs) {
        if (alreadyChecked.find(s) == alreadyChecked.end()){
          subIDToCheck.push(s);
          alreadyChecked.insert(s);
        }
      }
    }

    /*
     * Merge partition if one is found; reiterate the merge check on it
     */
    if (minSubsetAID != -1) {
      // errs() << "Merging " << minSubsetAID << " with " << minSubsetBID << "\n";
      auto mergedSubID = partition.mergeSubsets(minSubsetAID, minSubsetBID);
      modified = true;
      subIDToCheck.push(mergedSubID);
      alreadyChecked.insert(mergedSubID);

      /*
       * Readjust subset cost tracking
       */
      subsetIDToCost[mergedSubID] = chosenMergeCost;
      subsetIDToInstCount[mergedSubID] = minInstCount;
      totalCost -= maxLoweredCost;
    }

  } while (modified);

  return ;
}
