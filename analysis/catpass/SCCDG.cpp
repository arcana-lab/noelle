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

  auto nodeSCCMap = unordered_map<DGNode<Instruction> *, SCC *>();

  /*
  errs() << "All nodes in calculation:\n";
  for (auto nodeI = pdg->begin_nodes(); nodeI != pdg->end_nodes(); nodeI++) {
    (*nodeI)->getNode()->print(errs());
    errs() << "\n";
  }
  errs() << "All edges in calculation:\n";
  for (auto edgeI = pdg->begin_edges(); edgeI != pdg->end_edges(); edgeI++) {
    (*edgeI)->print(errs());
    errs() << "\n";
  }
  */

  scc_iterator<PDG *> pdgI = scc_begin(pdg);
  while (!pdgI.isAtEnd()) {
    const std::vector<DGNode<Instruction> *> nodesInSCC = *pdgI;
    auto scc = new SCC(nodesInSCC);
    sccDG->createNodeFrom(scc, /*inclusion=*/ true);

    /*
     * Maintain association of each internal node to its SCC
     */
    //errs() << "SCC:\n";
    for (auto nodeI = scc->begin_nodes(); nodeI != scc->end_nodes(); nodeI++) {
      //(*nodeI)->getNode()->print(errs());
      //errs() << "\n";
      nodeSCCMap[(*nodeI)] = scc;
    }
    ++pdgI;
  }
  //errs() << "\n";

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
  }

  return sccDG;
}