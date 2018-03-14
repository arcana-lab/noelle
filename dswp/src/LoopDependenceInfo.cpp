#include "PDG.hpp"
#include "SCCDG.hpp"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "../include/LoopDependenceInfo.hpp"

using namespace std;
using namespace llvm;

llvm::LoopDependenceInfo::LoopDependenceInfo(Function *f, LoopInfo &li, DominatorTree &dt, ScalarEvolution &se, Loop *l, PDG *fG, std::vector<Instruction *> bodyInst, std::vector<Instruction *> otherInst)
		: func{f}, LI{li}, DT{dt}, SE{se}, loop{l}, functionDG{fG}, bodyInstOfLoop{bodyInst}, otherInstOfLoop{otherInst} {
	loopDG = functionDG->createLoopsSubgraph(LI);
	loopBodyDG = loopDG->createInstListSubgraph(bodyInst);
	sccBodyDG = SCCDG::createSCCGraphFrom(loopBodyDG);
};

llvm::LoopDependenceInfo::~LoopDependenceInfo() {
	delete loopDG;
	delete loopBodyDG;
	delete sccBodyDG;
}