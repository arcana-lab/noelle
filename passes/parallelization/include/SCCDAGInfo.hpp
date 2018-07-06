#pragma once

#include "SCCDAG.hpp"
#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace llvm;

namespace llvm {
  
  struct SCCInfo {
    SCC *scc;
    std::set<BasicBlock *> bbs;
    int cost;
    bool hasLoopCarriedDep;

    SCCInfo (SCC *s) : scc{s}, cost{0}, hasLoopCarriedDep{0} {
      for (auto nodePair : scc->internalNodePairs()) {
        bbs.insert(cast<Instruction>(nodePair.first)->getParent());
      }
    }
  };

  struct SCCDAGInfo {
    std::unordered_map<SCC *, std::unique_ptr<SCCInfo>> sccToInfo;

    void populate (SCCDAG *loopSCCDAG) {
      for (auto node : loopSCCDAG->getNodes()) {
        sccToInfo[node->getT()] = std::move(std::make_unique<SCCInfo>(node->getT()));
      }
    }
  };
}