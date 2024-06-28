/*
 * Copyright 2021 - 2024  Simone Campanoni
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
#include "arcana/noelle/core/LoopTransformer.hpp"
#include "arcana/noelle/core/Scheduler.hpp"
#include "arcana/noelle/core/LoopWhilify.hpp"
#include "arcana/noelle/core/LoopUnroll.hpp"
#include "arcana/noelle/core/LoopDistribution.hpp"

namespace arcana::noelle {

LoopTransformer::LoopTransformer(
    std::function<llvm::ScalarEvolution &(Function &F)> getSCEV,
    std::function<llvm::LoopInfo &(Function &F)> getLoopInfo,
    std::function<llvm::PostDominatorTree &(Function &F)> getPDT,
    std::function<llvm::DominatorTree &(Function &F)> getDT,
    std::function<llvm::AssumptionCache &(Function &F)> getAssumptionCache)
  : getSCEV{ getSCEV },
    getLoopInfo{ getLoopInfo },
    getPDT{ getPDT },
    getDT{ getDT },
    getAssumptionCache{ getAssumptionCache } {
  return;
}

void LoopTransformer::setPDG(PDG *programDependenceGraph) {
  this->pdg = programDependenceGraph;
  assert(this->pdg != nullptr);

  return;
}

LoopUnrollResult LoopTransformer::unrollLoop(LoopContent *loop,
                                             uint32_t unrollFactor) {

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
  auto &LLVMLoops = this->getLoopInfo(*lsFunction);
  auto &DT = this->getDT(*lsFunction);
  auto &SE = this->getSCEV(*lsFunction);
  auto &AC = this->getAssumptionCache(*lsFunction);

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
  auto unrolled =
      UnrollLoop(llvmLoop, opts, &LLVMLoops, &SE, &DT, &AC, &ORE, true);

  return unrolled;
}

bool LoopTransformer::fullyUnrollLoop(LoopContent *loop) {

  /*
   * Fetch the unroller
   */
  auto loopUnroll = LoopUnroll();

  /*
   * Fetch the function
   */
  auto ls = loop->getLoopStructure();
  auto &loopFunction = *ls->getFunction();
  auto &LS = this->getLoopInfo(loopFunction);
  auto &DT = this->getDT(loopFunction);
  auto &SE = this->getSCEV(loopFunction);
  auto &AC = this->getAssumptionCache(loopFunction);
  auto modified = loopUnroll.fullyUnrollLoop(*loop, LS, DT, SE, AC);

  return modified;
}

bool LoopTransformer::whilifyLoop(LoopContent *loop) {
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
  auto &DT = this->getDT(*func);
  auto &PDT = this->getPDT(*func);
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

bool LoopTransformer::splitLoop(LoopContent *loop,
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

} // namespace arcana::noelle
