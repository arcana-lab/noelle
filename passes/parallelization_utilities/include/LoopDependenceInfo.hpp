#pragma once

#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "LoopInfoSummary.hpp"
#include "SCCDAGAttrs.hpp"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "LoopEnvironment.hpp"

using namespace std;

namespace llvm {

  class LoopDependenceInfo {
   public:
    LoopInfoSummary liSummary;

    /*
     * Context
     */
    Function *function;
    LoopEnvironment *environment;

    /*
     * Loop entry and exit points.
     */
    BasicBlock *header;
    BasicBlock *preHeader;
    SmallVector<BasicBlock *, 10> loopExitBlocks;

    /*
     * Loop
     */
    std::vector<BasicBlock *> loopBBs;
    unordered_map<BasicBlock *, BasicBlock *> loopBBtoPD;

    /*
     * Dependences
     */
    PDG *functionDG;
    PDG *loopDG;
    PDG *loopInternalDG;

    /*
     * SCCDAG.
     */
    // REFACTOR(angelo): rename loopSCCDAG to loopInternalSCCDAG
    SCCDAG *loopSCCDAG;
    SCCDAGAttrs sccdagAttrs;

    /*
     * Parallelized loop
     */
    Value *envArray;
    ArrayType *envArrayType;
    BasicBlock *entryPointOfParallelizedLoop;
    BasicBlock *exitPointOfParallelizedLoop;

    /*
     * Methods
     */
    LoopDependenceInfo (Function *f, PDG *fG, Loop *l, LoopInfo &li, PostDominatorTree &pdt);
    virtual void createPDGs (void) ;
    ~LoopDependenceInfo();
  };

}
