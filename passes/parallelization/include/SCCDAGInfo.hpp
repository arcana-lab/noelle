#pragma once

#include "SCCDAG.hpp"
#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace llvm;

namespace llvm {

  /*
   * Modelling assumption: Each edge listed may yield potential queue of unit cost
   */
  struct SCCEdgeInfo {
    std::set<Value *> edges;
  };

  struct SCCInfo {
    SCC *scc;
    std::set<BasicBlock *> bbs;

    int internalCost;
    std::unordered_map<SCC *, std::unique_ptr<SCCEdgeInfo>> sccToEdgeInfo;
    bool hasLoopCarriedDep;

    SCCInfo (SCC *s) : scc{s}, internalCost{0}, hasLoopCarriedDep{0} {
      for (auto nodePair : this->scc->internalNodePairs()) {
        this->bbs.insert(cast<Instruction>(nodePair.first)->getParent());
      }
    }
  };

  struct SCCDAGInfo {
    SCCDAG *sccdag;
    std::unordered_map<SCC *, std::unique_ptr<SCCInfo>> sccToInfo;

    void populate (SCCDAG *loopSCCDAG) {
      this->sccdag = loopSCCDAG;
      for (auto node : loopSCCDAG->getNodes()) {
        this->sccToInfo[node->getT()] = std::move(std::make_unique<SCCInfo>(node->getT()));
      }
    }
  };
}
