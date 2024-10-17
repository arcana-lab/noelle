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
#ifndef NOELLE_SRC_CORE_MEMORY_CLONING_ANALYSIS_CLONABLEMEMORYOBJECT_H_
#define NOELLE_SRC_CORE_MEMORY_CLONING_ANALYSIS_CLONABLEMEMORYOBJECT_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/PDG.hpp"
#include "arcana/noelle/core/SCCDAG.hpp"
#include "arcana/noelle/core/Invariants.hpp"
#include "arcana/noelle/core/Dominators.hpp"
#include "arcana/noelle/core/Lumberjack.hpp"

namespace arcana::noelle {

class ClonableMemoryObject {
public:
  ClonableMemoryObject(AllocaInst *allocation,
                       uint64_t sizeInBits,
                       LoopStructure *loop,
                       DominatorSummary &DS,
                       PDG *ldg);

  AllocaInst *getAllocation(void) const;

  std::unordered_set<Instruction *> getLoopInstructionsUsingLocation(
      void) const;

  std::unordered_set<Instruction *> getInstructionsUsingLocationOutsideLoop(
      void) const;

  bool isInstructionCastOrGEPOfLocation(Instruction *I) const;

  bool isInstructionStoringLocation(Instruction *I) const;

  bool isInstructionLoadingLocation(Instruction *I) const;

  bool mustAliasAMemoryLocationWithinObject(Value *ptr) const;

  bool isClonableLocation(void) const;

  bool doPrivateCopiesNeedToBeInitialized(void) const;

  uint64_t getSizeInBits(void) const;

  std::unordered_set<Instruction *> getPointersUsedToAccessObject(void) const;

  static bool isMemCpyInstrinsicCall(CallInst *call);

private:
  AllocaInst *allocation;
  Type *allocatedType;
  uint64_t sizeInBits;
  LoopStructure *loop;
  bool isClonable;
  bool isScopeWithinLoop;
  bool needInitialization;

  std::unordered_set<Instruction *> castsAndGEPs;
  std::unordered_set<Instruction *> storingInstructions;
  std::unordered_set<Instruction *> loadInstructions;
  std::unordered_set<Instruction *> nonStoringInstructions;

  Logger log;

  bool identifyStoresAndOtherUsers(LoopStructure *loop, DominatorSummary &DS);

  bool isThereAMemoryDependenceBetweenLoopIterations(
      LoopStructure *loop,
      AllocaInst *al,
      PDG *ldg,
      const std::unordered_set<Instruction *> &insts) const;

  bool isThereRAWThroughMemoryFromLoopToOutside(LoopStructure *loop,
                                                AllocaInst *al,
                                                PDG *ldg) const;

  bool isThereRAWThroughMemoryBetweenLoopIterations(LoopStructure *loop,
                                                    AllocaInst *al,
                                                    PDG *ldg) const;

  bool isThereRAWThroughMemoryBetweenLoopIterations(
      LoopStructure *loop,
      AllocaInst *al,
      PDG *ldg,
      const std::unordered_set<Instruction *> &insts) const;

  bool isThereRAWThroughMemoryFromLoopToOutside(
      LoopStructure *loop,
      AllocaInst *al,
      PDG *ldg,
      std::unordered_set<Instruction *> insts) const;

  bool isThereRAWThroughMemoryFromOutsideToLoop(LoopStructure *loop,
                                                AllocaInst *al,
                                                PDG *ldg) const;

  bool isThereRAWThroughMemoryFromOutsideToLoop(
      LoopStructure *loop,
      AllocaInst *al,
      PDG *ldg,
      std::unordered_set<Instruction *> insts) const;

  /*
   * A set of storing instructions that completely override the allocation's
   * values before any use it dominates gets to using the allocation
   */
  struct OverrideSet {
    BasicBlock *dominatingBlockOfNonStoringInsts;
    std::unordered_set<Instruction *> initialStoringInstructions;
    std::unordered_set<Instruction *> subsequentNonStoringInstructions;
  };
  std::unordered_set<std::unique_ptr<OverrideSet>> overrideSets;

  bool identifyInitialStoringInstructions(LoopStructure *loop,
                                          DominatorSummary &DS);

  bool areOverrideSetsFullyCoveringTheAllocationSpace(void) const;

  bool isOverrideSetFullyCoveringTheAllocationSpace(
      OverrideSet *overrideSet) const;

  void setObjectScope(AllocaInst *allocation,
                      LoopStructure *loop,
                      DominatorSummary &ds);
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_MEMORY_CLONING_ANALYSIS_CLONABLEMEMORYOBJECT_H_
