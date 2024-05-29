/*
 * Copyright 2019 - 2024  Angelo Matni, Simone Campanoni
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
#include "noelle/tools/LoopStats.hpp"

namespace arcana::noelle {

void LoopStats::collectStatsForLoop(Hot *profiles,
                                    uint32_t id,
                                    ScalarEvolution &SE,
                                    PDG *loopDG,
                                    Loop &llvmLoop) {
  auto statsForLoop = new Stats();
  statsByLoopAccordingToLLVM.insert(std::make_pair(id, statsForLoop));
  statsForLoop->loopID = id;

  collectStatsOnLLVMIVs(profiles, SE, llvmLoop, statsForLoop);
  collectStatsOnLLVMInvariants(profiles, llvmLoop, statsForLoop);
  collectStatsOnLLVMSCCs(profiles, loopDG, statsForLoop);

  /*
   * Compute the coverage for the loop.
   */
  for (auto bb : llvmLoop.getBlocks()) {
    statsForLoop->dynamicTotalInstructions = profiles->getTotalInstructions(bb);
  }

  return;
}

void LoopStats::collectStatsForLoop(Hot *profiles,
                                    LoopContent &loopContent,
                                    Loop &llvmLoop) {
  auto loopStructure = loopContent.getLoopStructure();
  auto loopIDOpt = loopStructure->getID();
  assert(loopIDOpt); // ED: we are collecting loop statistics, loops should have
                     // IDs so we can distinguish them.
  auto loopID = loopIDOpt.value();

  auto statsForLoop = new Stats();
  statsByLoopAccordingToNoelle.insert(std::make_pair(loopID, statsForLoop));
  statsForLoop->loopID = loopID;

  collectStatsOnNoelleIVs(profiles, loopContent, statsForLoop);
  collectStatsOnNoelleSCCs(profiles, loopContent, statsForLoop, llvmLoop);
  collectStatsOnNoelleInvariants(profiles, loopContent, statsForLoop);

  /*
   * Compute the coverage for the loop.
   */
  statsForLoop->dynamicTotalInstructions =
      profiles->getTotalInstructions(loopStructure);

  return;
}

} // namespace arcana::noelle
