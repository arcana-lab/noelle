#include "DSWP.hpp"

using namespace llvm;

void DSWP::partitionSCCDAG (DSWPLoopDependenceInfo *LDI) {

  /*
   * Print the current SCCDAG.
   */
  if (this->verbose) {
    errs() << "DSWP:  Before partitioning the SCCDAG\n";
    printSCCs(LDI->loopSCCDAG);
    errs() << "DSWP:    Number of nodes in the SCCDAG: " << LDI->loopSCCDAG->numNodes() << "\n";
  }

  /*
   * Check if we can cluster SCCs.
   */
  if (this->forceNoSCCPartition) {
    return ;
  }

  // WARNING: Uses LI to determine subloop information
  clusterSubloops(LDI);

  /*
   * Assign SCCs that have no partition to their own partitions.
   */
  for (auto nodePair : LDI->loopSCCDAG->internalNodePairs()) {

    /*
     * Check if the current SCC can be removed (e.g., because it is due to induction variables).
     * If it is, then this SCC has already been assigned to every dependent partition.
     */
    auto currentSCC = nodePair.first;
    if (LDI->partitions.isRemovable(currentSCC)) continue ;

    /*
     * Check if the current SCC has been already assigned to a partition; if not, assign it to a new partition.
     */
    if (LDI->partitions.partitionOf(currentSCC)) {
      LDI->partitions.addPartition(nodePair.first);
    }
  }

  /*
   * Print the partitioned SCCDAG.
   */
  if (this->verbose) {
    errs() << "DSWP:  After partitioning the SCCDAG\n";
    printSCCs(LDI->loopSCCDAG);
    errs() << "DSWP:    Number of nodes in the SCCDAG after obvious merging: " << LDI->loopSCCDAG->numNodes() << "\n";
  }

  return ;
}

void DSWP::mergeTrivialNodesInSCCDAG (DSWPLoopDependenceInfo *LDI) {

  /*
   * Print the current SCCDAG.
   */
  if (this->verbose) {
    errs() << "DSWP:  Before merging SCCs\n";
    printSCCs(LDI->loopSCCDAG);
  }

  /*
   * Merge SCCs.
   */
  mergePointerLoadInstructions(LDI);
  mergeSinglePHIs(LDI);
  mergeBranchesWithoutOutgoingEdges(LDI);

  /*
   * Print the current SCCDAG.
   */
  if (this->verbose) {
    errs() << "DSWP:  After merging SCCs\n";
    printSCCs(LDI->loopSCCDAG);
  }

  return ;
}

void DSWP::mergePointerLoadInstructions (DSWPLoopDependenceInfo *LDI) {
  while (true) {
    auto mergeNodes = false;
    for (auto sccEdge : LDI->loopSCCDAG->getEdges()) {
      auto fromSCCNode = sccEdge->getOutgoingNode();
      auto toSCCNode = sccEdge->getIncomingNode();
      for (auto instructionEdge : sccEdge->getSubEdges()) {
        auto producer = instructionEdge->getOutgoingT();
        bool isPointerLoad = isa<GetElementPtrInst>(producer);
        isPointerLoad |= (isa<LoadInst>(producer) && producer->getType()->isPointerTy());
        if (!isPointerLoad) continue;
        producer->print(errs() << "INSERTING INTO POINTER LOAD GROUP:\t"); errs() << "\n";
        mergeNodes = true;
      }

      if (mergeNodes) {
        std::set<DGNode<SCC> *> GEPGroup = { fromSCCNode, toSCCNode };
        LDI->loopSCCDAG->mergeSCCs(GEPGroup);
        break;
      }
    }
    if (!mergeNodes) break;
  }
}

void DSWP::mergeSinglePHIs (DSWPLoopDependenceInfo *LDI) {
  std::vector<std::set<DGNode<SCC> *>> singlePHIs;
  for (auto sccNode : LDI->loopSCCDAG->getNodes()) {
    auto scc = sccNode->getT();
    if (scc->numInternalNodes() > 1) continue;
    if (!isa<PHINode>(scc->begin_internal_node_map()->first)) continue;
    if (sccNode->numOutgoingEdges() == 1) {
      std::set<DGNode<SCC> *> nodes = { sccNode, (*sccNode->begin_outgoing_edges())->getIncomingNode() };
      singlePHIs.push_back(nodes);
    }
  }

  for (auto sccNodes : singlePHIs) LDI->loopSCCDAG->mergeSCCs(sccNodes);
}

void DSWP::clusterSubloops (DSWPLoopDependenceInfo *LDI) {
  auto &LI = getAnalysis<LoopInfoWrapperPass>(*LDI->function).getLoopInfo();
  auto loop = LI.getLoopFor(LDI->header);
  auto loopDepth = (int)LI.getLoopDepth(LDI->header);

  unordered_map<Loop *, std::set<SCC *>> loopSets;
  for (auto sccNode : LDI->loopSCCDAG->getNodes()) {
    for (auto iNodePair : sccNode->getT()->internalNodePairs()) {
      auto bb = cast<Instruction>(iNodePair.first)->getParent();
      auto loop = LI.getLoopFor(bb);
      auto subloopDepth = (int)loop->getLoopDepth();
      if (loopDepth >= subloopDepth) continue;

      if (loopDepth == subloopDepth - 1) loopSets[loop].insert(sccNode->getT());
      else {
        while (subloopDepth - 1 > loopDepth) {
          loop = loop->getParentLoop();
          subloopDepth--;
        }
        loopSets[loop].insert(sccNode->getT());
      }
      break;
    }
  }

  /*
   * WARNING: Should check if SCCs are already in a partition; if so, merge partitions
   */
  for (auto loopSetPair : loopSets) {
    LDI->partitions.addPartition(loopSetPair.second);
  }
}

void DSWP::mergeBranchesWithoutOutgoingEdges (DSWPLoopDependenceInfo *LDI) {
  std::vector<DGNode<SCC> *> tailCmpBrs;
  for (auto sccNode : LDI->loopSCCDAG->getNodes()) {
    auto scc = sccNode->getT();
    if (sccNode->numIncomingEdges() == 0 || sccNode->numOutgoingEdges() > 0) continue ;

    bool allCmpOrBr = true;
    for (auto node : scc->getNodes()) {
      allCmpOrBr &= (isa<TerminatorInst>(node->getT()) || isa<CmpInst>(node->getT()));
    }
    if (allCmpOrBr) tailCmpBrs.push_back(sccNode);
  }

  /*
   * Merge trailing compare/branch scc into previous depth scc
   */
  for (auto tailSCC : tailCmpBrs) {
    std::set<DGNode<SCC> *> nodesToMerge = { tailSCC };
    nodesToMerge.insert(*LDI->loopSCCDAG->previousDepthNodes(tailSCC).begin());
    LDI->loopSCCDAG->mergeSCCs(nodesToMerge);
  }
}
