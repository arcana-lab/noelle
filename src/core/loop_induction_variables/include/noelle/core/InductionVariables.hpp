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

#include "llvm/Analysis/IVDescriptors.h"
#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/ScalarEvolutionReferencer.hpp"
#include "noelle/core/LoopForest.hpp"
#include "noelle/core/LoopEnvironment.hpp"
#include "noelle/core/Dominators.hpp"
#include "noelle/core/SCCDAG.hpp"
#include "noelle/core/SCC.hpp"
#include "noelle/core/Invariants.hpp"
#include "noelle/core/InductionVariable.hpp"
#include "noelle/core/LoopGoverningInductionVariable.hpp"

namespace arcana::noelle {

class InductionVariableManager {
public:
  InductionVariableManager(LoopTree *loop,
                           InvariantManager &IVM,
                           ScalarEvolution &SE,
                           SCCDAG &sccdag,
                           LoopEnvironment &loopEnv,
                           Loop &LLVMLoop);

  InductionVariableManager() = delete;

  /*
   * Return all induction variables including the loop-governing one of the
   * outermost loop of the loop sub-tree related to @this.
   */
  std::unordered_set<InductionVariable *> getInductionVariables(void) const;

  /*
   * Return all induction variables including the loop-governing one.
   */
  std::unordered_set<InductionVariable *> getInductionVariables(
      LoopStructure &LS) const;

  /*
   * Return all induction variables that @i is involved in for any loop/sub-loop
   * related to this manager.
   */
  std::unordered_set<InductionVariable *> getInductionVariables(
      Instruction *i) const;

  InductionVariable *getInductionVariable(LoopStructure &LS,
                                          Instruction *i) const;

  bool doesContributeToComputeAnInductionVariable(Instruction *i) const;

  LoopGoverningInductionVariable *getLoopGoverningInductionVariable(void) const;

  LoopGoverningInductionVariable *getLoopGoverningInductionVariable(
      LoopStructure &LS) const;

  InductionVariable *getDerivingInductionVariable(
      LoopStructure &LS,
      Instruction *derivedInstruction) const;

  ~InductionVariableManager();

private:
  LoopTree *loop;
  std::unordered_map<LoopStructure *, std::unordered_set<InductionVariable *>>
      loopToIVsMap;
  std::unordered_map<LoopStructure *, LoopGoverningInductionVariable *>
      loopToGoverningIVAttrMap;
};

} // namespace arcana::noelle
