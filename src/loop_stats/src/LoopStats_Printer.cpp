/*
 * Copyright 2019 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopStats.hpp"

using namespace llvm;

void LoopStats::printPerLoopStats (Stats *stats) {
  if (stats->loopID != -1){
    errs() << "  Loop: " << stats->loopID << "\n";
  }
  errs() << "    Induction variables (IVs):\n";
  errs() << "      Number of IVs: " << stats->numberOfIVs << "\n";
  errs() << "      Number of dynamic IVs: " << stats->numberOfDynamicIVs << "\n";
  errs() << "      Has loop governing IV: " << stats->isGovernedByIV << "\n";
  errs() << "    Invariants\n";
  errs() << "      Number of invariants contained within loop: " << stats->numberOfInvariantsContainedWithinTheLoop << "\n";
  errs() << "    Dependences\n";
  errs() << "      Number of nodes in SCCDAG: " << stats->numberOfNodesInSCCDAG << "\n";
  errs() << "      Number of SCCs: " << stats->numberOfSCCs << "\n";
  errs() << "      Number of sequential SCCs: " << stats->numberOfSequentialSCCs << "\n";
}

void LoopStats::printStatsHumanReadable (void) {
  Stats totalInfoNoelle{};
  Stats totalInfoLLVM{};

  errs() << "Per loop statistics\n";
  for (auto idAndNoelleLoop : statsByLoopAccordingToNoelle) {

    /*
     * Fetch the loop information.
     */
    auto id = idAndNoelleLoop.first;
    auto noelleStats = idAndNoelleLoop.second;
    auto llvmStats = statsByLoopAccordingToLLVM.at(id);

    /*
     * Print the per loop statistics.
     */
    errs() << " Noelle:\n";
    printPerLoopStats(noelleStats);
    errs() << " LLVM:\n";
    printPerLoopStats(llvmStats);

    /*
     * Update the total statistics.
     */
    totalInfoNoelle = totalInfoNoelle + *noelleStats;
    totalInfoLLVM = totalInfoLLVM + *llvmStats;
  }

  /*
   * Print the total statistics.
   */
  errs() << "Total statistics\n";
  errs() << " Noelle:\n";
  printPerLoopStats(&totalInfoNoelle);
  errs() << " LLVM:\n";
  printPerLoopStats(&totalInfoLLVM);

  return;
}
