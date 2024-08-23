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

#include <functional>
#include <queue>
#include <stack>
#include <vector>
#include <unordered_set>

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

  queue<BasicBlock *> worklist;
  unordered_set<BasicBlock *> enqueued;
  vector<Instruction *> Begins;
  vector<Instruction *> Ends;

  // Breadth-first search on the CFG

  auto root = &F.getEntryBlock();
  worklist.push(root);
  enqueued.insert(root);

  while (!worklist.empty()) {
    auto BB = worklist.front();
    worklist.pop();

    for (auto &I : *BB) {
      if (isBegin(&I)) {
        Begins.push_back(&I);
      } else if (isEnd(&I)) {
        auto End = &I;
        // Find a dominator for End among the Begins
        int i;
        for (i = Begins.size() - 1; i >= 0; i--) {
          if (DT->dominates(Begins[i], End)) {
            break;
          }
        }
        if (i < 0) {
          Ends.push_back(End);
        } else {
          auto MERT = new MultiExitRegionTree(this->DT, Begins[i], End);
          MERT->parent = this;
          this->children.push_back(MERT);
          Begins.erase(Begins.begin() + i);
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

  errs() << "Unmatched Begins = " << Begins.size() << "\n";

  for (auto T : this->children) {
    errs() << "B: " << *T->Begin << "\n";
    errs() << "E: " << *T->End << "\n";
    errs() << "\n";
  }
}

MultiExitRegionTree::~MultiExitRegionTree() {
  if (this->isRoot) {
    assert(this->parent == nullptr);
    free(this->DT);
  }
}

bool MultiExitRegionTree::contains(const Instruction *I) const {
  return false;
}

bool MultiExitRegionTree::strictlyContains(const Instruction *I) const {
  return false;
}

MultiExitRegionTree *MultiExitRegionTree::findSubregionFor(
    const Instruction *I) {
  return nullptr;
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

} // namespace arcana::noelle
