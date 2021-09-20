/*
 * Copyright 2021 Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopTransformer.hpp"

namespace llvm::noelle {

LoopTransformer::LoopTransformer()
  : FunctionPass{ID}
{
  return;
}

LoopUnrollResult LoopTransformer::unrollLoop (LoopDependenceInfo *loop, uint32_t unrollFactor){

  /*
   * Fetch the function that contains the loop we want to unroll.
   */
  auto ls = loop->getLoopStructure();
  auto lsFunction = ls->getFunction();

  /*
   * Fetch the trip count.
   */
  auto loopTripCount = (uint32_t)loop->getCompileTimeTripCount();

  /*
   * Fetch the LLVM loop abstractions.
   */
  auto& LLVMLoops = getAnalysis<LoopInfoWrapperPass>(*lsFunction).getLoopInfo();
  auto& DT = getAnalysis<DominatorTreeWrapperPass>(*lsFunction).getDomTree();
  auto& SE = getAnalysis<ScalarEvolutionWrapperPass>(*lsFunction).getSE();
  auto& AC = getAnalysis<AssumptionCacheTracker>().getAssumptionCache(*lsFunction);

  /*
   * Fetch the LLVM loop.
   */
  auto h = ls->getHeader();
  auto llvmLoop = LLVMLoops.getLoopFor(h);
  assert(llvmLoop != nullptr);

  /*
   * Try to unroll the loop
   */
  UnrollLoopOptions opts;
  opts.Count = unrollFactor;
  opts.TripCount = loopTripCount; 
  opts.Force = false;
  opts.AllowRuntime = false;
  opts.AllowExpensiveTripCount = true;
  opts.PreserveCondBr = false;
  opts.TripMultiple = SE.getSmallConstantTripMultiple(llvmLoop);
  opts.PeelCount = 0;
  opts.UnrollRemainder = false;
  opts.ForgetAllSCEV = true;
  OptimizationRemarkEmitter ORE(lsFunction);
  auto unrolled = UnrollLoop(
    llvmLoop, opts, 
    &LLVMLoops, &SE, &DT, &AC, &ORE, 
    true);

  return unrolled;
}

LoopTransformer::~LoopTransformer() {
  return;
}

}
