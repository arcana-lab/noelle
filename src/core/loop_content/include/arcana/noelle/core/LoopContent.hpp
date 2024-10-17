/*
 * Copyright 2016 - 2023  Angelo Matni, Simone Campanoni, Brian Homerding
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
#ifndef NOELLE_SRC_CORE_LOOP_CONTENT_LOOPDEPENDENCEINFO_H_
#define NOELLE_SRC_CORE_LOOP_CONTENT_LOOPDEPENDENCEINFO_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/CompilationOptionsManager.hpp"
#include "arcana/noelle/core/PDG.hpp"
#include "arcana/noelle/core/SCCDAG.hpp"
#include "arcana/noelle/core/InductionVariables.hpp"
#include "arcana/noelle/core/Invariants.hpp"
#include "arcana/noelle/core/LoopGoverningInductionVariable.hpp"
#include "arcana/noelle/core/LoopEnvironment.hpp"
#include "arcana/noelle/core/LoopEnvironmentBuilder.hpp"
#include "arcana/noelle/core/SCCDAGAttrs.hpp"
#include "arcana/noelle/core/LoopIterationSpaceAnalysis.hpp"
#include "arcana/noelle/core/LoopTransformationsOptions.hpp"
#include "arcana/noelle/core/LDGGenerator.hpp"

namespace arcana::noelle {

class LoopContent {
public:
  /*
   * Constructors.
   */
  LoopContent(LDGGenerator &ldgGenerator,
              CompilationOptionsManager *compilationOptionsManager,
              PDG *fG,
              LoopTree *loopNode,
              Loop *l,
              DominatorSummary &DS,
              ScalarEvolution &SE);

  LoopContent(LDGGenerator &ldgGenerator,
              CompilationOptionsManager *compilationOptionsManager,
              PDG *fG,
              LoopTree *loopNode,
              Loop *l,
              DominatorSummary &DS,
              ScalarEvolution &SE,
              uint32_t maxCores);

  LoopContent(LDGGenerator &ldgGenerator,
              CompilationOptionsManager *compilationOptionsManager,
              PDG *fG,
              LoopTree *loopNode,
              Loop *l,
              DominatorSummary &DS,
              ScalarEvolution &SE,
              uint32_t maxCores,
              std::unordered_set<LoopContentOptimization> optimizations);

  LoopContent(LDGGenerator &ldgGenerator,
              CompilationOptionsManager *compilationOptionsManager,
              PDG *fG,
              LoopTree *loopNode,
              Loop *l,
              DominatorSummary &DS,
              ScalarEvolution &SE,
              uint32_t maxCores,
              bool enableLoopAwareDependenceAnalyses);

  LoopContent(LDGGenerator &ldgGenerator,
              CompilationOptionsManager *compilationOptionsManager,
              PDG *fG,
              LoopTree *loop,
              Loop *l,
              DominatorSummary &DS,
              ScalarEvolution &SE,
              uint32_t maxCores,
              std::unordered_set<LoopContentOptimization> optimizations,
              bool enableLoopAwareDependenceAnalyses);

  LoopContent(LDGGenerator &ldgGenerator,
              CompilationOptionsManager *compilationOptionsManager,
              PDG *fG,
              LoopTree *loop,
              Loop *l,
              DominatorSummary &DS,
              ScalarEvolution &SE,
              uint32_t maxCores,
              std::unordered_set<LoopContentOptimization> optimizations,
              bool enableLoopAwareDependenceAnalyses,
              uint32_t chunkSize);

  LoopContent() = delete;

  /*
   * Return the object containing all loop structures at and nested within this
   * loop
   */
  LoopTree *getLoopHierarchyStructures(void) const;

  /*
   * Return the object that describes the loop in terms of induction variables,
   * trip count, and control structure (e.g., latches, header)
   */
  LoopStructure *getLoopStructure(void) const;

  /*
   * Return the nested-most loop for this instruction
   */
  LoopStructure *getNestedMostLoopStructure(Instruction *I) const;

  /*
   * Get the dependence graph of the loop.
   */
  PDG *getLoopDG(void) const;

  /*
   * Copy all options from otherLC to "this".
   */
  void copyParallelizationOptionsFrom(LoopContent *otherLC);

  /*
   * Iterate over children of "this" recursively following the loop nesting tree
   * rooted by "this". This will go through children of children etc...
   */
  bool iterateOverSubLoopsRecursively(
      std::function<bool(const LoopStructure &child)> funcToInvoke);

  InductionVariableManager *getInductionVariableManager(void) const;

  SCCDAGAttrs *getSCCManager(void) const;

  InvariantManager *getInvariantManager(void) const;

  LoopTransformationsManager *getLoopTransformationsManager(void) const;

  LoopEnvironment *getEnvironment(void) const;

  LoopIterationSpaceAnalysis *getLoopIterationSpaceAnalysis(void) const;

  MemoryCloningAnalysis *getMemoryCloningAnalysis(void) const;

  bool doesHaveCompileTimeKnownTripCount(void) const;

  uint64_t getCompileTimeTripCount(void) const;

  /*
   * Deconstructor.
   */
  ~LoopContent();

private:
  /*
   * Fields
   */
  LoopTree *loop;

  LoopEnvironment *environment;

  PDG *loopDG; /* Dependence graph of the loop.
                * This graph does not include instructions outside the loop
                * (i.e., no external dependences are included).
                */

  InductionVariableManager *inductionVariables;

  InvariantManager *invariantManager;

  LoopIterationSpaceAnalysis *domainSpaceAnalysis;

  MemoryCloningAnalysis *memoryCloningAnalysis;

  bool compileTimeKnownTripCount;

  uint64_t tripCount;

  SCCDAGAttrs *sccdagAttrs;

  LoopTransformationsManager *loopTransformationsManager;

  CompilationOptionsManager *com;

  /*
   * Methods
   */
  void fetchLoopAndBBInfo(Loop *l, ScalarEvolution &SE);

  std::pair<PDG *, SCCDAG *> createDGsForLoop(LDGGenerator &ldgGenerator,
                                              CompilationOptionsManager *com,
                                              Loop *l,
                                              LoopTree *loopNode,
                                              PDG *functionDG,
                                              DominatorSummary &DS,
                                              ScalarEvolution &SE);

  uint64_t computeTripCounts(Loop *l, ScalarEvolution &SE);

  void removeUnnecessaryDependenciesThatCloningMemoryNegates(
      LoopTree *loopNode,
      PDG *loopInternalDG,
      DominatorSummary &DS);

  void removeUnnecessaryDependenciesWithThreadSafeLibraryFunctions(
      LoopTree *loopNode,
      PDG *loopDG,
      DominatorSummary &DS);
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_LOOP_CONTENT_LOOPDEPENDENCEINFO_H_
