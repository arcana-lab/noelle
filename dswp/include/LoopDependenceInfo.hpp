#pragma once

#include "PDG.hpp"
#include "SCCDG.hpp"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace llvm;

namespace llvm {

	struct LoopDependenceInfo {
		LoopDependenceInfo(Function *f, LoopInfo &li, DominatorTree &dt, ScalarEvolution &se, Loop *l, PDG *fG, std::vector<Instruction *> bodyInst, std::vector<Instruction *> otherInst);
		~LoopDependenceInfo();

		Function *func;
		LoopInfo &LI;
		ScalarEvolution &SE;
		DominatorTree &DT;
		Loop *loop;
		PDG *functionDG;
		PDG *loopDG;

		PDG *loopBodyDG;
		SCCDG *sccBodyDG, *loopIterationSCCDG;
		
		std::vector<Instruction *> bodyInstOfLoop;
		std::vector<Instruction *> otherInstOfLoop;
	};
}