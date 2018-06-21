#include "DSWP.hpp"

using namespace llvm;

void DSWP::mergePointerLoadInstructions (DSWPLoopDependenceInfo *LDI)
{
  while (true)
  {
    bool mergeNodes = false;
    for (auto sccEdge : LDI->loopSCCDAG->getEdges())
    {
      auto fromSCCNode = sccEdge->getOutgoingNode();
      auto toSCCNode = sccEdge->getIncomingNode();
      for (auto instructionEdge : sccEdge->getSubEdges())
      {
        auto producer = instructionEdge->getOutgoingT();
        bool isPointerLoad = isa<GetElementPtrInst>(producer);
        isPointerLoad |= (isa<LoadInst>(producer) && producer->getType()->isPointerTy());
        if (!isPointerLoad) continue;
        producer->print(errs() << "INSERTING INTO POINTER LOAD GROUP:\t"); errs() << "\n";
        mergeNodes = true;
      }

      if (mergeNodes)
      {
        std::set<DGNode<SCC> *> GEPGroup = { fromSCCNode, toSCCNode };
        LDI->loopSCCDAG->mergeSCCs(GEPGroup);
        break;
      }
    }
    if (!mergeNodes) break;
  }
}

void DSWP::mergeSinglePHIs (DSWPLoopDependenceInfo *LDI)
{
  std::vector<std::set<DGNode<SCC> *>> singlePHIs;
  for (auto sccNode : LDI->loopSCCDAG->getNodes())
  {
    auto scc = sccNode->getT();
    if (scc->numInternalNodes() > 1) continue;
    if (!isa<PHINode>(scc->begin_internal_node_map()->first)) continue;
    if (sccNode->numOutgoingEdges() == 1)
    {
      std::set<DGNode<SCC> *> nodes = { sccNode, (*sccNode->begin_outgoing_edges())->getIncomingNode() };
      singlePHIs.push_back(nodes);
    }
  }

  for (auto sccNodes : singlePHIs) LDI->loopSCCDAG->mergeSCCs(sccNodes);
}

void DSWP::clusterSubloops (DSWPLoopDependenceInfo *LDI)
{
  auto &LI = getAnalysis<LoopInfoWrapperPass>(*LDI->function).getLoopInfo();
  auto loop = LI.getLoopFor(LDI->header);
  auto loopDepth = (int)LI.getLoopDepth(LDI->header);

  unordered_map<Loop *, std::set<DGNode<SCC> *>> loopSets;
  for (auto sccNode : LDI->loopSCCDAG->getNodes())
  {
    for (auto iNodePair : sccNode->getT()->internalNodePairs())
    {
      auto bb = cast<Instruction>(iNodePair.first)->getParent();
      auto loop = LI.getLoopFor(bb);
      auto subloopDepth = (int)loop->getLoopDepth();
      if (loopDepth >= subloopDepth) continue;

      if (loopDepth == subloopDepth - 1) loopSets[loop].insert(sccNode);
      else
      {
        while (subloopDepth - 1 > loopDepth)
        {
          loop = loop->getParentLoop();
          subloopDepth--;
        }
        loopSets[loop].insert(sccNode);
      }
      break;
    }
  }

  for (auto loopSetPair : loopSets)
  {
    /*
     * WARNING: Should check if SCCs are already in a partition; if so, merge partitions
     */
    for (auto sccNode : loopSetPair.second) LDI->sccToPartition[sccNode->getT()] = LDI->nextPartitionID;
    LDI->nextPartitionID++;
  }
}

void DSWP::mergeBranchesWithoutOutgoingEdges (DSWPLoopDependenceInfo *LDI)
{
  std::vector<DGNode<SCC> *> tailCmpBrs;
  for (auto sccNode : LDI->loopSCCDAG->getNodes())
  {
    auto scc = sccNode->getT();
    if (sccNode->numIncomingEdges() == 0 || sccNode->numOutgoingEdges() > 0) continue ;

    bool allCmpOrBr = true;
    for (auto node : scc->getNodes())
    {
      allCmpOrBr &= (isa<TerminatorInst>(node->getT()) || isa<CmpInst>(node->getT()));
    }
    if (allCmpOrBr) tailCmpBrs.push_back(sccNode);
  }

  /*
   * Merge trailing compare/branch scc into previous depth scc
   */
  for (auto tailSCC : tailCmpBrs)
  {
    std::set<DGNode<SCC> *> nodesToMerge = { tailSCC };
    nodesToMerge.insert(*LDI->loopSCCDAG->previousDepthNodes(tailSCC).begin());
    LDI->loopSCCDAG->mergeSCCs(nodesToMerge);
  }
}

void DSWP::partitionSCCDAG (DSWPLoopDependenceInfo *LDI)
{
  // errs() << "Number of unmerged nodes: " << LDI->loopSCCDAG->numNodes() << "\n";
  if (this->forceNoSCCMerge) return ;

  /*
   * Merge the SCC related to a single PHI node and its use if there is only one.
   */
  mergePointerLoadInstructions(LDI);
  mergeSinglePHIs(LDI);
  mergeBranchesWithoutOutgoingEdges(LDI);

  // WARNING: Uses LI to determine subloop information
  clusterSubloops(LDI);

  /*
   * Separate remaining unpartitioned nodes into their own partitions
   */
  for (auto nodePair : LDI->loopSCCDAG->internalNodePairs()) {
    if (LDI->removableSCCs.find(nodePair.first) != LDI->removableSCCs.end()) continue;
    if (LDI->sccToPartition.find(nodePair.first) == LDI->sccToPartition.end()) {
      LDI->sccToPartition[nodePair.first] = LDI->nextPartitionID++;
    }
  }

  // errs() << "Number of merged nodes: " << LDI->loopSCCDAG->numNodes() << "\n";
  return ;
}
