/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni, Brian Homerding
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/SCCDAGNormalizer.hpp"
#include "LoopCarriedDependencies.hpp"

namespace llvm::noelle {

SCCDAGNormalizer::SCCDAGNormalizer (
  SCCDAG &dag, 
  StayConnectedNestedLoopForestNode *loop
  )
  : loop{loop}, sccdag{dag} {

  return ;
}

void SCCDAGNormalizer::normalizeInPlace (void) {

  /*
   * Note: the grouping of LCSSA instructions with the loop header PHI they
   * close is necessary for parallelization techniques and must be prioritized
   * above any other normalizations that merge PHI instruction SCCs
   */
  mergeLCSSAPhis();

  /*
   * NOTE: The merging of external loop carried dependencies between SCC is
   * necessary for parallelization techniques
   */
  mergeSCCsWithExternalInterIterationDependencies();

  mergeSingleSyntacticSugarInstrs();
  mergeBranchesWithoutOutgoingEdges();

  collapseIntroducedCycles();

}

void SCCDAGNormalizer::mergeLCSSAPhis () {
  MergeGroups mergeGroups;
  for (auto sccNode : sccdag.getNodes()) {
    auto scc = sccNode->getT();
    if (scc->numInternalNodes() != 1) continue;

    auto I = scc->begin_internal_node_map()->first;
    if (!isa<PHINode>(I)) continue;

    auto phi = cast<PHINode>(I);
    if (phi->getNumIncomingValues() != 1) continue;

    auto incomingI = phi->getIncomingValue(0);
    if (!isa<PHINode>(incomingI)) continue;

    auto incomingPHI = cast<PHINode>(incomingI);
    auto incomingLoop = this->loop->getInnermostLoopThatContains(incomingPHI->getParent());
    if (!incomingLoop || incomingLoop->getHeader() != incomingPHI->getParent()) continue;

    mergeGroups.merge(sccdag.fetchNode(sccdag.sccOfValue(incomingI)), sccNode);
  }

  for (auto sccNodes : mergeGroups.groups) { 
    sccdag.mergeSCCs(*sccNodes);
  }
}

void SCCDAGNormalizer::mergeSCCsWithExternalInterIterationDependencies (void) {
  auto isLastValuePHI = [](SCC *scc) -> bool {
    if (scc->numInternalNodes() == 1) {
      auto I = scc->begin_internal_node_map()->first;
      if (isa<PHINode>(I)) {
        return true;
      }
    }

    return false;
  };

  MergeGroups mergeGroups{};
  for (auto loop : this->loop->getLoops()) {
    auto loopCarriedEdges = LoopCarriedDependencies::getLoopCarriedDependenciesForLoop(*loop, this->loop, sccdag);
    for (auto edge : loopCarriedEdges) {
      if (!edge->isDataDependence()) continue;

      auto producer = edge->getOutgoingT();
      auto consumer = edge->getIncomingT();
      auto producerSCC = sccdag.sccOfValue(producer);
      if (!producerSCC->isExternal(consumer)) {
        continue ;
      }

      /*
      * Fetch the SCC that is the destination of the current loop-carried data dependence of @producerSCC.
      *
      * Notice that @producerSCC cannot be @consumerSCC as the latter has one node that is not included in the former.
      */
      auto consumerSCC = sccdag.sccOfValue(consumer);
      assert(producerSCC != consumerSCC);

      /*
      * Check the consumer SCC.
      */
      // if (!isLastValuePHI(consumerSCC)) {
      //   errs() << "SCCDAGNormalizer:  Unknown SCC with external loop carried dependence edge!\n";
      //   edge->print(errs()) << "\n";
      // }

      /*
      * Merge @produerSCC with @consumerSCC
      */
      mergeGroups.merge(sccdag.fetchNode(producerSCC), sccdag.fetchNode(consumerSCC));
    }
  }

  for (auto sccNodes : mergeGroups.groups) { 
    sccdag.mergeSCCs(*sccNodes);
  }
}

void SCCDAGNormalizer::mergeSingleSyntacticSugarInstrs (void) {
  MergeGroups mergeGroups;

  /*
   * Iterate over SCCs.
   */
  for (auto sccPair : sccdag.internalNodePairs()){
    auto scc = sccPair.first;
    auto sccNode = sccPair.second;

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

    mergeGroups.merge(sccNode, adjacentNode);
  }

  for (auto sccNodes : mergeGroups.groups) { 
    sccdag.mergeSCCs(*sccNodes);
  }
}

void SCCDAGNormalizer::mergeBranchesWithoutOutgoingEdges (void) {
  std::vector<DGNode<SCC> *> tailCmpBrs;
  for (auto sccPair : sccdag.internalNodePairs()){
    auto scc = sccPair.first;
    auto sccNode = sccPair.second;

    /*
     * Merging this CmpInst and/or terminator containing SCC node is only done
     * when there is no child SCC and at least one parent SCC
     */
    if (sccNode->numIncomingEdges() == 0 || sccNode->numOutgoingEdges() > 0) continue ;

    bool allCmpOrBr = true;
    for (auto nodePair : scc->internalNodePairs()){
      auto nodeValue = nodePair.first;
      auto node = nodePair.second;

      /*
       * Handle the cmp instruction.
       */
      if (isa<CmpInst>(nodeValue)){
        allCmpOrBr &= true;
        continue ;
      }

      /*
       * Handle the branch instruction.
       */
      auto nodeInst = dyn_cast<Instruction>(nodeValue);
      if (nodeInst == nullptr){
        allCmpOrBr &= false;
        continue ;
      }
      allCmpOrBr &= nodeInst->isTerminator();
    }
    if (allCmpOrBr) tailCmpBrs.push_back(sccNode);
  }

  /*
   * Merge trailing compare/branch scc into previous depth scc
   */
  for (auto tailSCC : tailCmpBrs) {
    std::set<DGNode<SCC> *> nodesToMerge = { tailSCC };
    nodesToMerge.insert(*sccdag.getPreviousDepthNodes(tailSCC).begin());
    sccdag.mergeSCCs(nodesToMerge);
  }
}

SCCDAGNormalizer::MergeGroups::~MergeGroups() {
  for (auto group : groups) {
    delete group;
  }
}

void SCCDAGNormalizer::MergeGroups::merge(DGNode<SCC> *sccNode1, DGNode<SCC> *sccNode2) {

  /*
   * Determine whether each node is grouped already. Merge groups where necessary
   */
  bool isGrouped1 = sccToGroupMap.find(sccNode1) != sccToGroupMap.end();
  bool isGrouped2 = sccToGroupMap.find(sccNode2) != sccToGroupMap.end();
  if (isGrouped1 && isGrouped2) {

    /*
     * Fetch the groups.
     */
    auto group1 = sccToGroupMap[sccNode1];
    auto group2 = sccToGroupMap[sccNode2];

    /*
     * If the two SCCs already belong to the same group, then there is nothing to do.
     */
    if (group1 == group2) {
      return ;
    }

    /*
     * Add the second group to the first one.
     */
    for (auto node : *group2) {
      group1->insert(node);
      sccToGroupMap[node] = group1;
    }

    /*
     * Delete the second group.
     */
    groups.erase(group2);
    delete group2;

  } else if (isGrouped1) {
    sccToGroupMap[sccNode1]->insert(sccNode2);
    sccToGroupMap[sccNode2] = sccToGroupMap[sccNode1];
  } else if (isGrouped2) {
    sccToGroupMap[sccNode2]->insert(sccNode1);
    sccToGroupMap[sccNode1] = sccToGroupMap[sccNode2];
  } else {
    auto nodes = new std::set<DGNode<SCC> *>({ sccNode1, sccNode2 });
    groups.insert(nodes);
    sccToGroupMap[sccNode1] = nodes;
    sccToGroupMap[sccNode2] = nodes;
  }
}

// void SCCDAGNormalizer::collapseIntroducedCycles () {

//   /*
//    * Construct SCC relations needed for SCCDAGPartition to process
//    * each SCC as its own subset and to handle any potential cycles
//    * we may have introduced by merging SCCs
//    */
//   std::unordered_map<SCC *, std::set<SCC *>> sccToParentMap;
//   std::set<std::set<SCC *> *> singleSCCs;
//   for (auto consumerNode : sccdag.getNodes()) {
//     auto consumerSCC = consumerNode->getT();
//     std::set<SCC *> * singleSCC = new std::set<SCC *>();
//     singleSCC->insert(consumerSCC);
//     singleSCCs.insert(singleSCC);
//     for (auto edge : consumerNode->getIncomingEdges()) {
//       auto producerSCC = edge->getOutgoingT();
//       sccToParentMap[consumerSCC].insert(producerSCC);
//     }
//   }

//   /*
//    * Use the partition to collapse cycles.
//    * Merge any resulting subsets that aren't individual SCCs
//    */
//   SCCDAGPartition partition(&sccdag, sccToParentMap, LIS.getLoopNestingTreeRoot(), &singleSCCs);
//   for (auto subset : *partition.getSubsets()) {
//     if (subset->size() == 1) continue;

//     std::set<DGNode<SCC> *> nodesToMerge;
//     for (auto scc : *subset) {
//       nodesToMerge.insert(sccdag.fetchNode(scc));
//     }

//     sccdag.mergeSCCs(nodesToMerge);
//   }

//   for (auto singleSCC : singleSCCs) {
//     delete singleSCC;
//   }
// }

}
