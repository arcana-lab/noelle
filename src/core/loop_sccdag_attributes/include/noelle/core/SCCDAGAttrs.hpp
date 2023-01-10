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
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/SCCDAG.hpp"
#include "noelle/core/SCC.hpp"
#include "noelle/core/SCCAttrs.hpp"
#include "noelle/core/LoopCarriedSCC.hpp"
#include "noelle/core/InductionVariables.hpp"
#include "noelle/core/LoopGoverningIVAttribution.hpp"
#include "noelle/core/LoopEnvironment.hpp"
#include "noelle/core/Variable.hpp"
#include "noelle/core/MemoryCloningAnalysis.hpp"

namespace llvm::noelle {

class SCCDAGAttrs {
public:
  SCCDAGAttrs(bool enableFloatAsReal,
              PDG *loopDG,
              SCCDAG *loopSCCDAG,
              LoopForestNode *loopNode,
              InductionVariableManager &IV,
              DominatorSummary &DS);

  SCCDAGAttrs() = delete;

  /*
   * Isolated clonable SCCs and resulting inherited parents
   */
  std::unordered_map<SCC *, std::unordered_set<SCC *>> parentsViaClones;
  std::unordered_map<SCC *, std::unordered_set<DGEdge<SCC> *>> edgesViaClones;

  /*
   * Methods on SCCDAG.
   */
  std::set<LoopCarriedSCC *> getSCCsWithLoopCarriedDependencies(void) const;
  std::set<LoopCarriedSCC *> getSCCsWithLoopCarriedDataDependencies(void) const;
  std::set<LoopCarriedSCC *> getSCCsWithLoopCarriedControlDependencies(
      void) const;
  std::unordered_set<SCCAttrs *> getSCCsOfKind(SCCAttrs::SCCKind K);
  bool isLoopGovernedBySCC(SCC *scc) const;
  std::set<uint32_t> getLiveOutVariablesThatAreNotReducable(
      LoopEnvironment *env) const;

  /*
   * Methods on single SCC.
   */
  bool isSCCContainedInSubloop(LoopForestNode *loop, SCC *scc) const;
  SCCAttrs *getSCCAttrs(SCC *scc) const;

  /*
   * Return the SCCDAG of the loop.
   */
  SCCDAG *getSCCDAG(void) const;

  /*
   * Debug methods
   */
  void dumpToFile(int id);

  ~SCCDAGAttrs();

private:
  std::map<SCC *, Criticisms> sccToLoopCarriedDependencies;
  bool enableFloatAsReal;
  std::unordered_map<SCC *, SCCAttrs *> sccToInfo;
  PDG *loopDG;
  SCCDAG *sccdag; /* SCCDAG of the related loop.  */
  MemoryCloningAnalysis *memoryCloningAnalysis;

  /*
   * Helper methods on SCCDAG
   */
  void collectSCCGraphAssumingDistributedClones();
  void collectLoopCarriedDependencies(LoopForestNode *loopNode);

  /*
   * Helper methods on single SCC
   */
  LoopCarriedVariable *checkIfReducible(SCC *scc, LoopForestNode *loop);

  bool checkIfIndependent(SCC *scc);

  std::set<InductionVariable *> checkIfSCCOnlyContainsInductionVariables(
      SCC *scc,
      LoopForestNode *loop,
      std::set<InductionVariable *> &loopGoverningIVs,
      std::set<InductionVariable *> &IVs) const;

  void checkIfClonable(SCC *scc, LoopForestNode *loop);

  std::set<ClonableMemoryLocation *> checkIfClonableByUsingLocalMemory(
      SCC *scc,
      LoopForestNode *loop) const;

  std::set<Instruction *> checkIfRecomputable(SCC *scc,
                                              LoopForestNode *loopNode) const;

  bool isClonableByInductionVars(SCC *scc) const;
  bool isClonableBySyntacticSugarInstrs(SCC *scc) const;
  bool isClonableByCmpBrInstrs(SCC *scc) const;
  bool isClonableByHavingNoMemoryOrLoopCarriedDataDependencies(
      SCC *scc,
      LoopForestNode *loop) const;
};

} // namespace llvm::noelle
