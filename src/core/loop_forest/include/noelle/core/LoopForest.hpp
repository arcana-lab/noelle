/*
 * Copyright 2019 - 2021  Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_LOOP_FOREST_LOOPFOREST_H_
#define NOELLE_SRC_CORE_LOOP_FOREST_LOOPFOREST_H_
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/Dominators.hpp"
#include "noelle/core/LoopStructure.hpp"

namespace arcana::noelle {

class LoopForest;

class LoopTree {
public:
  LoopTree(LoopForest *f, LoopStructure *l);

  LoopTree(LoopForest *f, LoopStructure *l, LoopTree *parent);

  LoopStructure *getLoop(void) const;

  LoopStructure *getInnermostLoopThatContains(Instruction *i);

  LoopStructure *getInnermostLoopThatContains(BasicBlock *bb);

  LoopStructure *getOutermostLoopThatContains(Instruction *i);

  LoopStructure *getOutermostLoopThatContains(BasicBlock *bb);

  std::set<LoopTree *> getNodes(void);

  std::set<LoopStructure *> getLoops(void);

  LoopTree *getParent(void) const;

  std::unordered_set<LoopTree *> getChildren(void) const;

  std::unordered_set<LoopTree *> getDescendants(void);

  bool isIncludedInItsSubLoops(Instruction *inst) const;

  /*
   * This function returns the total number of sub-loops contained by @this.
   * This includes the sub-loops of sub-loops.
   */
  uint32_t getNumberOfSubLoops(void) const;

  bool visitPreOrder(
      std::function<bool(LoopTree *n, uint32_t treeLevel)> funcToInvoke);

  bool visitPostOrder(
      std::function<bool(LoopTree *n, uint32_t treeLevel)> funcToInvoke);

  ~LoopTree();

private:
  friend class LoopForest;
  LoopForest *forest;
  LoopStructure *loop;
  LoopTree *parent;
  std::unordered_set<LoopTree *> children;

  bool visitPreOrder(
      std::function<bool(LoopTree *n, uint32_t treeLevel)> funcToInvoke,
      uint32_t treeLevel);
  bool visitPostOrder(
      std::function<bool(LoopTree *n, uint32_t treeLevel)> funcToInvoke,
      uint32_t treeLevel);
};

class LoopForest {
public:
  LoopForest(std::vector<LoopStructure *> const &loops,
             std::unordered_map<Function *, DominatorSummary *> const &doms);

  uint64_t getNumberOfLoops(void) const;

  std::unordered_set<LoopTree *> getTrees(void) const;

  void removeTree(LoopTree *tree);

  void addTree(LoopTree *tree);

  LoopTree *getNode(LoopStructure *loop) const;

  LoopTree *getInnermostLoopThatContains(Instruction *i) const;

  LoopTree *getInnermostLoopThatContains(BasicBlock *bb) const;

  ~LoopForest();

private:
  std::unordered_map<LoopStructure *, LoopTree *> nodes;
  std::unordered_set<LoopTree *> trees;
  std::unordered_map<Function *, std::unordered_set<LoopStructure *>>
      functionLoops;
  std::unordered_map<BasicBlock *, LoopTree *> headerLoops;

  void addChildrenToTree(
      LoopTree *root,
      std::unordered_map<Function *, DominatorSummary *> const &doms,
      std::unordered_set<LoopTree *> &potentialTrees);
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_LOOP_FOREST_LOOPFOREST_H_
