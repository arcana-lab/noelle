#pragma once

#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"

#include "LoopDependenceInfo.hpp"
#include "PipelineInfo.hpp"
#include "SCCDAGPartition.hpp"

using namespace std;
using namespace llvm;

namespace llvm {

  class DSWPLoopDependenceInfo : public LoopDependenceInfo {
    public:

      /*
       * Stores characterizations of Loop SCCDAG
       */
      SCCDAGPartition partition;
      std::set<SCC *> singleInstrNodes;

      /*
       * Stores new pipeline execution
       */
      unordered_map<SCC *, StageInfo *> sccToStage;
      std::vector<std::unique_ptr<StageInfo>> stages;
      std::vector<std::unique_ptr<QueueInfo>> queues;

      /*
       * Types for arrays storing dependencies and stages
       */
      ArrayType *queueArrayType;
      ArrayType *stageArrayType;
      Value *zeroIndexForBaseArray;

      DSWPLoopDependenceInfo(Function *f, PDG *fG, Loop *l, LoopInfo &li, PostDominatorTree &pdt);

      ~DSWPLoopDependenceInfo();
  };
}
