#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "LoopDependenceInfo.hpp"

using namespace std;
using namespace llvm;

llvm::LoopDependenceInfo::LoopDependenceInfo(Function *f, PDG *fG, Loop *l, LoopInfo &li, DominatorTree &dt, ScalarEvolution &se)
		: func{f}, LI{li}, DT{dt}, SE{se}, loop{l}, functionDG{fG} {
	loopDG = functionDG->createLoopsSubgraph(LI);
	loopSCCDAG = SCCDAG::createSCCDAGFrom(loopDG);
};

llvm::LoopDependenceInfo::~LoopDependenceInfo() {
	delete loopDG;
	delete loopSCCDAG;
}
