#pragma once

#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Support/raw_ostream.h"

#include "PipelineInfo.hpp"

using namespace std;
using namespace llvm;

namespace llvm {

	struct LoopDependenceInfo {
		LoopDependenceInfo(Function *f, PDG *fG, Loop *l, LoopInfo &li, DominatorTree &dt, PostDominatorTree &pdt, ScalarEvolution &se);
		~LoopDependenceInfo();

		Function *function;
		LoopInfo &LI;
		ScalarEvolution &SE;
		DominatorTree &DT;
		PostDominatorTree &PDT;
		Loop *loop;
		PDG *functionDG;
		PDG *loopDG;
		PDG *loopInternalDG;
		SCCDAG *loopSCCDAG;
		
		/*
		 * Stores new pipeline execution
		 */
		BasicBlock *pipelineBB;
		unordered_map<SCC *, StageInfo *> sccToStage;
		unordered_map<BasicBlock *, std::set<StageInfo *>> bbToStage;
		std::vector<std::unique_ptr<StageInfo>> stages;
		std::vector<std::unique_ptr<QueueInfo>> queues;
		std::unique_ptr<EnvInfo> environment;

		/*
		 * Types for arrays storing dependencies and stages
		 */
		ArrayType *envArrayType, *queueArrayType, *stageArrayType;
		Value *zeroIndexForBaseArray;
	};
}
