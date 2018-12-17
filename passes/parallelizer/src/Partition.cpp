/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Parallelizer.hpp"

using namespace llvm;

void Parallelizer::mergeTrivialNodesInSCCDAG (DSWPLoopDependenceInfo *LDI) {

  /*
   * Print the current SCCDAG.
   */
  // NOTE(angelo): For the sake of brevity in logging, these have been removed
  if (this->verbose >= Verbosity::Maximal) {
    // errs() << "Parallelizer:  Before merging SCCs\n";
    // printSCCs(LDI->loopSCCDAG);
  }

  /*
   * Merge SCCs.
   */
  // NOTE(angelo): For the sake of brevity in logging, instead of logging
  // the before and after, which can be VERY verbose, we should just log
  // each change made by these merge helpers. This would still capture everything
  // necessary for debugging purposes.
  mergeSingleSyntacticSugarInstrs(LDI);
  mergeBranchesWithoutOutgoingEdges(LDI);

  /*
   * Print the current SCCDAG.
   */
  // NOTE(angelo): For the sake of brevity in logging, these have been removed
  if (this->verbose >= Verbosity::Maximal) {
    // errs() << "Parallelizer:  After merging SCCs\n";
    // printSCCs(LDI->loopSCCDAG);
  }

  return ;
}

void Parallelizer::mergeSingleSyntacticSugarInstrs (DSWPLoopDependenceInfo *LDI) {
  std::unordered_map<DGNode<SCC> *, std::set<DGNode<SCC> *> *> mergedToGroup;
  std::set<std::set<DGNode<SCC> *> *> singles;
  for (auto sccNode : LDI->loopSCCDAG->getNodes()) {
    auto scc = sccNode->getT();

    /*
     * Determine if node is a single syntactic sugar instruction that has either
     * a single parent SCC or a single child SCC
     */
    if (scc->numInternalNodes() > 1) continue;
    auto I = scc->begin_internal_node_map()->first;
    if (!isa<PHINode>(I) && !isa<GetElementPtrInst>(I) && !isa<CastInst>(I)) continue;

    // TODO: Even if more than one edge exists, attempt next/previous depth SCCs.
    DGNode<SCC> *adjacentNode = nullptr;
    if (sccNode->numOutgoingEdges() == 1) {
      adjacentNode = (*sccNode->begin_outgoing_edges())->getIncomingNode();
    }
    if (sccNode->numIncomingEdges() == 1) {
      auto incomingOption = (*sccNode->begin_incoming_edges())->getOutgoingNode();
      if (!adjacentNode) {
        adjacentNode = incomingOption;
      } else {

        /*
         * NOTE: generally, these are lcssa PHIs, or casts of previous PHIs/instructions
         * If a GEP, it's load is in the child SCC, so leave it with the child
         */
        if (isa<PHINode>(I) || isa<CastInst>(I)) adjacentNode = incomingOption;
      }
    }
    if (!adjacentNode) continue;

    /*
     * Determine the merged state of the single instruction node and its adjacent
     * Merge the current merged nodes holding both of the above
     */
    bool mergedSCCNode = mergedToGroup.find(sccNode) != mergedToGroup.end();
    bool mergedAdjNode = mergedToGroup.find(adjacentNode) != mergedToGroup.end();
    if (mergedSCCNode && mergedAdjNode) {

      /*
       * Combine the adjacent node's merged group into that of the single instruction's merged group
       */
      auto adjSet = mergedToGroup[adjacentNode];
      for (auto node : *adjSet) {
        mergedToGroup[sccNode]->insert(node);
        mergedToGroup[node] = mergedToGroup[sccNode];
      }
      singles.erase(adjSet);
      delete adjSet;
    } else if (mergedSCCNode) {
      mergedToGroup[sccNode]->insert(adjacentNode);
      mergedToGroup[adjacentNode] = mergedToGroup[sccNode];
    } else if (mergedAdjNode) {
      mergedToGroup[adjacentNode]->insert(sccNode);
      mergedToGroup[sccNode] = mergedToGroup[adjacentNode];
    } else {
      auto nodes = new std::set<DGNode<SCC> *>({ sccNode, adjacentNode });
      singles.insert(nodes);
      mergedToGroup[sccNode] = nodes;
      mergedToGroup[adjacentNode] = nodes;
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
