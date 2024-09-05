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

  auto isBegin = [&](const Value *V) -> bool {
    auto CI = dyn_cast<CallInst>(V);
    if (CI == nullptr) {
      return false;
    }
    auto Callee = CI->getCalledFunction();
    if (Callee == nullptr) {
      return false;
    }
    if (Callee->getName() == "noelle_pragma_begin"
        || Callee->getName().startswith("_Z19noelle_pragma_begin")) {
      StringRef beginDirective;
      if (PragmaTree::getStringFromArg(CI->getArgOperand(0), beginDirective)) {
        if (beginDirective.startswith(directive)) {
          return true;
        }
      }
    }
    return false;
  };

  auto isEnd = [&](const Value *V) -> bool {
    auto CI = dyn_cast<CallInst>(V);
    if (CI == nullptr) {
      return false;
    }
    auto Callee = CI->getCalledFunction();
    if (Callee == nullptr) {
      return false;
    }
    if (!Callee->getName().startswith("noelle_pragma_end")) {
      return false;
    }
    return true;
  };

  // Time for a BFS
  // Even though searching for 'pragma end's is easier because it gives its
  // corresponding begin for free, we do a Breadth-first search on the CFG
  // search for 'pragma begin's. This is because this order guarantees that
  // whenever a new pragma region is found, its parent has been found already
  // and we can immediately nest them if necessery. Mind that B is contained in
  // A then A's begin dominates B's begin.

  queue<BasicBlock *> worklist;
  unordered_set<BasicBlock *> enqueued;
  auto root = &F.getEntryBlock();
  worklist.push(root);
  enqueued.insert(root);

  while (!worklist.empty()) {
    auto BB = worklist.front();
    worklist.pop();

    for (auto &I : *BB) {
      if (!isBegin(&I)) {
        continue;
      }

      Instruction *End = nullptr;
      for (auto U : I.users()) {
        if (isEnd(U)) {
          End = cast<Instruction>(U);
          break;
        }
      }

      assert(End != nullptr && "Can't find corresponding Pragma End");

      auto Begin = &I;
      assert(this->DT->dominates(Begin, End) && "Corrupted pragmas");
      auto newT = new PragmaTree(&F, this->DT, Begin, End);
      addChild(newT);
    }

    for (auto succBB : successors(BB)) {
      bool notEnqueued = enqueued.find(succBB) == enqueued.end();
      if (notEnqueued) {
        enqueued.insert(succBB);
        worklist.push(succBB);
      }
    }
  }
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
  assert(std::find(std::begin(this->trees), std::end(this->trees), T)
         == std::end(this->trees));

  auto foundParent =
      visitPostOrder([&](PragmaTree *potentialParent, auto) -> bool {
        if (potentialParent->contains(T->Begin)) {
          potentialParent->addChild(T);
          return true;
        }
        return false;
      });

  // If no parent is found it means that `T` is a new tree in the forest
  if (!foundParent) {
    this->trees.push_back(T);
    assert(T->parent == nullptr);
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
