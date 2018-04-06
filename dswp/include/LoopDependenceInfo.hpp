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
		SCCDG *loopBodySCCDG, *loopIterationSCCDG;
		
		/*
		 * Separates control flow instructions (within otherInstOfLoop) from the rest of the loop
		 */
		std::vector<Instruction *> bodyInstOfLoop;
		std::vector<Instruction *> otherInstOfLoop;

		/*
		 * Stores new pipeline execution
		 */
		BasicBlock *pipelineBB;

		/*
		 * Tracks Type of value used by dependents inside/outside of the loop
		 */
		std::vector<Type *> internalDependentTypes;
		std::vector<Type *> externalDependentTypes;

		/*
		 * Tracks byte lengths of the internal types stored above.
		 */
		std::vector<int> internalDependentByteLengths;

		/*
		 * Types for arrays storing dependencies and stages
		 */
		ArrayType *envArrayType, *queueArrayType, *stageArrayType;
		Value *zeroIndexForBaseArray;
	};
}