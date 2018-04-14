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
    pdg->extractNodesFromSelfInto(*cast<DG<Value>>(componentPDG), *componentNodes, *componentNodes->begin(), false);
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
  auto nodeSCCMap = unordered_map<DGNode<Value> *, SCC *>();
  for (auto sccNode : make_range(sccDG->begin_nodes(), sccDG->end_nodes()))
  {
    auto scc = sccNode->getT();
    for (auto nodePair : scc->internalNodePairs())
    {
      nodeSCCMap[nodePair.second] = scc;
    }
  }

  /*
   * Helper function to find or create an SCC from a node
   */
  auto fetchOrCreateSCC = [&nodeSCCMap, sccDG](DGNode<Value> *node) -> SCC* {
    auto sccI = nodeSCCMap.find(node);
    if (sccI == nodeSCCMap.end()) {
      vector<DGNode<Value> *> sccNodes = { node };
      auto scc = new SCC(sccNodes);
      sccDG->createNodeFrom(scc, /*inclusion=*/ false);
      nodeSCCMap[node] = scc;
      return scc;
    }
    return sccI->second;
  };

  /*
   * Add internal/external edges between SCCs
   */
  for (auto pdgEI = pdg->begin_edges(); pdgEI != pdg->end_edges(); ++pdgEI) {
    auto edge = *pdgEI;
    auto nodePair = edge->getNodePair();
    auto fromSCC = fetchOrCreateSCC(nodePair.first);
    auto toSCC = fetchOrCreateSCC(nodePair.second);

    /*
     * If the edge points to external SCCs or is contained in a single SCC, ignore 
     */
    if ((sccDG->isExternal(fromSCC) && sccDG->isExternal(toSCC)) || fromSCC == toSCC) continue;

    /*
     * Create edge between SCCs with same properties as the edge between values within the SCCs
     */
    auto sccEdge = sccDG->createEdgeFromTo(fromSCC, toSCC);
    sccEdge->setMemMustRaw(edge->isMemoryDependence(), edge->isMustDependence(), edge->isRAWDependence());
    sccEdge->addSubEdge(edge);
  }

  return sccDG;
}

SCCDAG *llvm::SCCDAG::extractSCCIntoGraph(DGNode<SCC> *sccNode)
{
  SCCDAG *sccDG = new SCCDAG();
  std::vector<DGNode<SCC> *> sccNodes = { sccNode };
  extractNodesFromSelfInto(*cast<DG<SCC>>(sccDG), sccNodes, sccNode, /*removeFromSelf=*/ true);
  return sccDG;
}
