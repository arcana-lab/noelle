/*
 * Copyright 2024  Federico Sossai, Simone Campanoni
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

#ifndef __MUTLI_EXIT_REGION_TREE__HPP__
#define __MUTLI_EXIT_REGION_TREE__HPP__

#include <functional>
#include <string>
#include <unordered_set>
#include <vector>

#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"

#include "arcana/noelle/core/LoopStructure.hpp"

namespace arcana::noelle {

class MultiExitRegionTree {
public:
  using ChildrenTy = std::vector<MultiExitRegionTree *>;

  MultiExitRegionTree(llvm::Function &F,
                      std::function<bool(const llvm::Instruction *)> isBegin,
                      std::function<bool(const llvm::Instruction *)> isEnd);

  ~MultiExitRegionTree();

  bool contains(const llvm::Instruction *I);

  bool contains(const llvm::BasicBlock *BB);

  bool contains(const LoopStructure *LS);

  bool strictlyContains(const llvm::Instruction *I);

  MultiExitRegionTree *findOutermostRegionFor(const llvm::Instruction *I);

  MultiExitRegionTree *findOutermostRegionFor(const llvm::BasicBlock *BB);

  MultiExitRegionTree *findOutermostRegionFor(const LoopStructure *LS);

  MultiExitRegionTree *findInnermostRegionFor(const llvm::Instruction *I);

  MultiExitRegionTree *findInnermostRegionFor(const llvm::BasicBlock *BB);

  MultiExitRegionTree *findInnermostRegionFor(const LoopStructure *LS);

  std::vector<MultiExitRegionTree *> getPathTo(const llvm::Instruction *I);

  ChildrenTy getChildren() const;

  llvm::Instruction *getBegin() const;

  llvm::Instruction *getEnd() const;

  MultiExitRegionTree *getParent();

  llvm::raw_ostream &print(llvm::raw_ostream &stream,
                           std::string prefixToUse = "");

private:
  MultiExitRegionTree(llvm::Function *F,
                      llvm::DominatorTree *DT,
                      llvm::Instruction *Begin,
                      llvm::Instruction *End);

  void addChild(MultiExitRegionTree *T);

  llvm::raw_ostream &print(llvm::raw_ostream &stream,
                           std::string prefixToUse,
                           int level);

  llvm::Function *F;
  llvm::DominatorTree *DT;
  MultiExitRegionTree *parent;
  llvm::Instruction *Begin;
  llvm::Instruction *End;

  // `children` is logically an unordered_set. But because of how the tree is
  // contructed we want to preserve the insertion order as it is likely to
  // reflect the control flow order
  ChildrenTy children;

  bool isArtificialRoot;
};

} // namespace arcana::noelle

#endif
