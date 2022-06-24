/*
 * Copyright 2019 - 2021  Souradip Ghosh, Simone Campanoni
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

#include "noelle/core/LoopDependenceInfo.hpp"
#include "noelle/core/Scheduler.hpp"
#include "noelle/core/SystemHeaders.hpp"

namespace llvm::noelle {

class WhilifierContext {

  /*
   * WhilifierContext --- TOP
   *
   * A package to share whilifier information across LoopWhilifier
   * member functions --- this is a shortcut over better engineering
   * because the LoopWhilifier does not keep any internal state.
   */

public:
  /*
   * Methods
   */
  WhilifierContext(LoopStructure *const LS);

  void Dump(void);

  /*
   * Context for loop body to whlify
   */
  BasicBlock *OriginalHeader;
  BasicBlock *OriginalPreHeader;
  BasicBlock *OriginalLatch;
  uint32_t NumLatches;
  std::vector<std::pair<BasicBlock *, BasicBlock *>> ExitEdges;
  std::vector<BasicBlock *> LoopBlocks;
  Function *F;

  /*
   * Context for whilification process
   */
  BasicBlock *TopAnchor;
  BasicBlock *BottomAnchor;
  ValueToValueMapTy BodyToPeelMap;
  SmallVector<BasicBlock *, 16> NewBlocks;

  /*
   * Analysis for whilification process
   */
  bool IsDoWhile = false;
  bool IsAppropriateToWhilify = false;
  bool IsSingleBlockLoop = false;
  bool ConsolidatedOriginalLatch = false;
  bool ResolvedLatch = false;
  DenseMap<PHINode *, Value *> ExitDependencies;
  DenseMap<Value *, Value *> ResolvedDependencyMapping;
  DenseMap<Instruction *, DenseMap<Instruction *, uint32_t>>
      OriginalLatchDependencies;
};

class LoopWhilifier {

public:
  /*
   * Methods
   */
  LoopWhilifier();

  bool whilifyLoop(LoopDependenceInfo &LDI,
                   Scheduler &scheduler,
                   DominatorSummary *DS,
                   PDG *FDG);

private:
  /*
   * Fields
   */
  std::string outputPrefix;

  /*
   * Methods
   */
  bool whilifyLoopDriver(LoopStructure *const LS,
                         Scheduler &scheduler,
                         DominatorSummary *DS,
                         PDG *FDG);

  bool containsInOriginalLoop(WhilifierContext const &WC, BasicBlock *const BB);

  void compressStructuralLatch(WhilifierContext &WC,
                               BasicBlock *&SemanticLatch);

  bool isSemanticLatch(WhilifierContext const &WC, BasicBlock *&LatchPred);

  bool isAppropriateToWhilify(WhilifierContext &WC,
                              BasicBlock *const SemanticLatch);

  bool isDoWhile(WhilifierContext &WC);

  bool canWhilify(WhilifierContext &WC);

  void transformSingleBlockLoop(WhilifierContext &WC);

  void buildAnchors(WhilifierContext &WC);

  void cloneLoopBlocksForWhilifying(WhilifierContext &WC);

  PHINode *buildNewHeaderDependencyPHI(WhilifierContext &WC, Value *Dependency);

  void resolveExitEdgeDependencies(WhilifierContext &WC, BasicBlock *NewHeader);

  void resolveNewHeaderPHIDependencies(WhilifierContext &WC);

  void findNonPHIOriginalLatchDependencies(WhilifierContext &WC);

  void resolveNewHeaderNonPHIDependencies(WhilifierContext &WC,
                                          BasicBlock *NewHeader);

  void resolveNewHeaderDependencies(WhilifierContext &WC,
                                    BasicBlock *NewHeader);

  void resolveOriginalHeaderPHIs(WhilifierContext &WC);

  void rerouteLoopBranches(WhilifierContext &WC, BasicBlock *NewHeader);
};

} // namespace llvm::noelle
