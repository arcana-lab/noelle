#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "LoopDependenceInfo.hpp"

using namespace std;
using namespace llvm;

llvm::LoopDependenceInfo::LoopDependenceInfo(Function *f, PDG *fG, Loop *l, LoopInfo &li, DominatorTree &dt, PostDominatorTree &pdt, ScalarEvolution &se)
		: function{f}, LI{li}, DT{dt}, PDT{pdt}, SE{se}, loop{l}, functionDG{fG} {
	loopDG = functionDG->createLoopsSubgraph(LI);

	/*
	 * Build a SCCDAG of loop-internal instructions
	 */
	std::vector<Value *> loopInternals;
	for (auto internalNode : loopDG->internalNodePairs()) loopInternals.push_back(internalNode.first);
	loopInternalDG = loopDG->createSubgraphFromValues(loopInternals, false);
	loopSCCDAG = SCCDAG::createSCCDAGFrom(loopInternalDG);
};

llvm::LoopDependenceInfo::~LoopDependenceInfo() {
	delete loopDG;
	delete loopInternalDG;
	delete loopSCCDAG;
}
