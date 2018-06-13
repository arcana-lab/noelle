#include "DSWPLoopDependenceInfo.hpp"

using namespace std;
using namespace llvm;

llvm::DSWPLoopDependenceInfo::DSWPLoopDependenceInfo (Function *f, PDG *fG, Loop *l, LoopInfo &li, PostDominatorTree &pdt)
		: LoopDependenceInfo(f, fG, l, li, pdt), nextPartitionID{0} {

  return ;
};

llvm::DSWPLoopDependenceInfo::~DSWPLoopDependenceInfo() {

  return ;
}
