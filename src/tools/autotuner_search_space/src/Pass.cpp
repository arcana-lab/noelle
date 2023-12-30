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
#include "AutotunerSearchSpace.hpp"

#include <cstdlib>
#include <fstream>
#include <string>

namespace arcana::noelle {

AutotunerSearchSpace::AutotunerSearchSpace() : ModulePass(this->ID) {}

bool AutotunerSearchSpace::doInitialization(Module &M) {
  return false;
}

bool AutotunerSearchSpace::runOnModule(Module &M) {
  errs() << "AutotunerSearchSpace: Start\n";

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
        << "ERROR: autotuner_search_space, no autotunerSPACE_FILE env var specified. Abort.\n";
    abort();
  }

  /*
   * Write autotuner space file (i.e., number of loops that makes sense to
   * parallelize)
   */
  std::ofstream file;
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
  std::string enabledLoopString =
      "2 0 0 3 " + std::to_string(maxNumCores) + " 8 0 0 0\n";

  // Get loop structures with 0.0 min hotness to ensure we get all loops
  std::vector<LoopStructure *> *loopStructures = noelle.getLoopStructures(0.0);
  for (auto loopStructure : *loopStructures) {
    auto loopIDOpt = loopStructure->getID();
    assert(loopIDOpt);
    uint64_t loopID = loopIDOpt.value();
    std::string enabledLoopStringWithLoopID =
        std::to_string(loopID) + " " + enabledLoopString;
    file << enabledLoopStringWithLoopID;
  }

  delete loopStructures;

  file.close();

  return false;
}

void AutotunerSearchSpace::getAnalysisUsage(AnalysisUsage &AU) const {
  /*
   * Noelle.
   */
  AU.addRequired<Noelle>();

  return;
}

} // namespace arcana::noelle

// Next there is code to register your pass to "opt"
char arcana::noelle::AutotunerSearchSpace::ID = 0;
static RegisterPass<arcana::noelle::AutotunerSearchSpace> X(
    "autotunersearchspace",
    "Generate file with number of loops that can be parallelized");

// Next there is code to register your pass to "clang"
static arcana::noelle::AutotunerSearchSpace *_PassMaker = NULL;
static RegisterStandardPasses _RegPass1(
    PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
      if (!_PassMaker) {
        PM.add(_PassMaker = new arcana::noelle::AutotunerSearchSpace());
      }
    }); // ** for -Ox
static RegisterStandardPasses _RegPass2(
    PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
      if (!_PassMaker) {
        PM.add(_PassMaker = new arcana::noelle::AutotunerSearchSpace());
      }
    }); // ** for -O0
