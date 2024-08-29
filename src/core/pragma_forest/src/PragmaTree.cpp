/*
 * Copyright 2024 - Federico Sossai
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

#include <algorithm>
#include <functional>
#include <iterator>
#include <queue>
#include <stack>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"

#include "arcana/noelle/core/LoopStructure.hpp"
#include "arcana/noelle/core/PragmaForest.hpp"

using namespace std;
using namespace llvm;

namespace arcana::noelle {

PragmaTree::PragmaTree(Function *F,
                       DominatorTree *DT,
                       Instruction *Begin,
                       Instruction *End)
  : F(F),
    DT(DT),
    Begin(Begin),
    End(End) {}

PragmaTree::~PragmaTree() {
  for (auto T : this->children) {
    delete T;
  }
}

void PragmaTree::addChild(PragmaTree *T) {
  // This function hides the container used to store children
  if (std::find(std::begin(this->children), std::end(this->children), T)
      == std::end(this->children)) {
    this->children.push_back(T);
    T->parent = this;
  } else {
    // We don't expect to insert the same child twice
    assert(false && "Child node added twice");
  }
}

bool PragmaTree::getStringFromArg(Value *arg, StringRef &result) {
  auto GEP = dyn_cast<GetElementPtrInst>(arg);
  if (GEP == nullptr) {
    return false;
  }
  auto Ptr = GEP->getPointerOperand();
  if (!isa<Constant>(Ptr)) {
    return false;
  }
  auto GV = dyn_cast<llvm::GlobalVariable>(Ptr);
  if (GV == nullptr || !GV->isConstant()) {
    return false;
  }
  auto ATy = dyn_cast<llvm::ArrayType>(GV->getValueType());
  if (ATy == nullptr) {
    return false;
  }
  if (!ATy->getElementType()->isIntegerTy(8)) {
    return false;
  }
  auto *CDA = llvm::dyn_cast<llvm::ConstantDataArray>(GV->getInitializer());
  if (!CDA->isString()) {
    return false;
  }
  result = CDA->getAsCString();
  return true;
}

bool PragmaTree::isEmpty() const {
  return this->children.size() == 0;
}

bool PragmaTree::contains(const Instruction *I) {
  if (!this->DT->dominates(this->Begin, I)) {
    // Dominance is a necessary condition therefore we can immediately return if
    // not met
    return false;
  }
  return findInnermostPragmaFor(I) != nullptr;
}

bool PragmaTree::contains(const BasicBlock *BB) {
  // The tree contains `BB` iff it contains its first and last instructions
  auto FirstI = &*BB->begin();
  auto LastI = BB->getTerminator();
  return contains(FirstI) && contains(LastI);
}

bool PragmaTree::contains(const LoopStructure *LS) {
  // The tree contains `LS` iff it contains its header and all latches
  for (auto Latch : LS->getLatches()) {
    if (!contains(Latch)) {
      return false;
    }
  }
  return contains(LS->getHeader());
}

bool PragmaTree::strictlyContains(const Instruction *I) {
  return this == findInnermostPragmaFor(I);
}

bool PragmaTree::strictlyContains(const BasicBlock *BB) {
  return this == findInnermostPragmaFor(BB);
}

bool PragmaTree::strictlyContains(const LoopStructure *LS) {
  return this == findInnermostPragmaFor(LS);
}

PragmaTree *PragmaTree::findInnermostPragmaFor(const Instruction *I) {
  queue<PragmaTree *> worklist1;
  unordered_map<const BasicBlock *, PragmaTree *> TargetBBs;

  worklist1.push(this);

  auto setCandidate = [&](PragmaTree *T) {
    auto BeginBB = T->Begin->getParent();
    TargetBBs[BeginBB] = T;
  };

  // Phase 1
  // If a child region dominates `I` there no point in considering the parent
  // as a candidate because the dominance of a child is a stronger information.
  // Given this fact, we find what can be thought of as a "dominance frontier"
  // of instruction `I` with respect to this region tree.
  // In layman's terms we find the regions `T` that dominate `I` and:
  // - `T` is a leaf
  //   or
  // - none of `T`'s children dominate `I`
  while (!worklist1.empty()) {
    auto T = worklist1.front();
    worklist1.pop();

    // We could be lucky. This check is needed in any case because instruction
    // `I` wouldn't dominate itself
    if (I == T->Begin || I == T->End) {
      return T;
    }

    auto children = T->getChildren();
    if (children.size() == 0) {
      // Leaf case:
      // If the current region is a leaf it's the deepest by definition
      if (this->DT->dominates(T->Begin, I)) {
        setCandidate(T);
      }
    } else {
      // Inner case:
      bool noChildrenDominates = true;
      for (auto C : children) {
        if (this->DT->dominates(T->Begin, I)) {
          // We will continue the search in `C`
          worklist1.push(C);
          noChildrenDominates = false;
        }
      }
      if (noChildrenDominates) {
        // `T` is the deepest node whose `Begin` dominates `I` therefore we
        // consider it a candidate
        setCandidate(T);
      }
    }
  }

  // Phase 2
  // Reverse BFS on the CFG starting from the block that contains `I`.
  // We search the first basic block that is contained in the set of targets
  // (`TargetBBs`). The associate region is guaranteed to be the one we are
  // looking for.
  //
  // Proof:
  // Assume we reach more than one target BB. That would imply
  // that there's more than one `Begin` instruction that reaches `I` because we
  // followed the CFG. This is a contradiction as all regions are single-entry
  // AND we only kept the deepest regions in Phase 1 (see above).

  queue<const BasicBlock *> worklist2;
  unordered_set<const BasicBlock *> enqueued;
  worklist2.push(I->getParent());

  while (!worklist2.empty()) {
    auto BB = worklist2.front();
    worklist2.pop();

    auto TargetBB = TargetBBs.find(BB);
    auto foundTarget = TargetBB != TargetBBs.end();
    if (foundTarget) {
      return get<PragmaTree *>(*TargetBB);
    }

    for (const auto pBB : predecessors(BB)) {
      bool notEnqueued = enqueued.find(pBB) == enqueued.end();
      if (notEnqueued) {
        enqueued.insert(pBB);
        worklist2.push(pBB);
      }
    }
  }
  return nullptr;
}

PragmaTree *PragmaTree::findInnermostPragmaFor(const BasicBlock *BB) {
  if (!contains(BB)) {
    return nullptr;
  }
  auto FirstI = &*BB->begin();
  return findInnermostPragmaFor(FirstI);
}

PragmaTree *PragmaTree::findInnermostPragmaFor(const LoopStructure *LS) {
  if (!contains(LS)) {
    return nullptr;
  }
  return findInnermostPragmaFor(LS->getHeader());
}

PragmaTree *PragmaTree::getRoot() {
  auto T = this;
  while (T->parent != nullptr) {
    T = T->parent;
  }
  return T;
}

unordered_set<Instruction *> PragmaTree::getInstructionsWithin() {
  unordered_set<Instruction *> Is;

  auto BeginBB = this->Begin->getParent();
  auto EndBB = this->End->getParent();
  if (BeginBB == EndBB) {
    // Adding instructions in between `Begin` and `End`
    auto itB = this->Begin->getIterator();
    auto itE = this->End->getIterator();
    while (itB != itE) {
      Is.insert(&*itB);
      ++itB;
    }
    Is.insert(&*itE);
  }

  for (auto BB : getBasicBlocksWithin()) {
    for (auto &I : *BB) {
      Is.insert(&I);
    }
  }

  return Is;
}

unordered_set<BasicBlock *> PragmaTree::getBasicBlocksWithin() {
  auto BeginBB = this->Begin->getParent();
  auto EndBB = this->End->getParent();

  if (BeginBB == EndBB) {
    auto FirstI = &*BeginBB->begin();
    auto LastI = BeginBB->getTerminator();
    if (FirstI == this->Begin && LastI == this->End) {
      return { BeginBB };
    }
    return {};
  }

  // Upward breadth-first search on the CFG starting from the predecessors of
  // `EndBB`.

  queue<BasicBlock *> worklist;
  unordered_set<BasicBlock *> enqueued;
  unordered_set<BasicBlock *> BBs;

  for (auto predBB : predecessors(EndBB)) {
    worklist.push(predBB);
    enqueued.insert(predBB);
  }

  while (!worklist.empty()) {
    auto BB = worklist.front();
    worklist.pop();

    if (!this->DT->dominates(this->Begin, BB)) {
      continue;
    }
    BBs.insert(BB);

    for (auto predBB : predecessors(BB)) {
      bool notEnqueued = enqueued.find(predBB) == enqueued.end();
      if (notEnqueued) {
        worklist.push(predBB);
        enqueued.insert(predBB);
      }
    }
  }

  return BBs;
}

vector<PragmaTree *> PragmaTree::getAncestorsFromRootTo(
    const llvm::Instruction *I) {
  auto destination = findInnermostPragmaFor(I);
  if (destination == nullptr) {
    return {};
  }

  // Upward tree traversal
  stack<PragmaTree *> ancestors;
  auto current = destination;
  while (current != this) {
    ancestors.push(current);
    current = current->parent;
  }
  ancestors.push(this);

  vector<PragmaTree *> path;
  while (!ancestors.empty()) {
    path.push_back(ancestors.top());
    ancestors.pop();
  }
  return path;
}

vector<PragmaTree *> PragmaTree::getChildren() const {
  return this->children;
}

Instruction *PragmaTree::getBeginDelimiter() const {
  return this->Begin;
}

Instruction *PragmaTree::getEndDelimiter() const {
  return this->End;
}

PragmaTree *PragmaTree::getParent() {
  return this->parent;
}

string PragmaTree::getDirective() const {
  auto CI = cast<CallInst>(this->Begin);
  auto GEP = cast<GetElementPtrInst>(CI->getArgOperand(0));
  auto Ptr = GEP->getPointerOperand();
  auto GV = cast<GlobalVariable>(Ptr);
  auto CDA = cast<ConstantDataArray>(GV->getInitializer());
  return CDA->getAsCString().str();
}

vector<Value *> PragmaTree::getArguments() const {
  auto CI = cast<CallInst>(this->Begin);

  // The first argument is skipped because it's the directive

  vector<Value *> args;
  for (size_t i = 1; i < CI->arg_size(); i++) {
    args.push_back(CI->getArgOperand(i));
  }

  return args;
}

raw_ostream &PragmaTree::print(raw_ostream &stream, string prefix) {
  return print(stream, prefix, LAST);
}

raw_ostream &PragmaTree::print(raw_ostream &stream,
                               string prefix,
                               SiblingType ST) {
  string nodePrefix = "";

  switch (ST) {
    case INNER:
      nodePrefix = "\u2523\u2501 ";
      break;
    case LAST:
      nodePrefix = "\u2517\u2501 ";
      break;
  }

  stream << prefix << nodePrefix << getDirective() << "\n";

  if (ST == LAST) {
    prefix += "   ";
  } else {
    prefix += "\u2503  ";
  }

  auto N = this->children.size();

  if (N >= 1) {
    for (size_t i = 0; i < N - 1; i++) {
      this->children[i]->print(stream, prefix, INNER);
    }
    this->children[N - 1]->print(stream, prefix, LAST);
  }

  return stream;
}

bool PragmaTree::visitPreOrder(
    std::function<bool(PragmaTree *T, uint32_t level)> callback) {
  return this->visitPreOrder(callback, 1);
}

bool PragmaTree::visitPostOrder(
    std::function<bool(PragmaTree *T, uint32_t level)> callback) {
  return this->visitPostOrder(callback, 1);
}

bool PragmaTree::visitPreOrder(
    std::function<bool(PragmaTree *T, uint32_t level)> callback,
    uint32_t level) {
  if (callback(this, level)) {
    return true;
  }
  for (auto child : this->children) {
    if (child->visitPreOrder(callback, level + 1)) {
      return true;
    }
  }
  return false;
}

bool PragmaTree::visitPostOrder(
    std::function<bool(PragmaTree *T, uint32_t level)> callback,
    uint32_t level) {
  for (auto child : this->children) {
    if (child->visitPostOrder(callback, level + 1)) {
      return true;
    }
  }
  if (callback(this, level)) {
    return true;
  }
  return false;
}

} // namespace arcana::noelle
