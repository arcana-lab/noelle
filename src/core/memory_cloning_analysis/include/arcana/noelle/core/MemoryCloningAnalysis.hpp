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

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/PDG.hpp"
#include "arcana/noelle/core/SCCDAG.hpp"
#include "arcana/noelle/core/Invariants.hpp"
#include "arcana/noelle/core/Dominators.hpp"
#include "arcana/noelle/core/ClonableMemoryObject.hpp"
#include "arcana/noelle/core/Lumberjack.hpp"

namespace arcana::noelle {

class MemoryCloningAnalysis {
public:
  MemoryCloningAnalysis(LoopStructure *loop, DominatorSummary &DS, PDG *ldg);

  const std::unordered_set<ClonableMemoryObject *> getClonableMemoryObjectsFor(
      Instruction *I) const;

  std::unordered_set<ClonableMemoryObject *> getClonableMemoryObjects(
      void) const;

private:
  Logger log;
  std::unordered_set<std::unique_ptr<ClonableMemoryObject>>
      clonableMemoryLocations;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_MEMORY_CLONING_ANALYSIS_MEMORYCLONINGANALYSIS_H_
