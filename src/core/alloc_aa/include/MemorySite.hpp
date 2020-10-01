/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/iterator_range.h"
#include <set>
#include <unordered_map>

using namespace llvm;
using namespace std;

namespace llvm {

  /*
   * GOAL: Traverse call graph to explore memory sites.
   * Aggregate references to a site to be able to look up the site across functions
   * Aggregate escaping values of a site to understand when the site is no longer understood
   * Query APIs: whether mem ops access different sites or the same site at the same offset
   * Provide the aggregated offset into the allocation of a particular reference
   * Provide a tree of MemorySite along with the offsets of that tree
   */

  enum class AllocAAResult { No, May, Must };

  struct MemoryLayout;
  struct MemorySite;
  struct MemoryReference;

  class MemorySiteInfo {
    public:
      AllocAAResult doesAlias (Value *V1, Value *V2);

    private:
      MemoryLayout *memoryLayout;
      std::unordered_map<CallInst *, MemorySite *> allocCallSites;
      std::unordered_map<AllocaInst *, MemorySite *> allocaSites;
      std::unordered_map<Value *, MemorySite *> referenceSites;
      std::set<MemorySite *> memorySites;
  };

  struct MemoryLayout {
    std::unordered_map<MemorySite *, std::set<MemoryReference *>> parents;
    std::unordered_map<MemorySite *, std::set<MemoryReference *>> children;
  };

  struct MemorySite {
    Value *allocation;
    int sizeInBits;

    std::unordered_map<Argument *, MemoryReference *> argumentReferences;
    std::unordered_map<Instruction *, MemoryReference *> instructionReferences;

    std::set<Value *> escapingValues;
  };

  struct MemoryReference {
    Value *reference;
    Value *offsetValue;

    bool offsetDetermined;
    int offsetInBits;
  };
}
