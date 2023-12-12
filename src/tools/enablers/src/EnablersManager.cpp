/*
 * Copyright 2019 - 2021 Angelo Matni, Simone Campanoni
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
#include "noelle/core/Noelle.hpp"
#include "EnablersManager.hpp"

namespace arcana::noelle {

EnablersManager::EnablersManager() : ModulePass{ ID } {

  return;
}

bool EnablersManager::runOnModule(Module &M) {

  /*
   * Check if enablers have been enabled.
   */
  if (!this->enableEnablers) {
    return false;
  }
  errs() << "EnablersManager: Start\n";

  /*
   * Fetch the outputs of the passes we rely on.
   */
  auto &noelle = getAnalysis<Noelle>();

  /*
   * Create the enablers.
   */
  auto &loopTransformer = noelle.getLoopTransformer();
  auto loopInvariantCodeMotion = LoopInvariantCodeMotion(noelle);
  auto scevSimplification = SCEVSimplification(noelle);

  /*
   * Fetch all the loops we want to parallelize.
   */
  auto loopsToParallelize =
      noelle.getLoopStructuresReachableFromEntryFunction();
  errs() << "EnablersManager:   Try to improve all "
         << loopsToParallelize->size() << " loops, one at a time\n";

  /*
   * Remove loops that have not been executed
   */
  auto hot = noelle.getProfiles();
  auto filter = [hot](LoopStructure *l) -> bool {
    if (!hot->hasBeenExecuted(l)) {
      return true;
    }
    return false;
  };
  noelle.filterOutLoops(*loopsToParallelize, filter);

  /*
   * Organize loops in a forest.
   */
  auto forest = noelle.organizeLoopsInTheirNestingForest(*loopsToParallelize);

  /*
   * Fetch the trees.
   */
  auto trees = forest->getTrees();

  /*
   * Sort the trees by hotness
   */
  auto sortedTrees = noelle.sortByHotness(trees);

  /*
   * Transform the loops selected.
   */
  auto modified = false;
  std::unordered_map<Function *, bool> modifiedFunctions;
  for (auto tree : sortedTrees) {

    /*
     * Parallelize all loops within this tree starting from the leafs.
     */
    auto f = [&loopTransformer,
              &loopInvariantCodeMotion,
              &scevSimplification,
              &noelle,
              &modifiedFunctions,
              this,
              &modified](LoopTree *n, uint32_t l) -> bool {
      /*
       * Fetch the loop
       */
      auto loopStructure = n->getLoop();

      /*
       * Print the loop.
       */
      errs() << "EnablersManager:   Loop:\n";
      errs() << "EnablersManager:     Function = \""
             << loopStructure->getFunction()->getName() << "\"\n";
      // errs() << "EnablersManager:     Loop ID  = " << loopStructure->getID()
      // << "\n"; // ED
      errs() << "EnablersManager:     Entry instruction = "
             << *loopStructure->getHeader()->getFirstNonPHI() << "\n";

      /*
       * Fetch the function that contains the current loop.
       */
      auto f = loopStructure->getFunction();

      /*
       * Check if we have already modified the function.
       */
      if (modifiedFunctions[f]) {
        errs()
            << "EnablersManager:     The current loop belongs to the function "
            << f->getName() << " , which has already been modified.\n";
        return false;
      }

      /*
       * Fetch the LoopContent
       */
      auto loopToImprove = noelle.getLoopContent(loopStructure);

      /*
       * Improve the current loop.
       */
      modifiedFunctions[f] |= this->applyEnablers(&*loopToImprove,
                                                  noelle,
                                                  loopTransformer,
                                                  loopInvariantCodeMotion,
                                                  scevSimplification);
      modified |= modifiedFunctions[f];

      return false;
    };
    tree->visitPostOrder(f);
  }

  /*
   * Free the memory.
   */
  delete loopsToParallelize;

  errs() << "EnablersManager: Exit\n";
  return modified;
}

} // namespace arcana::noelle
