/*
 * Copyright 2019 - 2020  Simone Campanoni
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
#include "arcana/noelle/core/LoopUnroll.hpp"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"

namespace arcana::noelle {

LoopUnroll::LoopUnroll() {
  return;
}

bool LoopUnroll::fullyUnrollLoop(LoopContent const &LC,
                                 LoopInfo &LI,
                                 DominatorTree &DT,
                                 ScalarEvolution &SE,
                                 AssumptionCache &AC) {
  auto modified = false;

  /*
   * Fetch the loop summary
   */
  auto ls = LC.getLoopStructure();

  /*
   * Check if the loop has compile time known trip count.
   */
  if (!LC.doesHaveCompileTimeKnownTripCount()) {
    return false;
  }

  /*
   * Fetch the function that includes the loop.
   */
  auto loopFunction = ls->getFunction();

  /*
   * Fetch the LLVM loop.
   */
  auto h = ls->getHeader();
  auto llvmLoop = LI.getLoopFor(h);

  /*
   * Fetch the trip count.
   */
  auto tripCount = LC.getCompileTimeTripCount();

  /*
   * Try to unroll the loop
   */
  UnrollLoopOptions opts;
  opts.Count = tripCount;
  opts.Force = false;
  opts.Runtime = false;
  opts.AllowExpensiveTripCount = true;
  opts.UnrollRemainder = false;
  opts.ForgetAllSCEV = false;
  llvm::OptimizationRemarkEmitter ORE(loopFunction);
  llvm::TargetTransformInfo TTI(loopFunction->getParent()->getDataLayout());
  llvm::LoopUnrollResult unrolled =
      UnrollLoop(llvmLoop, opts, &LI, &SE, &DT, &AC, &TTI, &ORE, true);

  /*
   * Check if the loop unrolled.
   */
  switch (unrolled) {
    case llvm::LoopUnrollResult::FullyUnrolled:
      errs() << "   Fully unrolled\n";
      modified = true;
      break;

    case llvm::LoopUnrollResult::PartiallyUnrolled:
      errs() << "   Partially unrolled\n";
      abort();

    case llvm::LoopUnrollResult::Unmodified:
      errs() << "   Not unrolled\n";
      modified = false;
      break;

    default:
      abort();
  }

  return modified;
}

} // namespace arcana::noelle
