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
#ifndef NOELLE_SRC_CORE_LOOP_SCCDAG_ATTRIBUTES_SCCDAGATTRS_H_
#define NOELLE_SRC_CORE_LOOP_SCCDAG_ATTRIBUTES_SCCDAGATTRS_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/SCCDAG.hpp"
#include "arcana/noelle/core/SCC.hpp"
#include "arcana/noelle/core/GenericSCC.hpp"
#include "arcana/noelle/core/LoopCarriedSCC.hpp"
#include "arcana/noelle/core/InductionVariables.hpp"
#include "arcana/noelle/core/PeriodicVariableSCC.hpp"
#include "arcana/noelle/core/LoopGoverningInductionVariable.hpp"
#include "arcana/noelle/core/LoopEnvironment.hpp"
#include "arcana/noelle/core/Variable.hpp"
#include "arcana/noelle/core/MemoryCloningAnalysis.hpp"

namespace arcana::noelle {

class SCCDAGAttrs {
public:
  SCCDAGAttrs(bool enableFloatAsReal,
              PDG *loopDG,
              SCCDAG *loopSCCDAG,
              LoopTree *loopNode,
              InductionVariableManager &IV,
              DominatorSummary &DS);

  SCCDAGAttrs() = delete;

  /*
   * Methods on SCCDAG.
   */
  std::set<LoopCarriedSCC *> getSCCsWithLoopCarriedDependencies(void) const;
  std::set<LoopCarriedSCC *> getSCCsWithLoopCarriedDataDependencies(void) const;
  std::set<LoopCarriedSCC *> getSCCsWithLoopCarriedControlDependencies(
      void) const;
  std::unordered_set<GenericSCC *> getSCCsOfKind(GenericSCC::SCCKind K);
  bool isLoopGovernedBySCC(SCC *scc) const;
  std::set<uint32_t> getLiveOutVariablesThatAreNotReducable(
      LoopEnvironment *env) const;

  /*
   * Methods on single SCC.
   */
  bool isSCCContainedInSubloop(LoopTree *loop, SCC *scc) const;
  GenericSCC *getSCCAttrs(SCC *scc) const;

  /*
   * Return the SCCDAG of the loop.
   */
  SCCDAG *getSCCDAG(void) const;
  std::pair<std::unordered_map<SCC *, std::unordered_set<SCC *>>,
            std::unordered_map<SCC *, std::unordered_set<DGEdge<SCC, SCC> *>>>
  computeSCCDAGWhenSCCsAreIgnored(
      std::function<bool(GenericSCC *)> ignoreSCC) const;

  /*
   * Debug methods
   */
  void dumpToFile(uint64_t id);

  ~SCCDAGAttrs();

private:
  std::map<SCC *, std::set<DGEdge<Value, Value> *>>
      sccToLoopCarriedDependencies;
  bool enableFloatAsReal;
  std::unordered_map<SCC *, GenericSCC *> sccToInfo;
  PDG *loopDG;
  SCCDAG *sccdag; /* SCCDAG of the related loop.  */
  MemoryCloningAnalysis *memoryCloningAnalysis;

  /*
   * Helper methods on SCCDAG
   */
  void collectLoopCarriedDependencies(LoopTree *loopNode);

  /*
   * Helper methods on single SCC
   */
  LoopCarriedVariable *checkIfReducible(SCC *scc, LoopTree *loop);

  std::tuple<bool, Value *, Value *, Value *, Value *> checkIfPeriodic(
      SCC *scc,
      LoopTree *loopNode);

  bool checkIfIndependent(SCC *scc);

  std::set<InductionVariable *> checkIfSCCOnlyContainsInductionVariables(
      SCC *scc,
      LoopTree *loop,
      std::set<InductionVariable *> &loopGoverningIVs,
      std::set<InductionVariable *> &IVs) const;

  std::set<ClonableMemoryObject *> checkIfClonableByUsingLocalMemory(
      SCC *scc,
      LoopTree *loop) const;

  std::set<Instruction *> checkIfRecomputable(SCC *scc,
                                              LoopTree *loopNode) const;

  bool isClonableByInductionVars(SCC *scc) const;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_LOOP_SCCDAG_ATTRIBUTES_SCCDAGATTRS_H_
