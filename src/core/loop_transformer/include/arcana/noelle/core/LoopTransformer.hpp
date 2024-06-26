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
#ifndef NOELLE_SRC_CORE_LOOP_TRANSFORMER_LOOPTRANSFORMER_H_
#define NOELLE_SRC_CORE_LOOP_TRANSFORMER_LOOPTRANSFORMER_H_

#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/LoopContent.hpp"

namespace arcana::noelle {

class LoopTransformer {
public:
  LoopTransformer(
      std::function<llvm::ScalarEvolution &(Function &F)> getSCEV,
      std::function<llvm::LoopInfo &(Function &F)> getLoopInfo,
      std::function<llvm::PostDominatorTree &(Function &F)> getPDT,
      std::function<llvm::DominatorTree &(Function &F)> getDT,
      std::function<llvm::AssumptionCache &(Function &F)> getAssumptionCache);

  void setPDG(PDG *programDependenceGraph);

  LoopUnrollResult unrollLoop(LoopContent *loop, uint32_t unrollFactor);

  bool fullyUnrollLoop(LoopContent *loop);

  bool whilifyLoop(LoopContent *loop);

  bool splitLoop(LoopContent *loop,
                 std::set<SCC *> const &SCCsToPullOut,
                 std::set<Instruction *> &instructionsRemoved,
                 std::set<Instruction *> &instructionsAdded);

  virtual ~LoopTransformer();

private:
  PDG *pdg;
  std::function<llvm::ScalarEvolution &(Function &F)> getSCEV;
  std::function<llvm::LoopInfo &(Function &F)> getLoopInfo;
  std::function<llvm::PostDominatorTree &(Function &F)> getPDT;
  std::function<llvm::DominatorTree &(Function &F)> getDT;
  std::function<llvm::AssumptionCache &(Function &F)> getAssumptionCache;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_LOOP_TRANSFORMER_LOOPTRANSFORMER_H_
