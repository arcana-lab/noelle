/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
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
