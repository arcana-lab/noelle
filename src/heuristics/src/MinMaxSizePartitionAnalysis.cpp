/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "MinMaxSizePartitionAnalysis.hpp"

void llvm::MinMaxSizePartitionAnalysis::checkIfShouldMerge (SCCset *sA, SCCset *sB) {
  bool yieldsCycle = partition.mergeYieldsCycle(sA, sB);

  if (verbose >= Verbosity::Maximal) {
    std::string subsetStrs = partition.subsetStr(sA) + " " + partition.subsetStr(sB);
    errs() << prefix << "Checking: " << subsetStrs;
    if (yieldsCycle) errs() << "\n";
    errs() << " Is possible\n";
  }

  if (yieldsCycle) return ;

  auto current = subsetCost[sA] + subsetCost[sB];
  auto insts = subsetInstCount[sA] + subsetInstCount[sB];
  std::set<SCCset *> subsets = { sA, sB };
  uint64_t merge = IL.latencyPerInvocation(&dagAttrs, subsets);
  uint64_t lowered = current - merge;

  if (partition.getSubsets()->size() <= numCores) return ;

  if (verbose >= Verbosity::Maximal) {
    errs() << prefix << "Lowered cost: " << lowered
      << " Merged cost: " << merge
      << " Instruction count: " << insts << "\n";
  }

  /*
   * Only merge if it is the cheapest of the merges
   */
  if (merge > mergedSubsetCost) return ;

  /*
   * Only merge if it is the smallest of equally cost effective merges
   */
  if (merge == mergedSubsetCost && insts > instCount) return ;

  minSubsetA = sA;
  minSubsetB = sB;
  loweredCost = lowered;
  instCount = insts;
  mergedSubsetCost = merge;
};
