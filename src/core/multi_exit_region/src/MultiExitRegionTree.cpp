/*
 * Copyright 2024 - Federico Sossai, Simone Campanoni
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
#include <queue>
#include <stack>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"

#include "arcana/noelle/core/LoopStructure.hpp"
#include "arcana/noelle/core/MultiExitRegionTree.hpp"

using namespace std;
using namespace llvm;

namespace arcana::noelle {

MultiExitRegionTree::MultiExitRegionTree(Function *F,
                                         DominatorTree *DT,
                                         Instruction *Begin,
                                         Instruction *End)
  : F(F),
    DT(DT),
    Begin(Begin),
    End(End),
    isArtificialRoot(false) {}

MultiExitRegionTree::MultiExitRegionTree(
    Function &F,
    function<bool(const Instruction *)> isBegin,
    function<bool(const Instruction *)> isEnd)
  : F(&F),
    parent(nullptr),
    Begin(nullptr),
    End(nullptr),
    isArtificialRoot(true) {

  this->DT = new DominatorTree(F);

  vector<Instruction *> UnmatchedBegins;
  unordered_map<Instruction *, MultiExitRegionTree *> BeginToIncompleteRegion;

  auto indexOfLastUnmatchedBeginThatDominates = [&](const Instruction *End) {
    int i = UnmatchedBegins.size() - 1;
    for (; i >= 0; i--) {
      if (this->DT->dominates(UnmatchedBegins[i], End)) {
        break;
      }
    }
    return i;
  };

  // We do a Breadth-first search on the CFG:
  // -  When a Begin is found it is marked as unmatched and a new region is
  //    instantiated (this will be called "incomplete region").
  // -  When an End is found it is matched against an unmatched Begin using
  //    dominance.
  // -  When a match is found, the new complete region is nested inside an
  //    incomplete one using dominance.

  queue<BasicBlock *> worklist;
  unordered_set<BasicBlock *> enqueued;
  auto root = &F.getEntryBlock();
  worklist.push(root);
  enqueued.insert(root);

  while (!worklist.empty()) {
    auto BB = worklist.front();
    worklist.pop();

    for (auto &I : *BB) {
      if (isBegin(&I)) {
        auto Begin = &I;
        // Found a new Begin. Let's create a new incomplete region that will be
        // complete (i.e. will have an End) later on
        auto T =
            new MultiExitRegionTree(this->F, this->DT, Begin, /*End=*/nullptr);

        BeginToIncompleteRegion[Begin] = T;
        UnmatchedBegins.push_back(Begin);
      } else if (isEnd(&I)) {
        auto End = &I;
        // We should be able to find a matching `Begin` for `End`.
        int i = indexOfLastUnmatchedBeginThatDominates(End);
        // If we are here means no Begin dominates the End. This is impossible
        // if the regions are well-formed. We call this Malformed input.
        // We could go ahead and ignore this End but it is more informative to
        // report the problem.
        assert(i >= 0 && "Malformed regions");

        // Found a match! <3
        auto MatchingBegin = UnmatchedBegins[i];
        auto T = BeginToIncompleteRegion.at(MatchingBegin);
        T->Begin = MatchingBegin;
        T->End = End;
        UnmatchedBegins.erase(UnmatchedBegins.begin() + i);

        // Use dominance info to determine nesting relations between this new
        // region we just found and the unmatched regions. The parent, if any,
        // must be a tree for which we haven't found an End yet (i.e. in
        // `UnmatchedBegins`)
        i = indexOfLastUnmatchedBeginThatDominates(End);
        if (i < 0) {
          // Couldn't find another Region that dominates `T`.
          // We deduce that `T` is not nested in any incomplete tree
          T->parent = this;
          this->addChild(T);
        } else {
          // `T` is nested in the i-th tree
          auto ParentT = BeginToIncompleteRegion.at(UnmatchedBegins[i]);
          T->parent = ParentT;
          ParentT->addChild(T);
        }
      }
    }

    for (auto succBB : successors(BB)) {
      bool notEnqueued = enqueued.find(succBB) == enqueued.end();
      if (notEnqueued) {
        enqueued.insert(succBB);
        worklist.push(succBB);
      }
    }
  }

  assert(UnmatchedBegins.size() == 0 && "Malformed regions");
}

MultiExitRegionTree::~MultiExitRegionTree() {
  if (this->isArtificialRoot) {
    assert(this->parent == nullptr);
    free(this->DT);
  }
}

void MultiExitRegionTree::addChild(MultiExitRegionTree *T) {
  // This function hides the container used to store children
  if (find(this->children, T) == end(this->children)) {
    this->children.push_back(T);
  } else {
    // We don't expect to insert the same child twice
    assert(false);
  }
}

bool MultiExitRegionTree::isEmpty() const {
  return this->children.size() == 0;
}

bool MultiExitRegionTree::contains(const Instruction *I) {
  if (this->isArtificialRoot) {
    // The root contains everything by definition, as long as `I` is in `F`
    return I->getParent()->getParent() == this->F;
  }
  if (!this->DT->dominates(this->Begin, I)) {
    // Dominance is a necessary condition therefore we can immediately return if
    // not met
    return false;
  }
  return this->findInnermostRegionFor(I) != nullptr;
}

