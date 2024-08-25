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

#include "arcana/noelle/core/MultiExitRegionTree.hpp"

using namespace std;
using namespace llvm;

namespace arcana::noelle {

MultiExitRegionTree::MultiExitRegionTree(DominatorTree *DT,
                                         Instruction *Begin,
                                         Instruction *End)
  : DT(DT),
    Begin(Begin),
    End(End),
    isRoot(false) {}

MultiExitRegionTree::MultiExitRegionTree(
    Function &F,
    function<bool(const Instruction *)> isBegin,
    function<bool(const Instruction *)> isEnd)
  : parent(nullptr),
    Begin(nullptr),
    End(nullptr),
    isRoot(true) {

  this->DT = new DominatorTree(F);

  // Breadth-first search on the CFG

  queue<BasicBlock *> worklist;
  unordered_set<BasicBlock *> enqueued;
  vector<Instruction *> UnmatchedBegins;
  unordered_map<Instruction *, MultiExitRegionTree *> BeginToMERT;

  auto root = &F.getEntryBlock();
  worklist.push(root);
  enqueued.insert(root);

  while (!worklist.empty()) {
    auto BB = worklist.front();
    worklist.pop();

    for (auto &I : *BB) {
      if (isBegin(&I)) {
        auto Begin = &I;
        // Found a new Begin. Let's instantiate an incomplete MERT
        // that will be completed (i.e. has an End) later on
        auto MERT = new MultiExitRegionTree(this->DT, Begin, /*End=*/nullptr);

        // For now we assume that it's not nested in any other tree.
        // Keep in mind that in this ctor, `this` is the artificial node that
        // contains ANY other subtree
        MERT->parent = this;
        BeginToMERT[Begin] = MERT;
        UnmatchedBegins.push_back(Begin);
      } else if (isEnd(&I)) {
        auto End = &I;
        // Find a dominator for End among the Begins
        int i;
        for (i = UnmatchedBegins.size() - 1; i >= 0; i--) {
          if (DT->dominates(UnmatchedBegins[i], End)) {
            break;
          }
        }
        if (i < 0) {
          // If we are here means no Begin dominates the End. This is impossible
          // if the regions are well-formed. We call this Malformed input.
          // We could go ahead and ignore this End but it is more instructive to
          // report the problem.
          assert(false && "Malformed MultiExitRegionTree\n");
        } else {
          // Found a match! <3
          auto MatchingBegin = UnmatchedBegins[i];
          auto MERT = BeginToMERT.at(MatchingBegin);
          MERT->Begin = MatchingBegin;
          MERT->End = End;
          UnmatchedBegins.erase(UnmatchedBegins.begin() + i);

          // Use dominance info to determine nesting relations between this new
          // region we just found. The parent, if any, must be a tree for which
          // we haven't found an End yet (i.e. in `UnmatchedBegins`)
          for (i = UnmatchedBegins.size() - 1; i >= 0; i--) {
            if (DT->dominates(UnmatchedBegins[i], End)) {
              break;
            }
          }
          if (i < 0) {
            // Couldn't find another Region that dominates `MERT`.
            // We deduce that `MERT` is not nested
            this->addChild(MERT);
          } else {
            // `MERT` is nested in the i-th tree
            auto ParentMERT = BeginToMERT.at(UnmatchedBegins[i]);
            MERT->parent = ParentMERT;
            ParentMERT->addChild(MERT);
          }
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

  if (UnmatchedBegins.size() > 0) {
    errs() << "Malformed Multi-exit region: " << UnmatchedBegins.size()
           << " unmatched Begins\n";
  }
  assert(UnmatchedBegins.size() == 0);
}

MultiExitRegionTree::~MultiExitRegionTree() {
  if (this->isRoot) {
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

bool MultiExitRegionTree::contains(const Instruction *I) {
  if (this->isRoot) {
    // The root contains everything by definition
    return true;
  }
  if (!this->DT->dominates(this->Begin, I)) {
    // Dominance is a necessary condition therefore we can immediately return if
    // not met
    return false;
  }
  return this->findInnermostRegionFor(I) != nullptr;
}

bool MultiExitRegionTree::strictlyContains(const Instruction *I) {
  return false;
}

MultiExitRegionTree *MultiExitRegionTree::findOutermostRegionFor(
    const Instruction *I) {
  if (this->isRoot) {
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

MultiExitRegionTree *MultiExitRegionTree::findInnermostRegionFor(
    const Instruction *I) {
  queue<MultiExitRegionTree *> worklist1;
  unordered_map<const BasicBlock *, MultiExitRegionTree *> TargetBBs;

  if (this->isRoot) {
    for (auto T : this->children) {
      worklist1.push(T);
    }
  } else {
    worklist1.push(this);
  }

  // The goal is to find the deepest set of sub trees whose associate `Begin`
  // dominates `I`. It can be thought as a "dominance frontier" of instruction
  // `I` with respect to this region tree
  while (!worklist1.empty()) {
    auto T = worklist1.front();
    worklist1.pop();

    // We could be lucky. This check is needed in any case because instruction
    // `I` wouldn't dominate itself
    if (I == T->Begin || I == T->End) {
      return T;
    }

    // If this sub tree doesn't dominate `I` then we consider its parent to be a
    // candidate and we stop searching in the other subtrees because if the
    // `Begin` of region doesn't dominate an instruction, neither will any
    // nested region as consequence of the single-entry property
    if (!this->DT->dominates(T->Begin, I)) {
      auto ParentT = T->parent;
      if (T != this && !ParentT->isRoot) {
        auto ParentBeginBB = ParentT->Begin->getParent();
        TargetBBs[ParentBeginBB] = ParentT;
      }
      continue;
    }

    for (auto C : T->children) {
      worklist1.push(C);
    }
  }

  // Reverse BFS on the CFG starting from the block that contains `I`.
  // We search the first basic block contained in the set of targets is
  // associated (through `TargetBBs`) to the region we are looking for.

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

vector<MultiExitRegionTree *> MultiExitRegionTree::buildPathTo(
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
  if (!this->isRoot) {
    ancestors.push(this);
  }

  vector<MultiExitRegionTree *> path;
  while (!ancestors.empty()) {
    path.push_back(ancestors.top());
    ancestors.pop();
  }
  return path;
}

MultiExitRegionTree *MultiExitRegionTree::getRoot() {
  auto current = this->parent;

  if (current == nullptr) {
    return this;
  }

  while (current->parent) {
    current = current->parent;
  }

  assert(current->isRoot);
  return current;
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
  string instBeginPrefix = "\u250F";
  string instEndPrefix = "\u2517";
  string levelPrefix = "";

  for (int i = 0; i < level; i++) {
    levelPrefix += "\u2503  ";
  }

  if (!isRoot) {
    stream << prefixToUse << levelPrefix << instBeginPrefix << *Begin << "\n";
  }

  for (auto T : this->children) {
    assert(T != nullptr);
    T->print(stream, prefixToUse, level + 1);
  }

  if (!isRoot) {
    stream << prefixToUse << levelPrefix << instEndPrefix << *End << "\n";
  }

  return stream;
}

} // namespace arcana::noelle
