#include "Parallelizer.hpp"

using namespace llvm;

void Parallelizer::mergeTrivialNodesInSCCDAG (DSWPLoopDependenceInfo *LDI) {

  /*
   * Print the current SCCDAG.
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DSWP:  Before merging SCCs\n";
    printSCCs(LDI->loopSCCDAG);
  }

  /*
   * Merge SCCs.
   */
  mergeSingleSyntacticSugarInstrs(LDI);
  mergeBranchesWithoutOutgoingEdges(LDI);

  /*
   * Print the current SCCDAG.
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DSWP:  After merging SCCs\n";
    printSCCs(LDI->loopSCCDAG);
  }

  return ;
}

void Parallelizer::mergeSingleSyntacticSugarInstrs (DSWPLoopDependenceInfo *LDI) {
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

void Parallelizer::mergeBranchesWithoutOutgoingEdges (DSWPLoopDependenceInfo *LDI) {
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
