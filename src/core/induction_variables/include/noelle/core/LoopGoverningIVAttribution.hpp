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

#include "noelle/core/InductionVariables.hpp"
#include "noelle/core/SCC.hpp"
#include "noelle/core/SystemHeaders.hpp"

namespace llvm::noelle {

class LoopGoverningIVAttribution {
public:
  LoopGoverningIVAttribution(LoopStructure *l,
                             InductionVariable &IV,
                             SCC &scc,
                             std::vector<BasicBlock *> &exitBlocks);

  LoopGoverningIVAttribution() = delete;

  InductionVariable &getInductionVariable(void) const;

  CmpInst *getHeaderCompareInstructionToComputeExitCondition(void) const;

  Value *getExitConditionValue(void) const;

  BranchInst *getHeaderBrInst(void) const;

  BasicBlock *getExitBlockFromHeader(void) const;

  bool valueOfExitConditionToJumpToTheLoopBody(void) const;

  bool isSCCContainingIVWellFormed(void) const;

  std::set<Instruction *> getConditionValueDerivation(void) const;

  Instruction *getValueToCompareAgainstExitConditionValue(void) const;

private:
  LoopStructure *loop;
  InductionVariable &IV;
  SCC &scc;
  std::set<Instruction *> conditionValueDerivation;
  Value *conditionValue;
  Instruction *intermediateValueUsedInCompare;
  CmpInst *headerCmp;
  BranchInst *headerBr;
  BasicBlock *exitBlock;
  bool isWellFormed;
};

} // namespace llvm::noelle
