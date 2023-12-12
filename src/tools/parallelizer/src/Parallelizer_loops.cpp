/*
 * Copyright 2023  Simone Campanoni
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
#include "Parallelizer.hpp"

namespace arcana::noelle {

bool Parallelizer::parallelizeLoops(Noelle &noelle, Heuristics *heuristics) {

  /*
   * Fetch the verbosity level.
   */
  auto verbosity = noelle.getVerbosity();

  /*
   * Collect information about C++ code we link parallelized loops with.
   */
  auto M = noelle.getProgram();
  errs() << "Parallelizer:  Analyzing the module " << M->getName() << "\n";
  if (!this->collectThreadPoolHelperFunctionsAndTypes(*M, noelle)) {
    errs()
        << "Parallelizer:    ERROR: I could not find the runtime within the module\n";
    return false;
  }

  /*
   * Fetch all the loops we want to parallelize.
   */
  errs() << "Parallelizer:  Fetching the program loops\n";
  auto forest = noelle.getLoopNestingForest();
  if (forest->getNumberOfLoops() == 0) {
    errs() << "Parallelizer:    There is no loop to consider\n";

    /*
     * Free the memory.
     */
    delete forest;

    errs() << "Parallelizer: Exit\n";
    return false;
  }
  errs() << "Parallelizer:    There are " << forest->getNumberOfLoops()
         << " loops in the program that are enabled from the options used\n";

  const auto isSelected = [&](int i) {
    const auto &WL = this->loopIndexesWhiteList;
    const auto &BL = this->loopIndexesBlackList;
    /*
     * If no index has been specified we will select them all
     */
    if (!WL.empty()) {
      return std::find(WL.begin(), WL.end(), i) != std::end(WL);
    }
    if (!BL.empty()) {
      return std::find(BL.begin(), BL.end(), i) == std::end(BL);
    }
    return true;
  };

  /*
   * Determine the parallelization order from the metadata.
   */
  auto mm = noelle.getMetadataManager();
  std::map<uint32_t, LoopContent *> loopParallelizationOrder;
  for (auto tree : forest->getTrees()) {
    auto selector = [&noelle, &mm, &loopParallelizationOrder, &isSelected](
                        LoopTree *n,
                        uint32_t treeLevel) -> bool {
      auto ls = n->getLoop();
      if (!mm->doesHaveMetadata(ls, "noelle.parallelizer.looporder")) {
        return false;
      }
      auto parallelizationOrderIndex =
          std::stoi(mm->getMetadata(ls, "noelle.parallelizer.looporder"));
      if (!isSelected(parallelizationOrderIndex)) {
        return false;
      }
      auto optimizations = { LoopContentOptimization::MEMORY_CLONING_ID,
                             LoopContentOptimization::THREAD_SAFE_LIBRARY_ID };
      auto ldi = noelle.getLoopContent(ls, optimizations);
      loopParallelizationOrder[parallelizationOrderIndex] = ldi;
      return false;
    };
    tree->visitPreOrder(selector);
  }
  errs() << "Parallelizer:    Selected loops with index: ";
  for (const auto &[k, v] : loopParallelizationOrder) {
    errs() << k << " ";
  }
  errs() << "\n";

  /*
   * Parallelize the loops in order.
   */
  auto modified = false;
  std::unordered_map<BasicBlock *, bool> modifiedBBs{};
  std::unordered_set<Function *> modifiedFunctions;
  for (auto indexLoopPair : loopParallelizationOrder) {
    auto ldi = indexLoopPair.second;

    /*
     * Check if we can parallelize this loop.
     */
    auto ls = ldi->getLoopStructure();
    auto safe = true;
    for (auto bb : ls->getBasicBlocks()) {
      if (modifiedBBs[bb]) {
        safe = false;
        break;
      }
    }

    /*
     * Get loop ID.
     */
    auto loopIDOpt = ls->getID();

    if (!safe) {
      errs() << "Parallelizer:    Loop ";
      // Parent loop has been parallelized, so basic blocks have been modified
      // and we might not have a loop ID for the child loop. If we have it we
      // print it, otherwise we don't.
      if (loopIDOpt) {
        auto loopID = loopIDOpt.value();
        errs() << loopID;
      }
      errs()
          << " cannot be parallelized because one of its parent has been parallelized already\n";
      continue;
    }

    /*
     * Parallelize the current loop.
     */
    auto loopIsParallelized = this->parallelizeLoop(ldi, noelle, heuristics);

    /*
     * Keep track of the parallelization.
     */
    if (loopIsParallelized) {
      errs() << "Parallelizer:    Loop ";
      assert(loopIDOpt);
      auto loopID = loopIDOpt.value();
      errs() << loopID;
      errs() << " has been parallelized\n";
      errs()
          << "Parallelizer:      Keep track of basic blocks being modified by the parallelization\n";
      modified = true;
      for (auto bb : ls->getBasicBlocks()) {
        modifiedBBs[bb] = true;
      }
      modifiedFunctions.insert(ls->getFunction());
    }
  }

  /*
   * Free the memory.
   */
  for (auto indexLoopPair : loopParallelizationOrder) {
    delete indexLoopPair.second;
  }

  /*
   * Erase calls to intrinsics in modified functions
   */
  std::unordered_set<CallInst *> intrinsicCallsToRemove;
  for (auto F : modifiedFunctions) {
    for (auto &I : *F) {
      if (auto callInst = dyn_cast<CallInst>(&I)) {
        if (callInst->isLifetimeStartOrEnd()) {
          intrinsicCallsToRemove.insert(callInst);
        }
      }
    }
  }
  for (auto call : intrinsicCallsToRemove) {
    call->eraseFromParent();
  }

  errs() << "Parallelizer: Exit\n";
  return modified;
}

} // namespace arcana::noelle
