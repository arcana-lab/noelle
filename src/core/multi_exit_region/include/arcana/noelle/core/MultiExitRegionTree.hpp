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

#include <vector>
#include <unordered_set>

#include "llvm/IR/Instructions.h"

namespace arcana::noelle {

class MultiExitRegionTree {
public:
  MultiExitRegionTree(const std::unordered_set<llvm::Instruction *> &Begins,
                      const std::unordered_set<llvm::Instruction *> &Ends);

  bool contains(const llvm::Instruction *I) const;

  bool strictlyContains(const llvm::Instruction *I) const;

  MultiExitRegionTree *findSubregionFor(const llvm::Instruction *I) const;

  MultiExitRegionTree *getRoot() const;

private:
  MultiExitRegionTree *parent;
  std::vector<MultiExitRegionTree *> children;
};

} // namespace arcana::noelle

#endif
