#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "LoopDependenceInfo.hpp"

using namespace std;
using namespace llvm;

llvm::LoopDependenceInfo::LoopDependenceInfo(Function *f, LoopInfo &li, DominatorTree &dt, ScalarEvolution &se, Loop *l, PDG *fG, std::vector<Instruction *> bodyInst, std::vector<Instruction *> otherInst)
		: func{f}, LI{li}, DT{dt}, SE{se}, loop{l}, functionDG{fG}, bodyInstOfLoop{bodyInst}, otherInstOfLoop{otherInst} {
	loopDG = functionDG->createLoopsSubgraph(LI);
	std::vector<Value*> bodyVals(bodyInst.begin(), bodyInst.end()); 
	for (auto bodyI : bodyInst)
		{ bodyI->print(errs()); errs() << "\n"; }
	loopBodyDG = loopDG->createSubgraphFromValues(bodyVals);
	loopBodySCCDAG = SCCDAG::createSCCDAGFrom(loopBodyDG);
};

llvm::LoopDependenceInfo::~LoopDependenceInfo() {
	delete loopDG;
	delete loopBodyDG;
	delete loopBodySCCDAG;
}