bool MultiExitRegionTree::contains(const BasicBlock *BB) {
  // The tree contains `BB` iff it contains its first and last instructions
  auto FirstI = &*BB->begin();
  auto LastI = BB->getTerminator();
  return this->contains(FirstI) && this->contains(LastI);
}

bool MultiExitRegionTree::contains(const LoopStructure *LS) {
  // The tree contains `LS` iff it contains its header and all latches
  for (auto Latch : LS->getLatches()) {
    if (!this->contains(Latch)) {
      return false;
    }
  }
  return this->contains(LS->getHeader());
}

bool MultiExitRegionTree::strictlyContains(const Instruction *I) {
  auto outermost = this->findOutermostRegionFor(I);
  if (outermost == nullptr) {
    return false;
  }
  auto innermost = this->findInnermostRegionFor(I);
  return outermost == innermost;
}

bool MultiExitRegionTree::strictlyContains(const BasicBlock *BB) {
  auto outermost = this->findOutermostRegionFor(BB);
  if (outermost == nullptr) {
    return false;
  }
  auto innermost = this->findInnermostRegionFor(BB);
  return outermost == innermost;
}

bool MultiExitRegionTree::strictlyContains(const LoopStructure *LS) {
  auto outermost = this->findOutermostRegionFor(LS);
  if (outermost == nullptr) {
    return false;
  }
  auto innermost = this->findInnermostRegionFor(LS);
  return outermost == innermost;
}

MultiExitRegionTree *MultiExitRegionTree::findOutermostRegionFor(
    const Instruction *I) {
  if (this->isArtificialRoot) {
    for (auto T : this->children) {
      if (T->findInnermostRegionFor(I) != nullptr) {
        return T;
      }
    }
  }
  if (this->findInnermostRegionFor(I) != nullptr) {
    return this;
  }
  return nullptr;
}

MultiExitRegionTree *MultiExitRegionTree::findOutermostRegionFor(
    const BasicBlock *BB) {
  if (!this->contains(BB)) {
    return nullptr;
  }
  auto FirstI = &*BB->begin();
  return this->findOutermostRegionFor(FirstI);
}

MultiExitRegionTree *MultiExitRegionTree::findOutermostRegionFor(
    const LoopStructure *LS) {
  if (!this->contains(LS)) {
    return nullptr;
  }
  return this->findOutermostRegionFor(LS->getHeader());
}

MultiExitRegionTree *MultiExitRegionTree::findInnermostRegionFor(
    const Instruction *I) {
  queue<MultiExitRegionTree *> worklist1;
  unordered_map<const BasicBlock *, MultiExitRegionTree *> TargetBBs;

  if (this->isArtificialRoot) {
    for (auto T : this->children) {
      worklist1.push(T);
    }
  } else {
    worklist1.push(this);
  }

  auto setCandidate = [&](MultiExitRegionTree *T) {
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
      return get<MultiExitRegionTree *>(*TargetBB);
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

MultiExitRegionTree *MultiExitRegionTree::findInnermostRegionFor(
    const BasicBlock *BB) {
  if (!this->contains(BB)) {
    return nullptr;
  }
  auto FirstI = &*BB->begin();
  return this->findInnermostRegionFor(FirstI);
}

MultiExitRegionTree *MultiExitRegionTree::findInnermostRegionFor(
    const LoopStructure *LS) {
  if (!this->contains(LS)) {
    return nullptr;
  }
  return this->findInnermostRegionFor(LS->getHeader());
}

vector<MultiExitRegionTree *> MultiExitRegionTree::getPathTo(
    const llvm::Instruction *I) {
  auto destination = findInnermostRegionFor(I);
  if (destination == nullptr) {
    return {};
  }

  // Upward tree traversal
  stack<MultiExitRegionTree *> ancestors;
  auto current = destination;
  while (current != this) {
    ancestors.push(current);
    current = current->parent;
  }
  if (!this->isArtificialRoot) {
    ancestors.push(this);
  }

  vector<MultiExitRegionTree *> path;
  while (!ancestors.empty()) {
    path.push_back(ancestors.top());
    ancestors.pop();
  }
  return path;
}

MultiExitRegionTree::ChildrenTy MultiExitRegionTree::getChildren() const {
  return this->children;
}

Instruction *MultiExitRegionTree::getBegin() const {
  return this->Begin;
}

Instruction *MultiExitRegionTree::getEnd() const {
  return this->End;
}

MultiExitRegionTree *MultiExitRegionTree::getParent() {
  return this->parent;
}

raw_ostream &MultiExitRegionTree::print(raw_ostream &stream,
                                        string prefixToUse) {
  return this->print(stream, prefixToUse, 0);
}

raw_ostream &MultiExitRegionTree::print(raw_ostream &stream,
                                        string prefixToUse,
                                        int level) {
  string beginPrefix = "\u250F ";
  string endPrefix = "\u2517 ";
  string levelPrefix = "";

  for (int i = 0; i < level; i++) {
    levelPrefix += "\u2503 ";
  }

  if (!this->isArtificialRoot) {
    stream << prefixToUse << levelPrefix << beginPrefix << *Begin << "\n";
  }

  for (auto T : this->children) {
    assert(T != nullptr);
    T->print(stream, prefixToUse, level + 1);
  }

  if (!this->isArtificialRoot) {
    stream << prefixToUse << levelPrefix << endPrefix << *End << "\n";
  }

  return stream;
}

} // namespace arcana::noelle
