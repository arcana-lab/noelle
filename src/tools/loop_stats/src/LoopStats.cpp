/*
 * Copyright 2019 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopStats.hpp"

#include "PDGPrinter.hpp"

using namespace llvm;
using namespace llvm::noelle;
    
void LoopStats::collectStatsForLoops (Noelle &noelle, std::vector<LoopDependenceInfo *> const & loops){

  /*
   * Fetch the profiles.
   */
  auto profiles = noelle.getProfiles();
  if (!profiles->isAvailable()){
    errs() << "LoopStats: WARNING: the profiles are not available\n";
  }

  /*
   * Collect statistics about each loop using noelle's abstractions.
   */
  for (auto loop : loops) {
    if (noelle.getVerbosity() > Verbosity::Disabled) {
      errs() << "LoopStats: Collecting stats for: \n";
    }
    collectStatsForLoop(profiles, *loop);
  }

  /*
   * TODO: Construct dependence graph using only LLVM APIs
   */
  for (auto LDI : loops) {
    auto loopStructure = LDI->getLoopStructure();
    auto loopHeader = loopStructure->getHeader();
    auto loopFunction = loopStructure->getFunction();

    auto id = loopStructure->getID();
    auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*loopFunction).getSE();
    auto &LI = getAnalysis<LoopInfoWrapperPass>(*loopFunction).getLoopInfo();
    auto llvmLoop = LI.getLoopFor(loopHeader);
    auto loopDG = LDI->getLoopDG();

    collectStatsForLoop(profiles, id, SE, loopDG, *llvmLoop);
  }

  /*
   * Print the statistics.
   */
  printStatsHumanReadable(profiles);

  return ;
}

LoopStats::LoopStats()
  : ModulePass{ID} {
  return;
}

LoopStats::~LoopStats() {

  for (auto loopAndStats : statsByLoopAccordingToLLVM) {
    delete loopAndStats.second;
  }
  for (auto loopAndStats : statsByLoopAccordingToNoelle) {
    delete loopAndStats.second;
  }
  statsByLoopAccordingToLLVM.clear();
  statsByLoopAccordingToNoelle.clear();

  return;
}
