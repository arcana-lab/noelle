#include "DSWP.hpp"

using namespace llvm;

void DSWP::partitionSCCDAG (DSWPLoopDependenceInfo *LDI, Heuristics *h) {

  /*
   * Initial the partition structure with the merged SCCDAG
   */
  LDI->partition.initialize(LDI->loopSCCDAG, &LDI->sccdagAttrs, &LDI->liSummary);

  /*
   * Check if we can cluster SCCs.
   */
  if (!this->forceNoSCCPartition) {
    clusterSubloops(LDI);
  }

  /*
   * Assign SCCs that have no partition to their own partitions.
   */
  for (auto nodePair : LDI->loopSCCDAG->internalNodePairs()) {

    /*
     * Check if the current SCC can be removed (e.g., because it is due to induction variables).
     * If it is, then this SCC has already been assigned to every dependent partition.
     */
    auto currentSCC = nodePair.first;
    if (LDI->partition.isRemovable(currentSCC)) continue ;

    /*
     * Check if the current SCC has been already assigned to a partition; if not, assign it to a new partition.
     */
    if (LDI->partition.subsetOf(currentSCC) == nullptr) {
      LDI->partition.addSubset(nodePair.first);
    }
  }

  /*
   * Print the initial partitions.
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DSWP:  Before partitioning the SCCDAG\n";
    printPartitions(LDI);
  }

  /*
   * Check if we can cluster SCCs.
   */
  if (!this->forceNoSCCPartition) {

    /*
     * Decide the partition of the SCCDAG by merging the trivial partitions defined above.
     */
    h->adjustParallelizationPartitionForDSWP(LDI->partition, LDI->sccdagAttrs, /*idealThreads=*/ 2);
  }

  /*
   * Print the partitioned SCCDAG.
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DSWP:  After partitioning the SCCDAG\n";
    printPartitions(LDI);
  }

  return ;
}

void DSWP::mergeSingleSyntacticSugarInstrs (DSWPLoopDependenceInfo *LDI) {
  std::unordered_map<DGNode<SCC> *, std::set<DGNode<SCC> *> *> mergedToGroup;
  std::set<std::set<DGNode<SCC> *> *> singles;
  for (auto sccNode : LDI->loopSCCDAG->getNodes()) {
    auto scc = sccNode->getT();

    /*
     * Determine if node is a single syntactic sugar instruction with only one dependent SCC
     */
    if (scc->numInternalNodes() > 1) continue;
    auto I = scc->begin_internal_node_map()->first;
    if (!isa<PHINode>(I) && !isa<GetElementPtrInst>(I) && !isa<CastInst>(I)) continue;
    if (sccNode->numOutgoingEdges() != 1) continue;
    auto dependentNode = (*sccNode->begin_outgoing_edges())->getIncomingNode();

    /*
     * Determine the merged state of the single instruction node and its dependent
     * Merge the current merged nodes holding both of the above
     */
    bool mergedSCCNode = mergedToGroup.find(sccNode) != mergedToGroup.end();
    bool mergedDepNode = mergedToGroup.find(dependentNode) != mergedToGroup.end();
    if (mergedSCCNode && mergedDepNode) {

      /*
       * Combine the dependent node's merged group into that of the single instruction's merged group
       */
      auto depSet = mergedToGroup[dependentNode];
      for (auto node : *depSet) {
        mergedToGroup[sccNode]->insert(node);
        mergedToGroup[node] = mergedToGroup[sccNode];
      }
      singles.erase(depSet);
      delete depSet;
    } else if (mergedSCCNode) {
      mergedToGroup[sccNode]->insert(dependentNode);
      mergedToGroup[dependentNode] = mergedToGroup[sccNode];
    } else if (mergedDepNode) {
      mergedToGroup[dependentNode]->insert(sccNode);
      mergedToGroup[sccNode] = mergedToGroup[dependentNode];
    } else {
      auto nodes = new std::set<DGNode<SCC> *>({ sccNode, dependentNode });
      singles.insert(nodes);
      mergedToGroup[sccNode] = nodes;
      mergedToGroup[dependentNode] = nodes;
    }
  }

  for (auto sccNodes : singles) { 
    LDI->loopSCCDAG->mergeSCCs(*sccNodes);
    delete sccNodes;
  }
}

void DSWP::clusterSubloops (DSWPLoopDependenceInfo *LDI) {
  auto &li = LDI->liSummary;
  auto loop = li.bbToLoop[LDI->header];
  auto loopDepth = loop->depth;

  unordered_map<LoopSummary *, std::set<SCC *>> loopSets;
  for (auto sccNode : LDI->loopSCCDAG->getNodes()) {
    if (LDI->partition.isRemovable(sccNode->getT())) continue;

    for (auto iNodePair : sccNode->getT()->internalNodePairs()) {
      auto bb = cast<Instruction>(iNodePair.first)->getParent();
      auto subL = li.bbToLoop[bb];
      auto subDepth = subL->depth;
      if (loopDepth >= subDepth) continue;

      while (subDepth - 1 > loopDepth) {
        subL = subL->parent;
        subDepth--;
      }
      loopSets[subL].insert(sccNode->getT());
      break;
    }
  }

  /*
   * Basic Heuristic: partition entire sub loops only if there is more than one
   */
  if (loopSets.size() == 1) return;
  for (auto loopSetPair : loopSets) {
    LDI->partition.addSubset(loopSetPair.second);
  }
}

void DSWP::addRemovableSCCsToStages (DSWPLoopDependenceInfo *LDI) {
  for (auto &stage : LDI->stages) {
    std::set<DGNode<SCC> *> visitedNodes;
    std::queue<DGNode<SCC> *> dependentSCCNodes;

    for (auto scc : stage->stageSCCs) {
      dependentSCCNodes.push(LDI->loopSCCDAG->fetchNode(scc));
    }

    while (!dependentSCCNodes.empty()) {
      auto depSCCNode = dependentSCCNodes.front();
      dependentSCCNodes.pop();

      for (auto sccEdge : depSCCNode->getIncomingEdges()) {
        auto fromSCCNode = sccEdge->getOutgoingNode();
        auto fromSCC = fromSCCNode->getT();
        if (visitedNodes.find(fromSCCNode) != visitedNodes.end()) continue;
        if (!LDI->partition.isRemovable(fromSCC)) continue;

        stage->removableSCCs.insert(fromSCC);
        dependentSCCNodes.push(fromSCCNode);
        visitedNodes.insert(fromSCCNode);
      }
    }
  }
}
