/*
 * Copyright 2021 Simone Campanoni
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
#include "noelle/core/LoopTransformer.hpp"
#include "noelle/core/Scheduler.hpp"
#include "noelle/core/LoopWhilify.hpp"
#include "noelle/core/LoopUnroll.hpp"
#include "noelle/core/LoopDistribution.hpp"

namespace llvm::noelle {

LoopTransformer::LoopTransformer() : ModulePass{ ID } {
  return;
}

void LoopTransformer::setPDG(PDG *programDependenceGraph) {
  this->pdg = programDependenceGraph;
  assert(this->pdg != nullptr);

  return;
}

LoopUnrollResult LoopTransformer::unrollLoop(LoopDependenceInfo *loop,
                                             uint32_t unrollFactor) {

  /*
   * Fetch the function that contains the loop we want to unroll.
   */
  auto ls = loop->getLoopStructure();
  auto lsFunction = ls->getFunction();

  /*
   * Fetch the LLVM loop abstractions.
   */
  auto &LLVMLoops = getAnalysis<LoopInfoWrapperPass>(*lsFunction).getLoopInfo();
  auto &DT = getAnalysis<DominatorTreeWrapperPass>(*lsFunction).getDomTree();
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*lsFunction).getSE();
  auto &AC =
      getAnalysis<AssumptionCacheTracker>().getAssumptionCache(*lsFunction);

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
  opts.Force = false;
  opts.Runtime = false;
  opts.AllowExpensiveTripCount = true;
  opts.UnrollRemainder = false;
  opts.ForgetAllSCEV = true;
  OptimizationRemarkEmitter ORE(lsFunction);
  TargetTransformInfo TTI(lsFunction->getParent()->getDataLayout());
  auto unrolled =
      UnrollLoop(llvmLoop, opts, &LLVMLoops, &SE, &DT, &AC, &TTI, &ORE, true);

  return unrolled;
}

bool LoopTransformer::fullyUnrollLoop(LoopDependenceInfo *loop) {

  /*
   * Fetch the unroller
   */
  auto loopUnroll = LoopUnroll();

  /*
   * Fetch the function
   */
  auto ls = loop->getLoopStructure();
  auto &loopFunction = *ls->getFunction();
  auto &LS = getAnalysis<LoopInfoWrapperPass>(loopFunction).getLoopInfo();
  auto &DT = getAnalysis<DominatorTreeWrapperPass>(loopFunction).getDomTree();
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(loopFunction).getSE();
  auto &AC =
      getAnalysis<AssumptionCacheTracker>().getAssumptionCache(loopFunction);
  auto modified = loopUnroll.fullyUnrollLoop(*loop, LS, DT, SE, AC);

  return modified;
}

bool LoopTransformer::whilifyLoop(LoopDependenceInfo *loop) {
  assert(this->pdg != nullptr);

  /*
   * Allocate the whilifier
   */
  auto loopWhilify = LoopWhilifier();

  /*
   * Get the necessary information
   */
  auto scheduler = Scheduler();
  auto loopStructure = loop->getLoopStructure();
  auto func = loopStructure->getFunction();
  auto &DT = getAnalysis<DominatorTreeWrapperPass>(*func).getDomTree();
  auto &PDT = getAnalysis<PostDominatorTreeWrapperPass>(*func).getPostDomTree();
  auto DS = new DominatorSummary(DT, PDT);
  auto FDG = this->pdg->createFunctionSubgraph(*func);

  /*
   * Whilify the loop.
   */
  auto modified = loopWhilify.whilifyLoop(*loop, scheduler, DS, FDG);

  return modified;
}

LoopTransformer::~LoopTransformer() {
  return;
}

bool LoopTransformer::splitLoop(LoopDependenceInfo *loop,
                                std::set<SCC *> const &SCCsToPullOut,
                                std::set<Instruction *> &instructionsRemoved,
                                std::set<Instruction *> &instructionsAdded) {

  /*
   * Check trivial cases
   */
  if (loop == nullptr) {
    return false;
  }

  /*
   * Split the loop.
   */
  LoopDistribution ld;
  auto modified = ld.splitLoop(*loop,
                               SCCsToPullOut,
                               instructionsRemoved,
                               instructionsAdded);

  return modified;
}

} // namespace llvm::noelle
