#pragma once

#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"

#include "LoopDependenceInfo.hpp"
#include "SCCDAGPartition.hpp"

using namespace std;
using namespace llvm;

namespace llvm {

  class LoopDependenceInfoForParallelizer : public LoopDependenceInfo {
    public:

      /*
       * Stores characterizations of Loop SCCDAG
       */
      SCCDAGPartition partition;
      std::set<SCC *> singleInstrNodes;

      LoopDependenceInfoForParallelizer(Function *f, PDG *fG, Loop *l, LoopInfo &li, PostDominatorTree &pdt);

      ~LoopDependenceInfoForParallelizer();
  };
}
