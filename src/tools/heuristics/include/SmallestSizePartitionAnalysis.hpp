/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef NOELLE_SRC_TOOLS_HEURISTICS_SMALLESTSIZEPARTITIONANALYSIS_H_
#define NOELLE_SRC_TOOLS_HEURISTICS_SMALLESTSIZEPARTITIONANALYSIS_H_
#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "noelle/core/SCC.hpp"
#include "noelle/core/SCCDAGPartition.hpp"
#include "noelle/core/SCCDAGAttrs.hpp"

#include "PartitionCostAnalysis.hpp"

using namespace std;

namespace arcana::noelle {

class SmallestSizePartitionAnalysis : public PartitionCostAnalysis {
public:
  SmallestSizePartitionAnalysis(
      InvocationLatency &IL,
      SCCDAGPartitioner &p,
      SCCDAGAttrs &attrs,
      int cores,
      std::function<bool(GenericSCC *scc)> canBeRematerialized,
      Verbosity v)
    : PartitionCostAnalysis{ IL, p, attrs, cores, canBeRematerialized, v } {};

  void checkIfShouldMerge(
      SCCSet *sA,
      SCCSet *sB,
      std::function<bool(GenericSCC *scc)> canBeRematerialized) override;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_TOOLS_HEURISTICS_SMALLESTSIZEPARTITIONANALYSIS_H_
