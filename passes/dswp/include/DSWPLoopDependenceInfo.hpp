#pragma once

#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"

#include "LoopDependenceInfo.hpp"
#include "SCCDAGPartition.hpp"
#include "TaskExecutionDSWP.hpp"

using namespace std;
using namespace llvm;

namespace llvm {

  class DSWPLoopDependenceInfo : public LoopDependenceInfo {
    public:

      /*
       * Stores new pipeline execution
       */
      unordered_map<SCC *, DSWPTask *> sccToStage;
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
