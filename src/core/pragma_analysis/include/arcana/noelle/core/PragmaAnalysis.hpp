/*
 * Copyright 2023 - 2024  Federico Sossai
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publoopsh, distribute, sublicense, and/or sell copies
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
#ifndef NOELLE_SRC_CORE_PRAGMA_ANALYSIS_PRAGMAANALYSIS_HPP_
#define NOELLE_SRC_CORE_PRAGMA_ANALYSIS_PRAGMAANALYSIS_HPP_

#include <unordered_set>

#include "arcana/noelle/core/DependenceAnalysis.hpp"
#include "arcana/noelle/core/PragmaForest.hpp"

namespace arcana::noelle {

class PragmaAnalysis : public DependenceAnalysis {
public:
  PragmaAnalysis();

  ~PragmaAnalysis();

  bool canThereBeAMemoryDataDependence(Instruction *fromInst,
                                       Instruction *toInst) override;

  bool canThereBeAMemoryDataDependence(Instruction *fromInst,
                                       Instruction *toInst,
                                       Function &function) override;

  bool canThereBeAMemoryDataDependence(Instruction *fromInst,
                                       Instruction *toInst,
                                       LoopStructure &loop) override;

  MemoryDataDependenceStrength isThereThisMemoryDataDependenceType(
      DataDependenceType t,
      Instruction *fromInst,
      Instruction *toInst) override;

  MemoryDataDependenceStrength isThereThisMemoryDataDependenceType(
      DataDependenceType t,
      Instruction *fromInst,
      Instruction *toInst,
      Function &function) override;

  MemoryDataDependenceStrength isThereThisMemoryDataDependenceType(
      DataDependenceType t,
      Instruction *fromInst,
      Instruction *toInst,
      LoopStructure &loop) override;

  bool canThisDependenceBeLoopCarried(DGEdge<Value, Value> *dep,
                                      LoopStructure &loop) override;

private:
  std::vector<StringRef> funcPrefixes;

  bool isPragma(Value *V) const;
};

} // namespace arcana::noelle

#endif
