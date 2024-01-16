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
#ifndef NOELLE_SRC_TOOLS_PARALLELIZATION_TECHNIQUE_PARALLELIZATIONTECHNIQUEFORLOOPSWITHLOOPCARRIEDDATADEPENDENCES_H_
#define NOELLE_SRC_TOOLS_PARALLELIZATION_TECHNIQUE_PARALLELIZATIONTECHNIQUEFORLOOPSWITHLOOPCARRIEDDATADEPENDENCES_H_

#include "noelle/core/SCCDAGPartition.hpp"
#include "noelle/tools/ParallelizationTechnique.hpp"

namespace arcana::noelle {

class ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences
  : public ParallelizationTechnique {
public:
  /*
   * Constructor.
   */
  ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences(
      Noelle &n,
      bool forceParallelization);

  bool canBeAppliedToLoop(LoopContent *loopContent,
                          Heuristics *h) const override;

  /*
   * Destructor.
   */
  ~ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences();

protected:
  /*
   * Fields
   */
  SCCDAGPartitioner *partitioner;
  bool forceParallelization;

  /*
   * Partition SCCDAG.
   */
  void partitionSCCDAG(LoopContent *loopContent);
  void partitionSCCDAG(LoopContent *loopContent,
                       std::function<bool(GenericSCC *scc)> skipSCC);

  void printSequentialCode(raw_ostream &stream,
                           const std::string &prefixString,
                           LoopContent *loopContent,
                           const std::set<SCC *> &sequentialSCCs);
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_TOOLS_PARALLELIZATION_TECHNIQUE_PARALLELIZATIONTECHNIQUEFORLOOPSWITHLOOPCARRIEDDATADEPENDENCES_H_
