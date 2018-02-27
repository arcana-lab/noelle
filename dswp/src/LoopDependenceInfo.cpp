#include "PDG.hpp"
#include "SCCDG.hpp"
#include "llvm/Analysis/LoopInfo.h"

#include "../include/LoopDependenceInfo.hpp"

using namespace std;
using namespace llvm;

llvm::LoopDependenceInfo::LoopDependenceInfo(Function *f, LoopInfo &li, Loop *l, PDG *dg) : func{f}, LI{li}, loop{l}, loopDG{dg} {
	sccDG = SCCDG::createSCCGraphFrom(loopDG);
};
