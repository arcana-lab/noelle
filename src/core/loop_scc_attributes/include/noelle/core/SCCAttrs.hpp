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
#include "noelle/core/SCC.hpp"
#include "noelle/core/LoopStructure.hpp"
#include "noelle/core/MemoryCloningAnalysis.hpp"

namespace llvm::noelle {

class SCCAttrs {
public:
  /*
   * Concrete sub-classes.
   */
  enum SCCKind {
    LOOP_CARRIED,
    REDUCTION,
    BINARY_REDUCTION,
    LAST_REDUCTION,
    LAST_LOOP_CARRIED,
    LOOP_ITERATION,
    LAST_LOOP_ITERATION
  };

  /*
   * Iterators.
   */
  typedef typename std::set<PHINode *>::iterator phi_iterator;
  typedef typename std::set<Instruction *>::iterator instruction_iterator;

  /*
   * Fields
   */
  std::set<Value *> stronglyConnectedDataValues;
  std::set<Value *> weaklyConnectedDataValues;

  /*
   * No public constructors.
   * Only objects of sub-classes can be allocated.
   */
  SCCAttrs() = delete;

  /*
   * Get the SCC.
   */
  SCC *getSCC(void);

  /*
   * Return true if it is safe to clone the SCC.
   * Return false otherwise.
   *
   * TODO: Break apart into two separate APIs:
   * canBeDirectlyClonedAndRepeatableEachIteration
   * canBeDirectlyClonedButExecutedOnlyOnceEachIteration
   */
  bool canBeCloned(void) const;

  /*
   * Return true if cloning is possible through memory AllocaInst cloning
   */
  bool canBeClonedUsingLocalMemoryLocations(void) const;

  /*
   * Return true if the SCC exists because of updates of an induction variable.
   * Return false otherwise.
   */
  bool isInductionVariableSCC(void) const;

  /*
   * Return true if the SCC is commutative
   * Return false otherwise.
   */
  bool isCommutative(void) const;

  /*
   * Get the PHIs.
   */
  iterator_range<phi_iterator> getPHIs(void) const;

  /*
   * Return the single header PHI if it exists. nullptr otherwise.
   */
  PHINode *getSingleHeaderPHI(void);

  /*
   * Return the memory locations that can be safely clone to void reusing the
   * same memory locations between invocations of this SCC.
   */
  std::unordered_set<AllocaInst *> getMemoryLocationsToClone(void) const;

  /*
   * Set the SCC as created by updated of an induction variable.
   */
  void setSCCToBeInductionVariable(bool hasIV = true);

  /*
   * Set the SCC to be clonable.
   */
  void setSCCToBeClonable(bool isClonable = true);

  void setSCCToBeClonableUsingLocalMemory(void);

  void addClonableMemoryLocationsContainedInSCC(
      std::unordered_set<const ClonableMemoryLocation *> locations);

  SCCKind getKind(void) const;

  ~SCCAttrs();

protected:
  LoopStructure *loop;
  SCC *scc;
  std::set<BasicBlock *> bbs;
  std::set<Instruction *> controlFlowInsts;
  std::set<PHINode *> PHINodes;
  std::set<PHINode *> headerPHINodes;

  SCCAttrs(SCCKind K, SCC *s, LoopStructure *loop);
  std::unordered_set<const ClonableMemoryLocation *> clonableMemoryLocations;
  bool isSCCClonableIntoLocalMemory;

  bool isClonable;
  bool hasIV;
  bool commutative;

  void collectPHIs(LoopStructure &LS);
  void collectControlFlowInstructions(void);

private:
  SCCKind kind;
};

} // namespace llvm::noelle
