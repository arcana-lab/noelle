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
#include "AutotunerDoallFilter.hpp"

#include <cstdlib>
#include <fstream>

namespace arcana::noelle {

AutotunerDoallFilter::AutotunerDoallFilter() : ModulePass(this->ID) {}

bool AutotunerDoallFilter::doInitialization(Module &M) {
  return false;
}

bool AutotunerDoallFilter::runOnModule(Module &M) {
  errs() << "AutotunerDoallFilter: Start\n";

  /*
   * Fetch noelle.
   */
  auto &noelle = getAnalysis<Noelle>();

  /*
   * Get autotuner_space.info file name
   */
  char *fileName = getenv("autotunerSPACE_FILE");
  if (fileName == nullptr) {
    errs()
        << "ERROR: autotuner_doall_filter, no autotunerSPACE_FILE env var specified. Abort.\n";
    abort();
  }

  /*
   * Write autotuner space file (i.e., number of loops that makes sense to
   * parallelize)
   */
  ofstream file;
  file.open(fileName);

  /*
   * Get the maximum number of cores we are allowed to use.
   * This will set the corresponding autotuner space parameter.
   */
  CompilationOptionsManager *com = noelle.getCompilationOptionsManager();
  uint32_t maxNumCores = com->getMaximumNumberOfCores();

  /*
   * Meaning (per index):
   * 0: disable/enable loop,
   * 1: unrool factor (NOT USED right now),
   * 2: peel factor (NOT USED right now),
   * 3: techniques to disable (0: None, 1: DSWP, 2: HELIX, 3: DOALL, 4: DSWP
   * HELIX, 5: DSWP DOALL, 6: HELIX DOALL), we translate this dimension for the
   * autotuner with 3 possible choices to select (not disable) a technique (0:
   * DOALL, 1: HELIX, 2: DSWP) which correspond to indexes 4, 5, 6 respectively
   * 4: number of cores to use,
   * 5: chunk factor (only useful if selected technique is DOALL),
   * 6: unknown (NOT USED right now),
   * 7: unknown (NOT USED right now),
   * 8: unknown (NOT USED right now)
   */
  std::string enabledNotDOALLLoopString =
      "2 0 0 3 " + std::to_string(maxNumCores) + " 0 0 0 0 \n";
  std::string enabledDOALLLoopString = "2 0 0 1 " + std::to_string(maxNumCores)
                                       + " 8 0 0 0\n"; // Enable only DOALL

  // Get loop structures with 0.0 min hotness to ensure we get all loops
  std::vector<LoopStructure *> *loopStructures = noelle.getLoopStructures(0.0);
  DOALL doall{ noelle };
  for (auto loopStructure : *loopStructures) {
    auto loopIDOpt = loopStructure->getID();
    assert(loopIDOpt);
    uint64_t loopID = loopIDOpt.value();

    LoopContent *loopDependenceInfo = noelle.getLoopContent(loopStructure);

    std::string enabledLoopStringWithLoopID = std::to_string(loopID) + " ";
    if (doall.canBeAppliedToLoop(loopDependenceInfo, nullptr)) {
      enabledLoopStringWithLoopID += enabledDOALLLoopString;
    } else {
      enabledLoopStringWithLoopID += enabledNotDOALLLoopString;
    }

    file << enabledLoopStringWithLoopID;
  }

  delete loopStructures;

  file.close();

  return false;
}

void AutotunerDoallFilter::getAnalysisUsage(AnalysisUsage &AU) const {
  /*
   * Noelle.
   */
  AU.addRequired<Noelle>();

  return;
}

} // namespace arcana::noelle

// Next there is code to register your pass to "opt"
char arcana::noelle::AutotunerDoallFilter::ID = 0;
static RegisterPass<arcana::noelle::AutotunerDoallFilter> X(
    "autotunerdoallfilter",
    "Reduce search space of DOALL loops.");

// Next there is code to register your pass to "clang"
static arcana::noelle::AutotunerDoallFilter *_PassMaker = NULL;
static RegisterStandardPasses _RegPass1(
    PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
      if (!_PassMaker) {
        PM.add(_PassMaker = new arcana::noelle::AutotunerDoallFilter());
      }
    }); // ** for -Ox
static RegisterStandardPasses _RegPass2(
    PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
      if (!_PassMaker) {
        PM.add(_PassMaker = new arcana::noelle::AutotunerDoallFilter());
      }
    }); // ** for -O0
