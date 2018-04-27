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


SCCDAG *llvm::SCCDAG::createSCCDAGFrom(PDG *pdg) {
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
      topLevelNode->print(errs() << "Top level node:\n") << "\n";

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
        errs() << "SCC:\n";
        for (auto node : nodes) node->print(errs()) << "\n";
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

  /*
   * Maintain association of each internal node to its SCC
   */
  auto valueToSCCNode = unordered_map<Value *, DGNode<SCC> *>();
  for (auto sccNode : sccDAG->getNodes())
  {
    for (auto instPair : sccNode->getT()->internalNodePairs())
    {
      valueToSCCNode[instPair.first] = sccNode;
    }
  }

  /*
   * Helper function to find or create an SCC from a node
   */
  auto fetchOrCreateSCCNode = [&](DGNode<Value> *node) -> DGNode<SCC> * {
    auto sccNodeI = valueToSCCNode.find(node->getT());
    if (sccNodeI == valueToSCCNode.end()) {
      set<DGNode<Value> *> sccNodes = { node };
      auto scc = new SCC(sccNodes);
      auto sccNode = sccDAG->addNode(scc, /*inclusion=*/ false);
      valueToSCCNode[node->getT()] = sccNode;
      return sccNode;
    }
    return sccNodeI->second;
  };

  /*
   * Add internal/external edges between SCCs
   */
  for (auto edge : pdg->getEdges())
  {
    auto nodePair = edge->getNodePair();
    auto fromSCCNode = fetchOrCreateSCCNode(nodePair.first);
    auto toSCCNode = fetchOrCreateSCCNode(nodePair.second);
    auto fromSCC = fromSCCNode->getT();
    auto toSCC = toSCCNode->getT();

    /*
     * If the edge points to external SCCs or is contained in a single SCC, ignore 
     */
    if ((sccDAG->isExternal(fromSCC) && sccDAG->isExternal(toSCC)) || fromSCC == toSCC) continue;

    /*
     * Find or create edge between SCCs
     * Add instruction subedge to it
     */
    DGEdge<SCC> *sccEdge;
    auto edgeSet = fromSCCNode->getEdgesToAndFromNode(toSCCNode);
    if (edgeSet.empty())
    {
      sccEdge = sccDAG->addEdge(fromSCC, toSCC);
    }
    else
    {
      sccEdge = (*edgeSet.begin());
    }

    sccEdge->addSubEdge(edge);
  }

  return sccDAG;
}