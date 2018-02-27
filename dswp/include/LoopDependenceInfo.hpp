#pragma once

#include "PDG.hpp"
#include "SCCDG.hpp"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace llvm;

namespace llvm {

	struct LoopDependenceInfo {
		LoopDependenceInfo(Function *f, LoopInfo &li, Loop *l, PDG *dg);

		Function *func;
		LoopInfo &LI;
		Loop *loop;
		PDG *loopDG;
		SCCDG *sccDG;
	};
}