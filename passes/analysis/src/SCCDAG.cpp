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
  auto sccDG = new SCCDAG();

  /*
   * Iterate over all connected components of the PDG and calculate strongly connected components
   */

  // RENAME: Get disconnected subgraphs
  auto components = pdg->collectConnectedComponents();
  
  for (auto componentNodes : components)
  {
    auto componentPDG = new PDG();
    pdg->partitionNodesIntoNewGraph(*cast<DG<Value>>(componentPDG), *componentNodes, *componentNodes->begin());
    delete componentNodes;

    std::set<DGNode<Value> *> nodesInSCCs;
    for (auto topLevelNode : componentPDG->getTopLevelNodes())
    {
      componentPDG->setEntryNode(topLevelNode);
      for (auto pdgI = scc_begin(componentPDG); pdgI != scc_end(componentPDG); ++pdgI)
      {
        std::vector<DGNode<Value> *> nodes;
        bool uniqueSCC = true;
        for (auto node : *pdgI)
        {
          if (nodesInSCCs.find(node) != nodesInSCCs.end())
          {
            uniqueSCC = false;
            break;
          }
          nodes.push_back(node);
          nodesInSCCs.insert(node);
        }

        if (!uniqueSCC) continue;
        auto scc = new SCC(nodes);
        sccDG->createNodeFrom(scc, /*inclusion=*/ true);
      }
    }

    /*
     * Delete just the component holder, not the nodes; the nodes are references to those of the pdg passed in
     */
    componentPDG->clear();
    delete componentPDG;
  }

  /*
   * Maintain association of each internal node to its SCC
   */
  auto valueNodeToSCCNode = unordered_map<DGNode<Value> *, DGNode<SCC> *>();
  for (auto sccNode : make_range(sccDG->begin_nodes(), sccDG->end_nodes()))
  {
    for (auto instPair : sccNode->getT()->internalNodePairs())
    {
      valueNodeToSCCNode[instPair.second] = sccNode;
    }
  }

  /*
   * Helper function to find or create an SCC from a node
   */
  auto fetchOrCreateSCCNode = [&](DGNode<Value> *node) -> DGNode<SCC> * {
    auto sccNodeI = valueNodeToSCCNode.find(node);
    if (sccNodeI == valueNodeToSCCNode.end()) {
      vector<DGNode<Value> *> sccNodes = { node };
      auto scc = new SCC(sccNodes);
      auto sccNode = sccDG->createNodeFrom(scc, /*inclusion=*/ false);
      valueNodeToSCCNode[node] = sccNode;
      return sccNode;
    }
    return sccNodeI->second;
  };

  /*
   * Add internal/external edges between SCCs
   */
  for (auto pdgEI = pdg->begin_edges(); pdgEI != pdg->end_edges(); ++pdgEI) {
    auto edge = *pdgEI;
    auto nodePair = edge->getNodePair();
    auto fromSCCNode = fetchOrCreateSCCNode(nodePair.first);
    auto toSCCNode = fetchOrCreateSCCNode(nodePair.second);
    auto fromSCC = fromSCCNode->getT();
    auto toSCC = toSCCNode->getT();

    /*
     * If the edge points to external SCCs or is contained in a single SCC, ignore 
     */
    if ((sccDG->isExternal(fromSCC) && sccDG->isExternal(toSCC)) || fromSCC == toSCC) continue;

    /*
     * Find or create edge between SCCs
     * Add instruction subedge to it
     */
    auto sccNodeIter = fromSCCNode->connectedNodeIterTo(toSCCNode);
    auto sccEdge = fromSCCNode->getEdgeFromConnectedNodeIterator(sccNodeIter);
    if (sccEdge == nullptr)
    {
      sccEdge = sccDG->createEdgeFromTo(fromSCC, toSCC);
    }
    sccEdge->addSubEdge(edge);
  }

  return sccDG;
}