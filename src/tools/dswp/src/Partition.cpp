/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "DSWP.hpp"

using namespace llvm;
using namespace llvm::noelle;

void DSWP::partitionSCCDAG (LoopDependenceInfo *LDI, Heuristics *h) {

  /*
   * Prepare the initial partition.
   */
  ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences::partitionSCCDAG(LDI);

  /*
   * Print the initial partitions.
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DSWP:  Before partitioning the SCCDAG\n";
    // partitioner->getPartitionGraph()->print(errs());
  }

  /*
   * NOTE: To prevent queues pushing/popping data unevenly, merge all LCSSA PHIs
   * with their incoming values so that the value produced by that partitioned set is
   * at the same loop nesting level as the consumer of that LCSSA PHI
   *
   * NOTE: For memory dependencies, synchronization needs to be in place; until then,
   * merge all memory dependencies into single sets
   */
  // partitioner->mergeLCSSAPhisWithTheValuesTheyPropagate();
  partitioner->mergeAlongMemoryEdges();

  /*
   * Check if we can cluster SCCs.
   */
  if (this->enableMergingSCC) {

    /*
     * Fetch the loop transformation manager
     */
    auto ltm = LDI->getLoopTransformationsManager();

    /*
     * Decide the partition of the SCCDAG by merging the trivial partitions defined above.
     */
    h->adjustParallelizationPartitionForDSWP(
      partitioner,
      *LDI->getSCCManager(),
      ltm->getMaximumNumberOfCores(),
      this->verbose
    );
  }

  /*
   * Print the partitioned SCCDAG.
   */
  if (this->verbose >= Verbosity::Minimal) {
    errs() << "DSWP:  Final number of partitions: " << this->partitioner->numberOfPartitions() << "\n";
  }
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "DSWP:  After partitioning the SCCDAG\n";
    // partitioner->getPartitionGraph()->print(errs());
  }

  return ;
}
