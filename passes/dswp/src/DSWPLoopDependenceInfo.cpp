#include "DSWPLoopDependenceInfo.hpp"

using namespace std;
using namespace llvm;

llvm::DSWPLoopDependenceInfo::DSWPLoopDependenceInfo (Function *f, PDG *fG, Loop *l, LoopInfo &li)
		: LoopDependenceInfo(f, fG, l, li) {

  return ;
};

llvm::DSWPLoopDependenceInfo::~DSWPLoopDependenceInfo() {

  return ;
}
