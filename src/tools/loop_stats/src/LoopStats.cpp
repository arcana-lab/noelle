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
#include "arcana/noelle/tools/LoopStats.hpp"
#include "arcana/noelle/core/PDGPrinter.hpp"

namespace arcana::noelle {

PreservedAnalyses LoopStats::run(Module &M, llvm::ModuleAnalysisManager &AM) {

  /*
   * Fetch noelle.
   */
  auto &noelle = AM.getResult<NoellePass>(M);
  if (noelle.getVerbosity() > Verbosity::Disabled) {
    errs() << "LoopStats: Start\n";
  }

  /*
   * Fetch all program loops.
   */
  auto programLoops = noelle.getLoopContents();

  /*
   * Analyze the loops.
   */
  this->collectStatsForLoops(noelle, M, *programLoops, AM);

  /*
   * Free the memory.
   */
  delete programLoops;

  if (noelle.getVerbosity() > Verbosity::Disabled) {
    errs() << "LoopStats: Exit\n";
  }

  return PreservedAnalyses::all();
}

void LoopStats::collectStatsForLoops(Noelle &noelle,
                                     Module &M,
                                     std::vector<LoopContent *> const &loops,
                                     llvm::ModuleAnalysisManager &AM) {

  /*
   * Fetch the profiles.
   */
  auto profiles = noelle.getProfiles();
  if (!profiles->isAvailable()) {
    errs() << "LoopStats: WARNING: the profiles are not available\n";
  }

  /*
   * Collect statistics about each loop using noelle's abstractions.
   */
  auto &fam = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  for (auto loop : loops) {
    if (noelle.getVerbosity() > Verbosity::Disabled) {
      errs() << "LoopStats: Collecting stats for: \n";
    }

    auto loopStructure = loop->getLoopStructure();
    auto loopHeader = loopStructure->getHeader();
    auto loopFunction = loopStructure->getFunction();

    auto &LI = fam.getResult<LoopAnalysis>(*loopFunction);
    auto llvmLoop = LI.getLoopFor(loopHeader);

    collectStatsForLoop(profiles, M, AM, *loop, *llvmLoop);
  }

  /*
   * TODO: Construct dependence graph using only LLVM APIs
   */
  for (auto loopContent : loops) {
    auto loopStructure = loopContent->getLoopStructure();
    auto loopHeader = loopStructure->getHeader();
    auto loopFunction = loopStructure->getFunction();

    auto loopIDOpt = loopStructure->getID();
    assert(loopIDOpt); // ED: we are collecting loop statistics, loops should
                       // have IDs so we can distinguish them.
    auto loopID = loopIDOpt.value();
    auto &SE = fam.getResult<ScalarEvolutionAnalysis>(*loopFunction);
    auto &LI = fam.getResult<LoopAnalysis>(*loopFunction);
    auto llvmLoop = LI.getLoopFor(loopHeader);
    auto loopDG = loopContent->getLoopDG();
    collectStatsForLoop(profiles, loopID, SE, loopDG, *llvmLoop);
  }

  /*
   * Print the statistics.
   */
  printStatsHumanReadable(profiles);

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

// Next there is code to register your pass to "opt"
llvm::PassPluginLibraryInfo getPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION,
    "LoopStats",
    LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      /*
       * REGISTRATION FOR "opt -passes='LoopStats'"
       *
       */
      PB.registerPipelineParsingCallback(
          [](StringRef Name,
             llvm::ModulePassManager &PM,
             ArrayRef<llvm::PassBuilder::PipelineElement>) {
            if (Name == "LoopStats") {
              PM.addPass(LoopStats());
              return true;
            }
            return false;
          });

      /*
       * REGISTRATION FOR "AM.getResult<NoellePass>()"
       */
      PB.registerAnalysisRegistrationCallback([](ModuleAnalysisManager &AM) {
        AM.registerPass([&] { return NoellePass(); });
      });
      PB.registerAnalysisRegistrationCallback([](FunctionAnalysisManager &AM) {
        AM.registerPass([&] { return LoopAnalysis(); });
        AM.registerPass([&] { return ScalarEvolutionAnalysis(); });
      });
    }
  };
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getPluginInfo();
}

} // namespace arcana::noelle
