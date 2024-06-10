/*
 * Copyright 2022 - 2024  Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_LDG_GENERATOR_LDGANALYSIS_H_
#define NOELLE_SRC_CORE_LDG_GENERATOR_LDGANALYSIS_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/CompilationOptionsManager.hpp"
#include "arcana/noelle/core/DependenceAnalysis.hpp"
#include "arcana/noelle/core/PDG.hpp"
#include "arcana/noelle/core/LoopStructure.hpp"
#include "noelle/core/InductionVariables.hpp"
#include "noelle/core/AliasAnalysisEngine.hpp"

namespace arcana::noelle {

class LDGGenerator {
public:
  LDGGenerator();

  void addAnalysis(DependenceAnalysis *a);

  bool areLoopDependenceAnalysesEnabled(void) const;

  void enableLoopDependenceAnalyses(bool enabled);

  PDG *generateLoopDependenceGraph(PDG *functionDG,
                                   ScalarEvolution &scalarEvolution,
                                   DominatorSummary &DS,
                                   CompilationOptionsManager *com,
                                   Loop *l,
                                   LoopTree &loopNode);

  SCCDAG *computeSCCDAGWithOnlyVariableAndControlDependences(PDG *loopDG);

  static std::set<AliasAnalysisEngine *> getLoopAliasAnalysisEngines(void);

private:
  std::set<DependenceAnalysis *> ddAnalyses;
  bool loopDependenceAnalysesEnabled;

  void removeDependences(PDG *loopDG, LoopStructure *loop);
  void removeLoopCarriedDependences(PDG *loopDG, LoopStructure *loop);
  void runAffineAnalysis(PDG &loopDG,
                         ScalarEvolution &scalarEvolution,
                         InductionVariableManager &ivManager,
                         LoopTree &loopNode);
  void improveDependenceGraph(PDG *loopDG, LoopStructure *loop);
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_LDG_GENERATOR_LDGANALYSIS_H_
