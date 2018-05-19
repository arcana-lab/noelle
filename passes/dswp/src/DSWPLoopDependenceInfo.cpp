#include "DSWPLoopDependenceInfo.hpp"

using namespace std;
using namespace llvm;

llvm::DSWPLoopDependenceInfo::DSWPLoopDependenceInfo (Function *f, PDG *fG, Loop *l, LoopInfo &li, DominatorTree &dt, PostDominatorTree &pdt, ScalarEvolution &se)
		: LoopDependenceInfo(f, fG, l, li, dt, pdt, se) {

  return ;
};

llvm::DSWPLoopDependenceInfo::~DSWPLoopDependenceInfo() {

  return ;
}
