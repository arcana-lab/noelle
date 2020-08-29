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

static double updateMovingAverage (double avg, double newValue, double n, double loopCoverage) {
  double newAvg = (newValue + ((double)n) * avg) / (n + loopCoverage);

  return newAvg;
}

void LoopStats::printPerLoopStats (Hot *profiles, Stats *stats) {
  if (stats->loopID != -1){
    errs() << "  Loop: " << stats->loopID << "\n";
  }
  errs() << "    Induction variables (IVs):\n";
  errs() << "      Number of IVs: " << stats->numberOfIVs << "\n";
  errs() << "      Number of dynamic IVs: " << stats->numberOfDynamicIVs << "\n";
  errs() << "      Number of loop governing IVs: " << stats->isGovernedByIV << "\n";
  errs() << "      Number of dynamic loop governing IVs: " << stats->numberOfDynamicGovernedIVs << "\n";
  errs() << "    Invariants\n";
  errs() << "      Number of invariants: " << stats->numberOfInvariants << "\n";
  errs() << "      Number of dynamic invariants: " << stats->numberOfDynamicInvariants << "\n";
  errs() << "    Dependences\n";
  errs() << "      Number of nodes in SCCDAG: " << stats->numberOfNodesInSCCDAG << "\n";
  errs() << "      Number of SCCs: " << stats->numberOfSCCs << "\n";
  errs() << "      Number of sequential SCCs: " << stats->numberOfSequentialSCCs << "\n";
  errs() << "      Number of dynamic instructions executed in sequential SCCs: " << stats->dynamicInstructionsOfSequentialSCCs << "\n";

  return ;
}

void LoopStats::printStatsHumanReadable (Hot *profiles) {
  Stats totalInfoNoelle{};
  Stats totalInfoLLVM{};

  /*
   * Compute the coverage of all loops.
   */
  uint64_t loopsDynamicInstructions = 0;
  for (auto idAndNoelleLoop : statsByLoopAccordingToNoelle) {
    auto noelleStats = idAndNoelleLoop.second;
    loopsDynamicInstructions += noelleStats->dynamicTotalInstructions;
  }

  /*
   * Print the loop statistics.
   */
  errs() << "Per loop statistics\n";
  double totalSequentialSCCNoelle = 0;
  double totalSequentialSCCLLVM = 0;
  double n = 0;
  for (auto idAndNoelleLoop : statsByLoopAccordingToNoelle) {

    /*
     * Fetch the loop information.
     */
    auto id = idAndNoelleLoop.first;
    auto noelleStats = idAndNoelleLoop.second;
    auto llvmStats = statsByLoopAccordingToLLVM.at(id);
    //assert(noelleStats->dynamicTotalInstructions == llvmStats->dynamicTotalInstructions); FIXME this assertion currently fails

    /*
     * Print the per loop statistics.
     */
    errs() << " Noelle:\n";
    printPerLoopStats(profiles, noelleStats);
    errs() << " LLVM:\n";
    printPerLoopStats(profiles, llvmStats);

    /*
     * Compute the coverage of the current loop compared to the total coverage of all the loops.
     */
    auto currentLoopCoverageOverAllLoops = ((double)noelleStats->dynamicTotalInstructions) / ((double)loopsDynamicInstructions);

    /*
     * Update the total statistics.
     */
    totalInfoNoelle = totalInfoNoelle + *noelleStats;
    totalInfoLLVM = totalInfoLLVM + *llvmStats;
    totalSequentialSCCNoelle += (((double)noelleStats->dynamicInstructionsOfSequentialSCCs) * currentLoopCoverageOverAllLoops);
    totalSequentialSCCLLVM += (((double)llvmStats->dynamicInstructionsOfSequentialSCCs) * currentLoopCoverageOverAllLoops);
    n++;
  }
  auto averageSequentialSCCNoelle = totalSequentialSCCNoelle / ((double)n);
  auto averageSequentialSCCLLVM = totalSequentialSCCLLVM / ((double)n);

  /*
   * Print the total statistics.
   */
  errs() << "Total statistics\n";
  errs() << " Noelle:\n";
  printPerLoopStats(profiles, &totalInfoNoelle);
  errs() << "      Average number of dynamic instructions of sequential SCCs: " << averageSequentialSCCNoelle << "\n";
  errs() << "      Average coverage of sequential SCCs: " << (averageSequentialSCCNoelle / ((double)profiles->getTotalInstructions())) << " %\n";
  errs() << " LLVM:\n";
  printPerLoopStats(profiles, &totalInfoLLVM);
  errs() << "      Average number of dynamic instructions of sequential SCCs: " << averageSequentialSCCLLVM << "\n";
  errs() << "      Average coverage of sequential SCCs: " << (averageSequentialSCCLLVM / ((double)profiles->getTotalInstructions())) << " %\n";

  return;
}
