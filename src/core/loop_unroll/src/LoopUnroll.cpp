/*
 * Copyright 2019 - 2020  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopUnroll.hpp"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"

using namespace llvm;
using namespace llvm::noelle;
 
bool LoopUnroll::fullyUnrollLoop (
  LoopDependenceInfo const &LDI,
  LoopInfo &LI,
  DominatorTree &DT,
  ScalarEvolution &SE,
  AssumptionCache &AC
  ){
  auto modified = false;

  /*
   * Fetch the loop summary
   */
  auto ls = LDI.getLoopStructure();

  /*
   * Check if the loop has compile time known trip count.
   */
  if (!LDI.doesHaveCompileTimeKnownTripCount()){
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
  auto tripCount = LDI.getCompileTimeTripCount();

  /*
   * Try to unroll the loop
   */
  UnrollLoopOptions opts;
  opts.Count = tripCount;
  opts.TripCount = tripCount;
  opts.Force = false;
  opts.AllowRuntime = false;
  opts.AllowExpensiveTripCount = true;
  opts.PreserveCondBr = false;
  opts.TripMultiple = SE.getSmallConstantTripMultiple(llvmLoop);
  opts.PeelCount = 0;
  opts.UnrollRemainder = false;
  opts.ForgetAllSCEV = false;
  OptimizationRemarkEmitter ORE(loopFunction);
  auto unrolled = UnrollLoop(
    llvmLoop, opts, 
    &LI, &SE, &DT, &AC, &ORE, 
    true);

  /*
   * Check if the loop unrolled.
   */
  switch (unrolled){
    case LoopUnrollResult::FullyUnrolled :
      errs() << "   Fully unrolled\n";
      modified = true;
      break ;

    case LoopUnrollResult::PartiallyUnrolled :
      errs() << "   Partially unrolled\n";
      abort();

    case LoopUnrollResult::Unmodified :
      errs() << "   Not unrolled\n";
      modified = false;
      break ;

    default:
      abort();
  }

  return modified;
}
