#pragma once

#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"

#include "LoopDependenceInfo.hpp"
#include "PipelineInfo.hpp"

using namespace std;
using namespace llvm;

namespace llvm {

	class DSWPLoopDependenceInfo : public LoopDependenceInfo {
      public:

		/*
		 * Stores new pipeline execution
		 */
		BasicBlock *pipelineBB;
		std::set<SCC *> scalarSCCs;
		unordered_map<SCC *, StageInfo *> sccToStage;
		std::vector<std::unique_ptr<StageInfo>> stages;
		std::vector<std::unique_ptr<QueueInfo>> queues;
		std::unique_ptr<EnvInfo> environment;

		/*
		 * Types for arrays storing dependencies and stages
		 */
		ArrayType *envArrayType, *queueArrayType, *stageArrayType;
		Value *zeroIndexForBaseArray;

		DSWPLoopDependenceInfo(Function *f, PDG *fG, Loop *l, LoopInfo &li);

		~DSWPLoopDependenceInfo();
	};
}
