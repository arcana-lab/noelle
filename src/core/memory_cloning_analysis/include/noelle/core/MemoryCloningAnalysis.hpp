/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
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

#ifndef NOELLE_SRC_CORE_MEMORY_CLONING_ANALYSIS_MEMORYCLONINGANALYSIS_H_
#define NOELLE_SRC_CORE_MEMORY_CLONING_ANALYSIS_MEMORYCLONINGANALYSIS_H_
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/PDG.hpp"
#include "noelle/core/SCCDAG.hpp"
#include "noelle/core/Invariants.hpp"
#include "noelle/core/Dominators.hpp"
#include "noelle/core/ClonableMemoryObject.hpp"

namespace arcana::noelle {

class MemoryCloningAnalysis {
public:
  MemoryCloningAnalysis(LoopStructure *loop, DominatorSummary &DS, PDG *ldg);

  const std::unordered_set<ClonableMemoryObject *> getClonableMemoryObjectsFor(
      Instruction *I) const;

  std::unordered_set<ClonableMemoryObject *> getClonableMemoryObjects(
      void) const;

private:
  std::unordered_set<std::unique_ptr<ClonableMemoryObject>>
      clonableMemoryLocations;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_MEMORY_CLONING_ANALYSIS_MEMORYCLONINGANALYSIS_H_
