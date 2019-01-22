/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/ADT/SCCIterator.h"
#include <set>
#include <unordered_map>

#include "../include/DGGraphTraits.hpp"
#include "../include/SCCDAG.hpp"

llvm::SCCDAG::SCCDAG() {}

llvm::SCCDAG::~SCCDAG() {
  for (auto *edge : allEdges)
    if (edge) delete edge;
  for (auto *node : allNodes)
    if (node) delete node;
}


SCCDAG *llvm::SCCDAG::createSCCDAGFrom(PDG *pdg)
{
  auto sccDAG = new SCCDAG();

  /*
   * Iterate over all disconnected subgraphs of the PDG and calculate their strongly connected components
   */
  auto subgraphs = pdg->getDisconnectedSubgraphs();
  for (auto subgraphNodeset : subgraphs)
  {
    auto subgraphPDG = new PDG();
    pdg->addNodesIntoNewGraph(*cast<DG<Value>>(subgraphPDG), *subgraphNodeset, *subgraphNodeset->begin());
    delete subgraphNodeset;

    std::set<Value *> valuesInSCCs;
    for (auto topLevelNode : subgraphPDG->getTopLevelNodes())
    {
      subgraphPDG->setEntryNode(topLevelNode);
      std::set<DGNode<Value> *> nodes;
      for (auto pdgI = scc_begin(subgraphPDG); pdgI != scc_end(subgraphPDG); ++pdgI)
      {
        nodes.clear();
        bool uniqueSCC = true;
        for (auto node : *pdgI)
        {
          if (valuesInSCCs.find(node->getT()) != valuesInSCCs.end())
          {
            uniqueSCC = false;
            break;
          }
          nodes.insert(node);
          valuesInSCCs.insert(node->getT());
        }

        if (!uniqueSCC) continue;
        auto scc = new SCC(nodes);
        sccDAG->addNode(scc, /*inclusion=*/ true);
      }
    }

    /*
     * Delete just the subgraph holder, not the nodes/edges which belong to the pdg input
     */
    subgraphPDG->clear();
    delete subgraphPDG;
  }

  sccDAG->markValuesInSCC();
  sccDAG->markEdgesAndSubEdges();
  return sccDAG;
}

void llvm::SCCDAG::markValuesInSCC()
{
  /*
   * Maintain association of each internal node to its SCC
   */
  this->valueToSCCNode.clear();
  for (auto sccNode : this->getNodes())
  {
    for (auto instPair : sccNode->getT()->internalNodePairs())
    {
      this->valueToSCCNode[instPair.first] = sccNode;
    }
  }
}

void llvm::SCCDAG::markEdgesAndSubEdges()
{
  /*
   * Add edges between SCCs by looking at each SCC's outgoing edges
   */
  std::set<DGEdge<SCC> *> clearedEdges;
  for (auto outgoingSCCNode : this->getNodes())
  {
    auto outgoingSCC = outgoingSCCNode->getT();
    for (auto externalNodePair : outgoingSCC->externalNodePairs())
    {
      auto incomingNode = externalNodePair.second;
      if (incomingNode->numIncomingEdges() == 0) continue;

      auto incomingSCCNode = this->valueToSCCNode[externalNodePair.first];
      auto incomingSCC = incomingSCCNode->getT();

      /*
       * Find or create unique edge between the two connected SCC
       */
      auto edgeSet = outgoingSCCNode->getEdgesToAndFromNode(incomingSCCNode);
      auto sccEdge = edgeSet.empty() ? this->addEdge(outgoingSCC, incomingSCC) : (*edgeSet.begin());

      /*
       * Clear out subedges if not already done once; add all currently existing subedges
       */
      if (clearedEdges.find(sccEdge) == clearedEdges.end())
      {
        sccEdge->clearSubEdges();
        clearedEdges.insert(sccEdge);
      }
      for (auto edge : incomingNode->getIncomingEdges()) sccEdge->addSubEdge(edge);
    }
  }
}

void llvm::SCCDAG::mergeSCCs(std::set<DGNode<SCC> *> &sccSet)
{
  if (sccSet.size() < 2) return;
  
  std::set<DGNode<Value> *> mergeNodes;
  for (auto sccNode : sccSet)
  {
    for (auto internalNodePair : sccNode->getT()->internalNodePairs())
    {
      mergeNodes.insert(internalNodePair.second);
    }
  }
  auto mergeSCC = new SCC(mergeNodes);

  /*
   * Add the new SCC and remove the old ones
   * Reassign values to the SCC they are now in
   * Recreate all edges from SCCs to the newly merged SCC
   */
  auto mergeSCCNode = this->addNode(mergeSCC, /*inclusion=*/ true);
  for (auto sccNode : sccSet) this->removeNode(sccNode);
  this->markValuesInSCC();
  this->markEdgesAndSubEdges();
}

SCC *llvm::SCCDAG::sccOfValue (Value *val) const {
  return valueToSCCNode.find(val)->second->getT();
}
