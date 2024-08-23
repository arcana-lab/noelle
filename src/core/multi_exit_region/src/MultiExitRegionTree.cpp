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
    isRoot(true) {
  this->DT = new DominatorTree(F);
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
