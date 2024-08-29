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

#include "llvm/IR/CFG.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"

#include "arcana/noelle/core/LoopStructure.hpp"
#include "arcana/noelle/core/PragmaForest.hpp"

using namespace std;
using namespace llvm;

namespace arcana::noelle {

PragmaForest::PragmaForest(llvm::Function &F, std::string directive)
  : F(F),
    directive(directive) {
  if (F.empty()) {
    return;
  }

  this->DT = new DominatorTree(F);

  vector<Instruction *> UnmatchedBegins;
  unordered_map<Instruction *, PragmaTree *> BeginToIncompleteTree;

  auto indexOfLastUnmatchedBeginThatDominates = [&](const Instruction *End) {
    int i = UnmatchedBegins.size() - 1;
    for (; i >= 0; i--) {
      if (this->DT->dominates(UnmatchedBegins[i], End)) {
        break;
      }
    }
    return i;
  };

  auto isPragma = [&](const Instruction *I, string funcName) -> bool {
    auto CI = dyn_cast<CallInst>(I);
    if (CI == nullptr) {
      return false;
    }
    auto callee = CI->getCalledFunction();
    if (callee == nullptr) {
      return false;
    }
    if (!callee->getName().startswith(funcName)) {
      return false;
    }
    StringRef str;
    auto FirstArg = CI->getArgOperand(0);
    if (!PragmaTree::getStringFromArg(FirstArg, str)) {
      return false;
    }
    if (!str.startswith(directive)) {
      return false;
    }
    return true;
  };

  // A pragma is a CallInst whose first argument is global constant that
  // represents a string that starts with `directive`
  // The difference betweem a Begin and an End is the name of the called
  // function

  auto isBegin = bind(isPragma, placeholders::_1, "_Z19noelle_pragma_begin");
  auto isEnd = bind(isPragma, placeholders::_1, "_Z17noelle_pragma_end");

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
        auto T = new PragmaTree(&this->F, this->DT, Begin, /*End=*/nullptr);

        BeginToIncompleteTree[Begin] = T;
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
        auto T = BeginToIncompleteTree.at(MatchingBegin);
        T->Begin = MatchingBegin;
        T->End = End;
        UnmatchedBegins.erase(UnmatchedBegins.begin() + i);

        // Use dominance info to determine nesting relations between this new
        // region we just found and the unmatched regions. The parent, if any,
        // must be a tree for which we haven't found an End yet (i.e. in
        // `UnmatchedBegins`)
        i = indexOfLastUnmatchedBeginThatDominates(End);
        if (i < 0) {
          // Couldn't find another Tree that dominates `T`.
          // We deduce that `T` is not nested in any incomplete tree
          T->parent = nullptr;
          this->addChild(T);
        } else {
          // `T` is nested in the i-th tree
          auto ParentT = BeginToIncompleteTree.at(UnmatchedBegins[i]);
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

PragmaForest::~PragmaForest() {
  delete this->DT;
  for (auto T : this->trees) {
    delete T;
  }
}

vector<PragmaTree *> PragmaForest::getTrees() const {
  return this->trees;
}

raw_ostream &PragmaForest::print(raw_ostream &stream, string prefix) {
  stream << prefix << "\e[1;32m" + this->F.getName().str() + "\e[0m\n";

  auto N = this->trees.size();

  if (N >= 1) {
    for (size_t i = 0; i < N - 1; i++) {
      this->trees[i]->print(stream, prefix, PragmaTree::INNER);
    }
    this->trees[N - 1]->print(stream, prefix, PragmaTree::LAST);
  }

  return stream;
}

PragmaTree *PragmaForest::findInnermostPragmaFor(const Instruction *I) {
  for (auto T : this->trees) {
    auto innermost = T->findInnermostPragmaFor(I);
    if (innermost != nullptr) {
      return innermost;
    }
  }
  return nullptr;
}

PragmaTree *PragmaForest::findInnermostPragmaFor(const BasicBlock *BB) {
  for (auto T : this->trees) {
    auto innermost = T->findInnermostPragmaFor(BB);
    if (innermost != nullptr) {
      return innermost;
    }
  }
  return nullptr;
}

PragmaTree *PragmaForest::findInnermostPragmaFor(const LoopStructure *LS) {
  for (auto T : this->trees) {
    auto innermost = T->findInnermostPragmaFor(LS);
    if (innermost != nullptr) {
      return innermost;
    }
  }
  return nullptr;
}

bool PragmaForest::isEmpty() const {
  return this->trees.size() == 0;
}

void PragmaForest::addChild(PragmaTree *T) {
  // This function hides the container used to store trees
  if (std::find(std::begin(this->trees), std::end(this->trees), T)
      == std::end(this->trees)) {
    this->trees.push_back(T);
    assert(T->parent == nullptr);
    T->parent = nullptr;
  } else {
    // We don't expect to insert the same child twice
    assert(false && "Child tree added twice");
  }
}

bool PragmaForest::visitPreOrder(
    std::function<bool(PragmaTree *T, uint32_t level)> callback) {
  for (auto T : this->trees) {
    if (T->visitPreOrder(callback)) {
      return true;
    }
  }
  return false;
}

bool PragmaForest::visitPostOrder(
    std::function<bool(PragmaTree *T, uint32_t level)> callback) {
  for (auto T : this->trees) {
    if (T->visitPostOrder(callback)) {
      return true;
    }
  }
  return false;
}

} // namespace arcana::noelle
