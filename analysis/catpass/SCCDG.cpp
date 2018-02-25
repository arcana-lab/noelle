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

  scc_iterator<PDG *> pdgi = scc_begin(pdg);
  auto nodeSCCMap = unordered_map<DGNode<Instruction> *, SCC *>();
  while (!pdgi.isAtEnd()) {
    const std::vector<DGNode<Instruction> *> nodesInSCC = *pdgi;
    auto scc = new SCC(nodesInSCC);
    sccDG->createNodeFrom(scc, /*inclusion=*/ true);
    ++pdgi;

    /*
     * Maintain association of each internal node to its SCC node
     */
    for (auto node : nodesInSCC) {
      nodeSCCMap[node] = scc;
    }
  }

  /*
   * Add internal/external edges on SCC nodes 
   */
  for (auto pdgEI = pdg->begin_edges(); pdgEI != pdg->end_edges(); ++pdgEI) {
    auto edge = *pdgEI;
    auto nodePair = edge->getNodePair();
    auto fromNode = nodePair.first;
    auto toNode = nodePair.second;

    /*
     * If both nodes are external or contained within the same SCC, ignore 
     */
    if (nodeSCCMap.find(fromNode) == nodeSCCMap.find(toNode)) continue;

    /*if (fromSCC == nodeSCCMap.end() || toSCC == nodeSCCMap.end()) {
      //errs() << "SCCDG Construction: Node encountered not in a SCC\n";
      //abort();
    }*/
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
     * Create edge between SCCs with same properties as the edge between instructions within the SCCs
     */
    auto sccEdge = sccDG->createEdgeFromTo(fetchOrCreateSCC(fromNode), fetchOrCreateSCC(toNode));
    sccEdge->setMemMustRaw(edge->isMemoryDependence(), edge->isMustDependence(), edge->isRAWDependence());
  }

  return sccDG;
}