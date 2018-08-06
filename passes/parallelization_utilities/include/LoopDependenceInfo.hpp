#pragma once

#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "LoopInfoSummary.hpp"
#include "SCCDAGAttrs.hpp"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace llvm;

namespace llvm {

  class LoopDependenceInfo {
   public:
    LoopInfoSummary liSummary;
    Function *function;
    BasicBlock *header;
    BasicBlock *preHeader;
    std::vector<BasicBlock *> loopBBs;
    unordered_map<BasicBlock *, BasicBlock *> loopBBtoPD;
    PDG *functionDG;
    PDG *loopDG;
    PDG *loopInternalDG;
    SCCDAG *loopSCCDAG;
    SCCDAGAttrs sccdagAttrs;
    SmallVector<BasicBlock *, 10> loopExitBlocks;

    LoopDependenceInfo(Function *f, PDG *fG, Loop *l, LoopInfo &li, PostDominatorTree &pdt);
    ~LoopDependenceInfo();
  };
}
