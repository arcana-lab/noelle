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
   * Types.
   */
  enum SCCType { SEQUENTIAL, INDEPENDENT };

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

  std::set<std::pair<Value *, Instruction *>> controlPairs;

  /*
   * Constructor
   */
  SCCAttrs(SCC *s, LoopStructure *loop);

  SCCAttrs() = delete;

  /*
   * Get the SCC.
   */
  SCC *getSCC(void);

  /*
   * Return the type of SCC.
   */
  SCCType getType(void) const;

  /*
   * Return true if the iterations of the SCC must execute sequentially.
   * Return false otherwise.
   */
  bool mustExecuteSequentially(void) const;

  /*
   * Return true if a reduction transformation can be applied to the SCC.
   * Return false otherwise.
   */
  virtual bool canExecuteReducibly(void) const;

  /*
   * Return true if the iterations of the SCC are independent between each
   * other. Return false otherwise.
   */
  bool canExecuteIndependently(void) const;

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
   * Check if the SCC contains a PHI instruction.
   */
  bool doesItContainThisPHI(PHINode *phi);

  /*
   * Return the single PHI if it exists. nullptr otherwise.
   */
  PHINode *getSinglePHI(void);

  /*
   * Return the single header PHI if it exists. nullptr otherwise.
   */
  PHINode *getSingleHeaderPHI(void);

  /*
   * Return the number of PHIs included in the SCC.
   */
  uint32_t numberOfPHIs(void);

  const std::pair<Value *, Instruction *>
      *getSingleInstructionThatControlLoopExit(void);

  /*
   * Return the memory locations that can be safely clone to void reusing the
   * same memory locations between invocations of this SCC.
   */
  std::unordered_set<AllocaInst *> getMemoryLocationsToClone(void) const;

  /*
   * Set the type of SCC.
   */
  void setType(SCCType t);

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

  ~SCCAttrs();

protected:
  LoopStructure *loop;
  SCC *scc;
  SCCType sccType;
  std::set<BasicBlock *> bbs;
  std::set<Instruction *> controlFlowInsts;
  std::set<PHINode *> PHINodes;
  std::set<PHINode *> headerPHINodes;

  std::unordered_set<const ClonableMemoryLocation *> clonableMemoryLocations;
  bool isSCCClonableIntoLocalMemory;

  bool isClonable;
  bool hasIV;
  bool commutative;

  void collectPHIs(LoopStructure &LS);
  void collectControlFlowInstructions(void);
};

} // namespace llvm::noelle
