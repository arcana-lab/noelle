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
#include "Parallelization.hpp"
#include "InvocationLatency.hpp"

using namespace std;

namespace llvm {

  class PartitionCostAnalysis {
   public:
    PartitionCostAnalysis (
      InvocationLatency &IL,
      SCCDAGPartition &p,
      SCCDAGAttrs &,
      int numCores,
      Verbosity verbose
    );

    void traverseAllPartitionSubsets ();

    virtual void checkIfShouldMerge (SCCset *sA, SCCset *sB) = 0;

    void resetCandidateSubsetInfo ();

    bool mergeCandidateSubsets ();

    void printCandidate (raw_ostream &stream);

    const static std::string prefix;
   protected:

    InvocationLatency &IL;
    SCCDAGPartition &partition;
    SCCDAGAttrs &dagAttrs;
    int numCores;

    uint64_t totalCost;
    uint64_t totalInstCount;
    std::unordered_map<SCCset *, uint64_t> subsetCost;
    std::unordered_map<SCCset *, uint64_t> subsetInstCount;

    SCCset *minSubsetA;
    SCCset *minSubsetB;
    uint64_t loweredCost;
    uint64_t instCount;
    uint64_t mergedSubsetCost;

    Verbosity verbose;
  };

}
