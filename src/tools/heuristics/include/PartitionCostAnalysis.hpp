/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "SCC.hpp"
#include "SCCDAGPartition.hpp"
#include "Noelle.hpp"
#include "InvocationLatency.hpp"

using namespace std;

namespace llvm::noelle {

  class PartitionCostAnalysis {
   public:
    PartitionCostAnalysis (
      InvocationLatency &IL,
      SCCDAGPartitioner &p,
      SCCDAGAttrs &,
      int numCores,
      Verbosity verbose
    );

    void traverseAllPartitionSubsets ();

    virtual void checkIfShouldMerge (SCCSet *sA, SCCSet *sB) = 0;

    void resetCandidateSubsetInfo ();

    bool mergeCandidateSubsets ();

    void printCandidate (raw_ostream &stream);

    const static std::string prefix;
   protected:

    InvocationLatency &IL;
    SCCDAGPartitioner &partitioner;
    SCCDAGAttrs &dagAttrs;
    int numCores;

    std::unordered_map<SCC *, uint64_t> sccToInstructionCountMap;
    uint64_t costIfAllSetsRunOnSeparateCores;
    uint64_t totalInstructionCount;

    std::unordered_set<SCCSet *> minSetsToMerge;
    uint64_t numInstructionsInSetsBeingMerged;
    uint64_t savedCostByMerging;
    uint64_t costOfMergedSet;

    Verbosity verbose;
  };

}
