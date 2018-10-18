#include "DSWPLoopDependenceInfo.hpp"

using namespace std;
using namespace llvm;

llvm::DSWPLoopDependenceInfo::DSWPLoopDependenceInfo (Function *f, PDG *fG, Loop *l, LoopInfo &li, PostDominatorTree &pdt)
		: LoopDependenceInfoForParallelizer(f, fG, l, li, pdt) {

  return ;
};

llvm::DSWPLoopDependenceInfo::~DSWPLoopDependenceInfo() {

  return ;
}
