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
#include "../include/SCCDG.hpp"

llvm::SCCDG::SCCDG() {}

llvm::SCCDG::~SCCDG() {
  for (auto *edge : allEdges)
    if (edge) delete edge;
  for (auto *node : allNodes)
    if (node) delete node;
}


SCCDG *llvm::SCCDG::createSCCGraphFrom(PDG *pdg) {
  auto sccDG = new SCCDG();

  /*
   * Iterate over all connected components of the PDG and calculate strongly connected components
   */
  auto components = pdg->collectConnectedComponents();
  for (auto componentNodes : components)
  {
    auto componentPDG = new PDG();
    pdg->extractNodesFromSelfInto(*cast<DG<Instruction>>(componentPDG), *componentNodes, *componentNodes->begin(), false);
    delete componentNodes;

    std::set<DGNode<Instruction> *> nodesInSCCs;
    for (auto topLevelNode : componentPDG->getTopLevelNodes())
    {
      componentPDG->setEntryNode(topLevelNode);
      for (auto pdgI = scc_begin(componentPDG); pdgI != scc_end(componentPDG); ++pdgI)
      {
        std::vector<DGNode<Instruction> *> nodes;
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
  auto nodeSCCMap = unordered_map<DGNode<Instruction> *, SCC *>();
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
  auto fetchOrCreateSCC = [&nodeSCCMap, sccDG](DGNode<Instruction> *node) -> SCC* {
    auto sccI = nodeSCCMap.find(node);
    if (sccI == nodeSCCMap.end()) {
      vector<DGNode<Instruction> *> sccNodes = { node };
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
     * Create edge between SCCs with same properties as the edge between instructions within the SCCs
     */
    auto sccEdge = sccDG->createEdgeFromTo(fromSCC, toSCC);
    sccEdge->setMemMustRaw(edge->isMemoryDependence(), edge->isMustDependence(), edge->isRAWDependence());
    sccEdge->addSubEdge(edge);
  }

  return sccDG;
}

SCCDG *llvm::SCCDG::extractSCCIntoGraph(DGNode<SCC> *sccNode)
{
  SCCDG *sccDG = new SCCDG();
  std::vector<DGNode<SCC> *> sccNodes = { sccNode };
  extractNodesFromSelfInto(*cast<DG<SCC>>(sccDG), sccNodes, sccNode, /*removeFromSelf=*/ true);
  return sccDG;
}

bool llvm::SCCDG::isPipeline()
{
  std::queue<DGNode<SCC> *> visiting;
  std::set<DGNode<SCC> *> visited;

  /*
   * Traverse from arbitrary SCC to the top, if one exists
   */
  auto topOfPipeline = *begin_nodes();
  while (topOfPipeline->numIncomingEdges() != 0)
  {
    if (visited.find(topOfPipeline) != visited.end()) return false;
    visited.insert(topOfPipeline);
    topOfPipeline = *topOfPipeline->begin_incoming_nodes();
  }

  /*
   * Traverse from top SCC to all nodes to confirm full coverage and no cycles
   */
  visited.clear();
  visiting.push(topOfPipeline);
  unsigned numNodesReached = 0;
  while (!visiting.empty())
  {
    auto currentNode = visiting.front();
    visiting.pop();
    if (visited.find(currentNode) != visited.end()) return false;
    visited.insert(currentNode);
    ++numNodesReached;

    for (auto node : make_range(currentNode->begin_outgoing_nodes(), currentNode->end_outgoing_nodes()))
    {
      visiting.push(node);
    }
  }

  return numNodesReached == this->numNodes();
}