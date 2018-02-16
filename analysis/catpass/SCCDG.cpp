#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include <set>

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
  auto nodeSCCMap = std::map<DGNode<Instruction> *, SCC<DGNode<Instruction>> *>();
  while (!pdgi.isAtEnd()) {
    const std::vector<DGNode<Instruction> *> nodesInSCC = *pdgi;
    auto scc = new SCC<DGNode<Instruction>>(nodesInSCC);
    sccDG->createNodeFrom(scc);
    ++pdgi;

    /*
     * Add edges between SCC nodes based on their internal nodes' edges 
     */
    for (auto node : nodesInSCC) {
      for (auto outgoing = node->begin_outgoing_nodes(); outgoing != node->end_outgoing_nodes(); ++outgoing) {
        auto sccIter = nodeSCCMap.find(*outgoing);
        if (sccIter == nodeSCCMap.end()) continue;
        sccDG->createEdgeFromTo(scc, sccIter->second);
      }
    }

    /*
     * Maintain association of each internal node to its SCC node
     */
    for (auto node : nodesInSCC)
      nodeSCCMap[node] = scc;
  }
}