/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni, Brian Homerding
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
#ifndef NOELLE_SRC_CORE_LOOP_SCCDAG_NORMALIZER_SCCDAGNORMALIZER_H_
#define NOELLE_SRC_CORE_LOOP_SCCDAG_NORMALIZER_SCCDAGNORMALIZER_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/PDG.hpp"
#include "arcana/noelle/core/SCCDAG.hpp"
#include "arcana/noelle/core/SCCDAGPartition.hpp"
#include "arcana/noelle/core/LoopForest.hpp"

namespace arcana::noelle {

class SCCDAGNormalizer {
public:
  SCCDAGNormalizer(SCCDAG &dag, LoopTree *loop);

  SCCDAGNormalizer() = delete;

  void normalizeInPlace(void);

private:
  LoopTree *loop;
  SCCDAG &sccdag;

  void mergeLCSSAPhis(void);
  void mergeSCCsWithExternalInterIterationDependencies(void);
  void mergeSingleSyntacticSugarInstrs(void);
  void mergeBranchesWithoutOutgoingEdges(void);

  class MergeGroups {
  public:
    std::unordered_map<DGNode<SCC> *, std::set<DGNode<SCC> *> *> sccToGroupMap;
    std::set<std::set<DGNode<SCC> *> *> groups;

    MergeGroups() : sccToGroupMap{} {}
    ~MergeGroups();

    void merge(DGNode<SCC> *sccNode1, DGNode<SCC> *sccNode2);
  };

  /*
   * TODO: Refactor with similar logic in SCCDAGPartition
   */
  void collapseIntroducedCycles(void);
};
} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_LOOP_SCCDAG_NORMALIZER_SCCDAGNORMALIZER_H_
