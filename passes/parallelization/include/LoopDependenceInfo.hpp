#pragma once

#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace llvm;

namespace llvm {

	class LoopDependenceInfo {
      public:
		Function *function;
        BasicBlock *header;
        BasicBlock *preHeader;
        std::vector<BasicBlock *> loopBBs;
		PDG *functionDG;
		PDG *loopDG;
		PDG *loopInternalDG;
		SCCDAG *loopSCCDAG;
		SmallVector<BasicBlock *, 10> loopExitBlocks;
		std::set<ReturnInst *> loopReturnInsts;

		LoopDependenceInfo(Function *f, PDG *fG, Loop *l, LoopInfo &li, DominatorTree &dt, PostDominatorTree &pdt, ScalarEvolution &se);
		~LoopDependenceInfo();
	};
}
