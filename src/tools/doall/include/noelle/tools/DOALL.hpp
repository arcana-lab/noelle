/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
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
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/LoopDependenceInfo.hpp"
#include "noelle/core/PDG.hpp"
#include "noelle/core/SCC.hpp"
#include "noelle/core/SCCDAG.hpp"
#include "noelle/core/PDGAnalysis.hpp"
#include "noelle/core/Noelle.hpp"
#include "noelle/core/IVStepperUtility.hpp"
#include "noelle/tools/ParallelizationTechnique.hpp"
#include "HeuristicsPass.hpp"

namespace llvm::noelle {

class DOALL : public ParallelizationTechnique {
public:
  /*
   * Methods
   */
  DOALL(Noelle &noelle);

  bool apply(LoopDependenceInfo *LDI, Heuristics *h) override;

  bool canBeAppliedToLoop(LoopDependenceInfo *LDI,
                          Heuristics *h) const override;

  uint32_t getMinimumNumberOfIdleCores(void) const override;

  std::string getName(void) const override;

  static std::set<SCC *> getSCCsThatBlockDOALLToBeApplicable(
      LoopDependenceInfo *LDI,
      Noelle &par);

protected:
  bool enabled;
  Function *taskDispatcher;
  Noelle &n;
  std::map<PHINode *, std::set<Instruction *>> IVValueJustBeforeEnteringBody;

  virtual void invokeParallelizedLoop(LoopDependenceInfo *LDI);

  /*
   * DOALL specific generation
   */
  void rewireLoopToIterateChunks(LoopDependenceInfo *LDI);

  void addJumpToLoop(LoopDependenceInfo *LDI, Task *t);

  /*
   * Helpers
   */
  Value *fetchClone(Value *original) const;

  /*
   * Interface
   */
  BasicBlock *getBasicBlockExecutedOnlyByLastIterationBeforeExitingTask(
      LoopDependenceInfo *LDI,
      uint32_t taskIndex,
      BasicBlock &bb) override;
};

} // namespace llvm::noelle
