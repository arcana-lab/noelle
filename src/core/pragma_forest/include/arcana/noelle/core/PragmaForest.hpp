/*
 * Copyright 2024  Federico Sossai
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

#ifndef __PRAGMA_FOREST_HPP__
#define __PRAGMA_FOREST_HPP__

#include <functional>
#include <string>
#include <unordered_set>
#include <vector>

#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"

#include "arcana/noelle/core/LoopStructure.hpp"

namespace arcana::noelle {

class PragmaForest; // Forward declaration

class PragmaTree {
  friend class PragmaForest;

public:
  PragmaTree(llvm::Function *F,
             llvm::DominatorTree *DT,
             llvm::Instruction *Begin,
             llvm::Instruction *End);

  ~PragmaTree();

  static bool getStringFromArg(llvm::Value *arg, llvm::StringRef &result);

  static bool getDoubleFromArg(llvm::Value *arg, double &result);

  static bool getIntFromArg(llvm::Value *arg, int &result);

  bool isEmpty() const;

  bool contains(const llvm::Instruction *I);

  bool contains(const llvm::BasicBlock *BB);

  bool contains(const LoopStructure *LS);

  bool strictlyContains(const llvm::Instruction *I);

  bool strictlyContains(const llvm::BasicBlock *BB);

  bool strictlyContains(const LoopStructure *LS);

  PragmaTree *findInnermostPragmaFor(const llvm::Instruction *I);

  PragmaTree *findInnermostPragmaFor(const llvm::BasicBlock *BB);

  PragmaTree *findInnermostPragmaFor(const LoopStructure *LS);

  PragmaTree *getRoot();

  std::unordered_set<llvm::Instruction *> getInstructionsWithin();

  std::unordered_set<llvm::BasicBlock *> getBasicBlocksWithin();

  std::vector<PragmaTree *> getAncestorsFromRootTo(const llvm::Instruction *I);

  std::vector<PragmaTree *> getChildren() const;

  std::vector<llvm::Value *> getArguments() const;

  std::string getDirective() const;

  llvm::Instruction *getBeginDelimiter() const;

  llvm::Instruction *getEndDelimiter() const;

  PragmaTree *getParent();

  llvm::raw_ostream &print(llvm::raw_ostream &stream, std::string prefix = "");

  void erase();

  bool visitPreOrder(
      std::function<bool(PragmaTree *T, uint32_t level)> callback);

  bool visitPostOrder(
      std::function<bool(PragmaTree *T, uint32_t level)> callback);

private:
  llvm::Function *F;
  llvm::DominatorTree *DT;
  PragmaTree *parent;
  llvm::Instruction *Begin;
  llvm::Instruction *End;

  // `children` is logically an unordered but because of how the tree is
  // contructed we want to preserve the insertion order as it is likely to
  // reflect the control flow order
  std::vector<PragmaTree *> children;

  enum SiblingType { INNER, LAST };

  void addChild(PragmaTree *T);

  llvm::raw_ostream &print(llvm::raw_ostream &stream,
                           std::string prefix,
                           SiblingType ST);

  bool visitPreOrder(
      std::function<bool(PragmaTree *T, uint32_t level)> callback,
      uint32_t level);

  bool visitPostOrder(
      std::function<bool(PragmaTree *T, uint32_t level)> callback,
      uint32_t level);
};

class PragmaForest {
public:
  PragmaForest(llvm::Function &F, std::string directive = "");

  ~PragmaForest();

  bool isEmpty() const;

  std::vector<PragmaTree *> getTrees() const;

  PragmaTree *findInnermostPragmaFor(const llvm::Instruction *I);

  PragmaTree *findInnermostPragmaFor(const llvm::BasicBlock *BB);

  PragmaTree *findInnermostPragmaFor(const LoopStructure *LS);

  llvm::raw_ostream &print(llvm::raw_ostream &stream, std::string prefix = "");

  void erase();

  bool visitPreOrder(
      std::function<bool(PragmaTree *T, uint32_t level)> callback);

  bool visitPostOrder(
      std::function<bool(PragmaTree *T, uint32_t level)> callback);

private:
  llvm::Function &F;
  std::string directive;
  llvm::DominatorTree *DT;
  std::vector<PragmaTree *> trees;

  void addChild(PragmaTree *T);
};

} // namespace arcana::noelle

#endif
