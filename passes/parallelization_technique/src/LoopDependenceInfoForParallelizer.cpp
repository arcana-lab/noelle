#include "LoopDependenceInfoForParallelizer.hpp"

using namespace std;
using namespace llvm;

llvm::LoopDependenceInfoForParallelizer::LoopDependenceInfoForParallelizer (Function *f, PDG *fG, Loop *l, LoopInfo &li, PostDominatorTree &pdt)
		: LoopDependenceInfo(f, fG, l, li, pdt) {

  return ;
};

llvm::LoopDependenceInfoForParallelizer::~LoopDependenceInfoForParallelizer() {

  return ;
}
