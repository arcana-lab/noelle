/*
 * Copyright 2016 - 2023  Angelo Matni, Simone Campanoni
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
#include <algorithm>

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/LoopStructure.hpp"
#include "noelle/core/Noelle.hpp"
#include "noelle/core/PDGAnalysis.hpp"
#include "noelle/core/Architecture.hpp"
#include "noelle/core/LoopForest.hpp"
#include "noelle/core/HotProfiler.hpp"

namespace llvm::noelle {

std::vector<LoopStructure *> *Noelle::getLoopStructures(Function *function) {
  return this->getLoopStructures(function, this->minHot);
}

std::vector<LoopStructure *> *Noelle::getLoopStructures(Function *function,
                                                        double minimumHotness) {

  /*
   * Check if the function has loops.
   */
  auto allLoops = new std::vector<LoopStructure *>();
  auto &LI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();
  if (std::distance(LI.begin(), LI.end()) == 0) {
    return allLoops;
  }

  /*
   * Fetch all loops of the current function.
   */
  auto loops = LI.getLoopsInPreorder();
  for (auto loop : loops) {

    /*
     * Check if the loop is hot enough.
     */
    auto loopStructure = new LoopStructure{ loop };
    if (minimumHotness > 0) {
      if (!isLoopHot(loopStructure, minimumHotness)) {
        delete loopStructure;
        continue;
      }
    }

    /*
     * Allocate the loop wrapper.
     */
    allLoops->push_back(loopStructure);
  }

  return allLoops;
}

std::vector<LoopStructure *> *Noelle::getLoopStructures(void) {
  return this->getLoopStructures(this->minHot);
}

std::vector<LoopStructure *> *Noelle::getLoopStructures(double minimumHotness) {
  auto fm = this->getFunctionsManager();
  auto allFunctions = fm->getFunctions();
  auto loops = this->getLoopStructures(minimumHotness, allFunctions);

  return loops;
}

std::vector<LoopStructure *>
    *Noelle::getLoopStructuresReachableFromEntryFunction(void) {
  return this->getLoopStructuresReachableFromEntryFunction(this->minHot);
}

std::vector<LoopStructure *>
    *Noelle::getLoopStructuresReachableFromEntryFunction(
        double minimumHotness) {

  /*
   * Default function to include loops
   */
  auto i = [](LoopStructure *l) -> bool { return true; };

  /*
   * Get the loops.
   */
  auto loops =
      this->getLoopStructuresReachableFromEntryFunction(minimumHotness, i);

  return loops;
}

std::vector<LoopStructure *>
    *Noelle::getLoopStructuresReachableFromEntryFunction(
        double minimumHotness,
        std::function<bool(LoopStructure *)> includeLoop) {

  /*
   * Fetch the list of functions of the module reachable from main.
   */
  auto functionsManager = this->getFunctionsManager();
  assert(functionsManager != nullptr);
  auto mainFunction = functionsManager->getEntryFunction();
  assert(mainFunction != nullptr);
  auto functions = functionsManager->getFunctionsReachableFrom(mainFunction);

  /*
   * Set the order for the functions.
   */
  auto s = this->fetchFunctionsSorting();

  /*
   * Fetch the loops
   */
  auto loops =
      this->getLoopStructures(minimumHotness, functions, s, includeLoop);

  return loops;
}

std::vector<LoopStructure *> *Noelle::getLoopStructures(
    double minimumHotness,
    const std::set<Function *> &functions) {

  /*
   * Set the order for the functions.
   */
  auto s = this->fetchFunctionsSorting();

  /*
   * Default function to include loops
   */
  auto i = [](LoopStructure *l) -> bool { return true; };

  /*
   * Fetch the loops
   */
  auto loops = this->getLoopStructures(minimumHotness, functions, s, i);

  return loops;
}

std::vector<LoopStructure *> *Noelle::getLoopStructures(
    double minimumHotness,
    std::function<bool(LoopStructure *)> includeLoop) {

  /*
   * Fetch all functions
   */
  auto fm = this->getFunctionsManager();
  auto allFunctions = fm->getFunctions();

  /*
   * Fetch the sorting function.
   */
  auto s = this->fetchFunctionsSorting();

  /*
   * Fetch the loops
   */
  auto loops =
      this->getLoopStructures(minimumHotness, allFunctions, s, includeLoop);

  return loops;
}

std::vector<LoopStructure *> *Noelle::getLoopStructures(
    double minimumHotness,
    const std::set<Function *> &functions,
    std::function<std::vector<Function *>(std::set<Function *> functions)>
        orderOfFunctionsToFollow,
    std::function<bool(LoopStructure *)> mustIncludeLoop) {

  auto allLoops = new std::vector<LoopStructure *>();

  /*
   * Check if we should filter out loops.
   */
  auto filterLoops = this->checkToGetLoopFilteringInfo();

  /*
   * Append loops of each function.
   */
  auto nextLoopIndex = 0;
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "Noelle: Filter out cold code\n";
  }
  auto sortedFunctions = orderOfFunctionsToFollow(functions);
  for (auto function : sortedFunctions) {

    /*
     * Check if this is application code.
     */
    if (function->empty()) {
      continue;
    }
    errs() << "Noelle:  Function \"" << function->getName() << "\"\n";

    /*
     * Check if the function is hot.
     */
    if (!isFunctionHot(function, minimumHotness)) {
      if (this->verbose >= Verbosity::Maximal) {
        errs() << "Noelle:  Disable \"" << function->getName()
               << "\" as cold function\n";
      }
      continue;
    }

    /*
     * Check if the function has loops.
     */
    auto &LI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();
    if (std::distance(LI.begin(), LI.end()) == 0) {
      continue;
    }

    /*
     * Consider all loops of the current function.
     */
    auto loops = LI.getLoopsInPreorder();
    for (auto loop : loops) {
      auto currentLoopIndex = nextLoopIndex++;

      /*
       * Check if the loop is hot enough.
       */
      errs() << "Noelle:     Loop \"" << *loop->getHeader()->getFirstNonPHI()
             << "\" (";
      auto loopStructure = new LoopStructure{ loop };
      errs() << (this->getProfiles()->getDynamicTotalInstructionCoverage(
                     loopStructure)
                 * 100)
             << "%)\n";
      auto loopHeader = loopStructure->getHeader();
      if (minimumHotness > 0) {
        if (!isLoopHot(loopStructure, minimumHotness)) {
          errs() << "Noelle:  Disable loop \"" << currentLoopIndex
                 << "\" as cold code\n";
          delete loopStructure;
          continue;
        }
      }

      // TODO: Print out more information than just loop hotness, perhaps the
      // loop header label errs() << "Noelle:  Loop hotness = " << hotness <<
      // "\n" ;

      /*
       * Check if we have to filter loops.
       * If no INDEX_FILE exists or the caller wants to include the loop, then
       * we must include it.
       */
      if (!filterLoops) {

        /*
         * Allocate the loop wrapper.
         */
        allLoops->push_back(loopStructure);
        this->loopHeaderToLoopIndexMap.insert(
            std::make_pair(loopHeader, currentLoopIndex));
        continue;
      }

      /*
       * We need to filter loops.
       *
       * Check if more than one thread is assigned to the current loop.
       * If that's the case, then we have to enable that loop.
       */
      errs() << "Noelle:      Current index = " << currentLoopIndex << "\n";
      auto maximumNumberOfCoresForTheParallelization =
          this->loopThreads[currentLoopIndex];
      if ((maximumNumberOfCoresForTheParallelization <= 1)
          && (!mustIncludeLoop(loopStructure))) {

        /*
         * Only one thread has been assigned to the current loop.
         * Hence, the current loop will not be parallelized.
         *
         * Jump to the next loop.
         */
        delete loopStructure;
        continue;
      }
      errs() << "Noelle:      Threads = "
             << maximumNumberOfCoresForTheParallelization << "\n";

      /*
       * Safety code.
       */
      if (currentLoopIndex >= loopThreads.size()) {
        errs()
            << "ERROR: the 'INDEX_FILE' file isn't correct. There are more than "
            << loopThreads.size() << " loops available in the program\n";
        abort();
      }

      /*
       * The current loop needs to be considered as specified by the user.
       */
      allLoops->push_back(loopStructure);
      this->loopHeaderToLoopIndexMap.insert(
          std::make_pair(loopHeader, currentLoopIndex));
    }
  }

  return allLoops;
}

LoopForest *Noelle::getLoopNestingForest(void) {

  /*
   * Fetch all loops
   */
  auto loopStructures = this->getLoopStructures();

  /*
   * Organize loops in forest.
   */
  auto forest = this->organizeLoopsInTheirNestingForest(*loopStructures);

  /*
   * Free the memory.
   */
  delete loopStructures;

  return forest;
}

LoopDependenceInfo *Noelle::getLoop(LoopStructure *l) {

  /*
   * Check if the loop is valid.
   */
  if (l == nullptr) {
    return nullptr;
  }

  /*
   * Compute the LDI abstraction.
   */
  auto ldi = this->getLoop(l, {});

  return ldi;
}

LoopDependenceInfo *Noelle::getLoop(
    LoopStructure *loop,
    std::unordered_set<LoopDependenceInfoOptimization> optimizations) {

  /*
   * Fetch the the function dependence graph, post dominators, and scalar
   * evolution
   */
  auto header = loop->getHeader();
  auto function = header->getParent();
  auto funcPDG = this->getFunctionDependenceGraph(function);
  auto DS = this->getDominators(function);

  /*
   * Check of loopIndex provided is within bounds
   */
  if (this->loopHeaderToLoopIndexMap.find(header)
      == this->loopHeaderToLoopIndexMap.end()) {
    auto ldi =
        this->getLoopDependenceInfoForLoop(header,
                                           funcPDG,
                                           DS,
                                           0,
                                           8,
                                           this->om->getMaximumNumberOfCores(),
                                           optimizations);

    delete DS;
    return ldi;
  }

  /*
   * Fetch the loop index.
   */
  auto loopIndex = this->loopHeaderToLoopIndexMap.at(header);

  /*
   * No filter file was provided. Construct LDI without profiler configurables
   */
  if (!this->hasReadFilterFile) {
    auto ldi =
        this->getLoopDependenceInfoForLoop(header,
                                           funcPDG,
                                           DS,
                                           0,
                                           8,
                                           this->om->getMaximumNumberOfCores(),
                                           optimizations);

    delete DS;
    return ldi;
  }

  /*
   * Ensure loop configurables exist for this loop index
   */
  if (loopIndex >= this->loopThreads.size()) {
    errs() << "ERROR: the 'INDEX_FILE' file isn't correct. There are more than "
           << this->loopThreads.size() << " loops available in the program\n";
    abort();
  }

  auto maximumNumberOfCoresForTheParallelization = this->loopThreads[loopIndex];
  auto ldi = this->getLoopDependenceInfoForLoop(
      header,
      funcPDG,
      DS,
      this->techniquesToDisable[loopIndex],
      this->DOALLChunkSize[loopIndex],
      maximumNumberOfCoresForTheParallelization,
      optimizations);

  delete DS;
  return ldi;
}

std::vector<LoopDependenceInfo *> *Noelle::getLoops(Function *function) {
  if (function->empty()) {
    return {};
  }
  auto v = this->getLoops(function, this->minHot);

  return v;
}

std::vector<LoopDependenceInfo *> *Noelle::getLoops(Function *function,
                                                    double minimumHotness) {

  /*
   * Allocate the vector of loops.
   */
  auto allLoops = new std::vector<LoopDependenceInfo *>();

  /*
   * Check if the function is hot.
   */
  if (!isFunctionHot(function, minimumHotness)) {
    return allLoops;
  }

  /*
   * Fetch the loop analysis.
   */
  auto &LI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();

  /*
   * Check if the function has loops.
   */
  if (std::distance(LI.begin(), LI.end()) == 0) {
    return allLoops;
  }

  /*
   * Fetch the function dependence graph.
   */
  auto funcPDG = this->getFunctionDependenceGraph(function);

  /*
   * Fetch the post dominators
   */
  auto DS = this->getDominators(function);

  /*
   * Fetch all loops of the current function.
   */
  auto loops = LI.getLoopsInPreorder();

  /*
   * Consider these loops.
   *
   * Collect the loop structures.
   */
  std::vector<LoopStructure *> loopStructures;
  for (auto loop : loops) {

    /*
     * Check if the loop is hot enough.
     */
    auto loopS = new LoopStructure(loop);
    if (minimumHotness > 0) {
      if (!this->isLoopHot(loopS, minimumHotness)) {
        delete loopS;
        continue;
      }
    }

    /*
     * Append the loop
     */
    for (auto edge : funcPDG->getEdges()) {
      assert(!edge->isLoopCarriedDependence() && "Flag set");
    }
    loopStructures.push_back(loopS);
  }

  /*
   * Organize loops in forest.
   */
  auto forest = this->organizeLoopsInTheirNestingForest(loopStructures);

  /*
   * Allocate the loop wrapper.
   */
  for (auto tree : forest->getTrees()) {
    for (auto loopNode : tree->getNodes()) {
      auto ls = loopNode->getLoop();
      assert(ls != nullptr);
      assert(ls->getFunction() == function);

      /*
       * Forest generation invalids the previous generated LoopInfo, we need to
       * recompute them
       */
      auto &newLI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();
      auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*function).getSE();
      auto llvmLoop = newLI.getLoopFor(ls->getHeader());
      auto ldi = new LoopDependenceInfo(funcPDG,
                                        loopNode,
                                        llvmLoop,
                                        *DS,
                                        SE,
                                        this->om->getMaximumNumberOfCores(),
                                        this->enableFloatAsReal,
                                        this->loopAwareDependenceAnalysis);
      allLoops->push_back(ldi);
    }
  }

  /*
   * Free the memory.
   */
  delete DS;

  return allLoops;
}

std::vector<LoopDependenceInfo *> *Noelle::getLoops(void) {
  auto v = this->getLoops(this->minHot);

  return v;
}

std::vector<LoopDependenceInfo *> *Noelle::getLoops(double minimumHotness) {

  /*
   * Allocate the vector of loops.
   */
  auto allLoops = new std::vector<LoopDependenceInfo *>();

  /*
   * Fetch the list of functions of the module.
   */
  auto fm = this->getFunctionsManager();
  auto mainFunction = fm->getEntryFunction();
  assert(mainFunction != nullptr);
  auto functions = fm->getFunctionsReachableFrom(mainFunction);

  /*
   * Check if we should filter out loops.
   */
  auto filterLoops = this->checkToGetLoopFilteringInfo();

  /*
   * Append loops of each function.
   */
  if (this->verbose >= Verbosity::Maximal) {
    errs() << "Noelle: Filter out cold code\n";
  }
  auto nextLoopIndex = 0;
  for (auto function : functions) {

    /*
     * Check if this is application code.
     */
    if (function->empty()) {
      continue;
    }

    /*
     * Check if the function is hot.
     */
    if (!isFunctionHot(function, minimumHotness)) {
      errs() << "Noelle:  Disable \"" << function->getName()
             << "\" as cold function\n";
      continue;
    }

    /*
     * Fetch the loop analysis.
     */
    auto &LI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();

    /*
     * Check if the function has loops.
     */
    if (std::distance(LI.begin(), LI.end()) == 0) {
      continue;
    }

    /*
     * Fetch the function dependence graph.
     */
    auto funcPDG = this->getFunctionDependenceGraph(function);

    /*
     * Fetch the post dominators and scalar evolutions
     */
    auto DS = this->getDominators(function);
    auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*function).getSE();

    /*
     * Fetch all loops of the current function.
     */
    auto loops = LI.getLoopsInPreorder();

    /*
     * Consider these loops.
     *
     * Organize loops in their forest
     */
    std::vector<LoopStructure *> loopStructures;
    std::map<LoopStructure *, uint32_t> loopIDs;
    for (auto loop : loops) {
      auto currentLoopIndex = nextLoopIndex++;

      /*
       * Check if the loop is hot enough.
       */
      auto loopS = new LoopStructure(loop);
      if (minimumHotness > 0) {
        if (!this->isLoopHot(loopS, minimumHotness)) {
          errs() << "Noelle:  Disable loop \"" << currentLoopIndex
                 << "\" as cold code\n";

          /*
           * Free the memory.
           */
          delete loopS;

          continue;
        }
      }

      /*
       * Check if we have to filter loops.
       */
      if (!filterLoops) {

        /*
         * Allocate the loop
         */
        loopStructures.push_back(loopS);
        loopIDs[loopS] = currentLoopIndex;
        continue;
      }

      /*
       * We need to filter loops.
       *
       * Check if more than one thread is assigned to the current loop.
       * If that's the case, then we have to enable that loop.
       */
      auto maximumNumberOfCoresForTheParallelization =
          this->loopThreads[currentLoopIndex];
      if (maximumNumberOfCoresForTheParallelization <= 1) {

        /*
         * Only one thread has been assigned to the current loop.
         * Hence, the current loop will not be parallelized.
         *
         * Jump to the next loop.
         */

        /*
         * Free the memory.
         */
        delete loopS;

        continue;
      }

      /*
       * Safety code.
       */
      if (this->hasReadFilterFile && currentLoopIndex >= loopThreads.size()) {
        errs()
            << "ERROR: the 'INDEX_FILE' file isn't correct. There are more than "
            << loopThreads.size() << " loops available in the program\n";
        abort();
      }

      /*
       * The current loop needs to be considered as specified by the user.
       */
      loopStructures.push_back(loopS);
      loopIDs[loopS] = currentLoopIndex;
    }

    /*
     * Organize the loops in forest.
     */
    auto forest = this->organizeLoopsInTheirNestingForest(loopStructures);

    /*
     * Compute the LoopDependeceInfo abstractions.
     */
    for (auto tree : forest->getTrees()) {
      for (auto loopNode : tree->getNodes()) {

        /*
         * Fetch the loop
         */
        auto ls = loopNode->getLoop();
        assert(loopIDs.find(ls) != loopIDs.end());
        auto currentLoopIndex = loopIDs[ls];

        /*
         * Fetch the LLVM loop
         */
        auto &LI =
            getAnalysis<LoopInfoWrapperPass>(*ls->getFunction()).getLoopInfo();
        auto LLVMLoop = LI.getLoopFor(ls->getHeader());

        /*
         * Check if we have to filter loops.
         */
        LoopDependenceInfo *ldi = nullptr;
        if (!filterLoops) {
          ldi = new LoopDependenceInfo(funcPDG,
                                       loopNode,
                                       LLVMLoop,
                                       *DS,
                                       SE,
                                       this->om->getMaximumNumberOfCores(),
                                       this->enableFloatAsReal,
                                       this->loopAwareDependenceAnalysis);

        } else {
          auto maximumNumberOfCoresForTheParallelization =
              loopThreads[currentLoopIndex];
          assert(maximumNumberOfCoresForTheParallelization > 1);
          ldi = this->getLoopDependenceInfoForLoop(
              loopNode,
              LLVMLoop,
              funcPDG,
              DS,
              &SE,
              this->techniquesToDisable[currentLoopIndex],
              this->DOALLChunkSize[currentLoopIndex],
              maximumNumberOfCoresForTheParallelization,
              {});
        }
        allLoops->push_back(ldi);
      }
    }

    /*
     * Free the memory.
     */
    delete DS;
  }

  return allLoops;
}

uint32_t Noelle::getNumberOfProgramLoops(void) {
  return this->getNumberOfProgramLoops(this->minHot);
}

uint32_t Noelle::getNumberOfProgramLoops(double minimumHotness) {
  uint32_t counter = 0;

  /*
   * Fetch the list of functions of the module.
   */
  auto fm = this->getFunctionsManager();
  auto mainFunction = fm->getEntryFunction();
  assert(mainFunction != nullptr);
  auto functions = fm->getFunctionsReachableFrom(mainFunction);

  /*
   * Check if we should filter out loops.
   */
  auto filterLoops = this->checkToGetLoopFilteringInfo();

  /*
   * Append loops of each function.
   */
  auto currentLoopIndex = 0;
  for (auto function : functions) {

    /*
     * Fetch the loop analysis.
     */
    auto &LI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();

    /*
     * Check if the function has loops.
     */
    if (std::distance(LI.begin(), LI.end()) == 0) {
      continue;
    }

    /*
     * Check if the function is hot.
     */
    if (!isFunctionHot(function, minimumHotness)) {
      continue;
    }

    /*
     * Fetch all loops of the current function.
     */
    auto loops = LI.getLoopsInPreorder();

    /*
     * Consider these loops.
     */
    for (auto loop : loops) {

      /*
       * Check if the loop is hot enough.
       */
      LoopStructure loopStructure{ loop };
      if (minimumHotness > 0) {
        if (!isLoopHot(&loopStructure, minimumHotness)) {
          currentLoopIndex++;
          continue;
        }
      }

      /*
       * Check if we have to filter loops.
       */
      if (!filterLoops) {
        counter++;
        currentLoopIndex++;
        continue;
      }

      /*
       * We need to filter loops.
       *
       * Check if more than one thread is assigned to the current loop.
       * If that's the case, then we have to enable that loop.
       */
      auto maximumNumberOfCoresForTheParallelization =
          loopThreads[currentLoopIndex];
      if (maximumNumberOfCoresForTheParallelization <= 1) {

        /*
         * Only one thread has been assigned to the current loop.
         * Hence, the current loop will not be parallelized.
         */
        currentLoopIndex++;

        /*
         * Jump to the next loop.
         */
        continue;
      }

      /*
       * The current loop has more than one core assigned to it.
       * Therefore, we need to parallelize this loop.
       * In other words, the current loop needs to be considered as specified by
       * the user.
       */
      counter++;
      currentLoopIndex++;
    }
  }

  return counter;
}

bool Noelle::checkToGetLoopFilteringInfo(void) {

  /*
   * Check the name of the file that lists the loops to consider.
   */
  if (!this->filterFileName) {
    return false;
  }

  /*
   * Check that the file hasn't been read already
   */
  if (this->hasReadFilterFile) {
    return true;
  }

  /*
   * We need to filter out loops.
   *
   * Open the file that specifies which loops to keep.
   */
  auto indexBuf = MemoryBuffer::getFileAsStream(this->filterFileName);
  if (auto ec = indexBuf.getError()) {
    errs() << "Failed to read INDEX_FILE = \"" << this->filterFileName
           << "\":" << ec.message() << "\n";
    abort();
  }

  /*
   * Read the file.
   */
  auto fileAsString = indexBuf.get()->getBuffer().str();
  std::stringstream indexString{ fileAsString };

  /*
   * Parse the file
   */
  auto filterLoops = false;
  while (indexString.peek() != EOF) {
    filterLoops = true;

    /*
     * Should the loop be parallelized?
     */
    auto shouldBeParallelized = this->fetchTheNextValue(indexString);
    assert(shouldBeParallelized == 0 || shouldBeParallelized == 1);

    /*
     * Unroll factor
     */
    auto unrollFactor = this->fetchTheNextValue(indexString);

    /*
     * Peel factor
     */
    auto peelFactor = this->fetchTheNextValue(indexString);

    /*
     * Techniques to disable
     * 0: None
     * 1: DSWP
     * 2: HELIX
     * 3: DOALL
     * 4: DSWP, HELIX
     * 5: DSWP, DOALL
     * 6: HELIX, DOALL
     */
    auto technique = this->fetchTheNextValue(indexString);

    /*
     * Number of cores
     */
    auto cores = this->fetchTheNextValue(indexString);

    /*
     * DOALL: chunk factor
     */
    auto DOALLChunkFactor = this->fetchTheNextValue(indexString);
    DOALLChunkFactor++; /*
                          DOALL chunk size is the one defined by INDEX_FILE + 1.
                          This is because chunk size must start from 1.
                         */

    /*
     * Skip
     */
    this->fetchTheNextValue(indexString);
    this->fetchTheNextValue(indexString);
    this->fetchTheNextValue(indexString);

    /*
     * If the loop needs to be parallelized, then we enable it.
     */
    if ((shouldBeParallelized) && (cores >= 2)) {
      this->loopThreads.push_back(cores);
      this->techniquesToDisable.push_back(technique);
      this->DOALLChunkSize.push_back(DOALLChunkFactor);

    } else {
      this->loopThreads.push_back(1);
      this->techniquesToDisable.push_back(0);
      this->DOALLChunkSize.push_back(1); /*
                            DOALL chunk size is the one defined by INDEX_FILE
                            + 1. This is because chunk size must start from 1.
                           */
    }
  }

  /*
   * Keep track that we have read the file
   */
  this->hasReadFilterFile = true;

  return filterLoops;
}

void Noelle::sortByHotness(std::vector<LoopDependenceInfo *> &loops) {

  /*
   * Fetch the profiles.
   */
  auto hot = this->getProfiles();

  /*
   * Define the order between loops.
   */
  auto compareLoops = [hot](LoopDependenceInfo *a,
                            LoopDependenceInfo *b) -> bool {
    assert(a != nullptr);
    assert(b != nullptr);

    /*
     * Fetch the information.
     */
    auto aLS = a->getLoopStructure();
    auto bLS = b->getLoopStructure();
    auto aInsts = hot->getTotalInstructions(aLS);
    auto bInsts = hot->getTotalInstructions(bLS);

    return aInsts > bInsts;
  };

  /*
   * Sort the loops.
   */
  std::sort(loops.begin(), loops.end(), compareLoops);

  return;
}

void Noelle::sortByHotness(std::vector<LoopStructure *> &loops) {

  /*
   * Fetch the profiles.
   */
  auto hot = this->getProfiles();

  /*
   * Define the order between loops.
   */
  auto compareLoops = [hot](LoopStructure *a, LoopStructure *b) -> bool {
    auto aInsts = hot->getTotalInstructions(a);
    auto bInsts = hot->getTotalInstructions(b);
    return aInsts > bInsts;
  };

  /*
   * Sort the loops.
   */
  std::sort(loops.begin(), loops.end(), compareLoops);

  return;
}

std::vector<LoopForestNode *> Noelle::sortByHotness(
    const std::unordered_set<LoopForestNode *> &loops) {
  std::vector<LoopForestNode *> s;

  /*
   * Convert the loops into the vector
   */
  for (auto n : loops) {
    s.push_back(n);
  }

  /*
   * Check if we need to sort
   */
  if (s.size() <= 1) {
    return s;
  }

  /*
   * Fetch the profiles.
   */
  auto hot = this->getProfiles();

  /*
   * Define the order between loops.
   */
  auto compareLoops = [hot](LoopForestNode *n0, LoopForestNode *n1) -> bool {
    assert(n0 != nullptr);
    assert(n1 != nullptr);

    /*
     * Fetch the information.
     */
    auto a = n0->getLoop();
    auto b = n1->getLoop();
    assert(a != nullptr);
    assert(b != nullptr);
    auto aInsts = hot->getTotalInstructions(a);
    auto bInsts = hot->getTotalInstructions(b);

    return aInsts > bInsts;
  };

  /*
   * Sort the loops.
   */
  std::sort(s.begin(), s.end(), compareLoops);

  return s;
}

void Noelle::sortByStaticNumberOfInstructions(
    std::vector<LoopDependenceInfo *> &loops) {

  /*
   * Define the order between loops.
   */
  auto compareLoops = [](LoopDependenceInfo *a, LoopDependenceInfo *b) -> bool {
    auto aLS = a->getLoopStructure();
    auto bLS = b->getLoopStructure();
    auto aInsts = aLS->getNumberOfInstructions();
    auto bInsts = bLS->getNumberOfInstructions();

    return aInsts > bInsts;
  };

  /*
   * Sort the loops.
   */
  std::sort(loops.begin(), loops.end(), compareLoops);

  return;
}

LoopDependenceInfo *Noelle::getLoopDependenceInfoForLoop(
    BasicBlock *header,
    PDG *functionPDG,
    DominatorSummary *DS,
    uint32_t techniquesToDisable,
    uint32_t DOALLChunkSize,
    uint32_t maxCores,
    std::unordered_set<LoopDependenceInfoOptimization> optimizations) {

  /*
   * Fetch the function
   */
  auto function = header->getParent();

  /*
   * Fetch the ForestNode of the loop
   */
  auto allLoopsOfFunction = this->getLoopStructures(function, 0);
  auto forest = this->organizeLoopsInTheirNestingForest(*allLoopsOfFunction);
  auto newLoopNode = forest->getInnermostLoopThatContains(&*header->begin());

  /*
   * Fetch the llvm loop corresponding to the loop structure
   */
  auto &LI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*function).getSE();
  auto llvmLoop = LI.getLoopFor(header);

  /*
   * Compute the LoopDependenceInfo
   */
  auto ldi = this->getLoopDependenceInfoForLoop(newLoopNode,
                                                llvmLoop,
                                                functionPDG,
                                                DS,
                                                &SE,
                                                techniquesToDisable,
                                                DOALLChunkSize,
                                                maxCores,
                                                optimizations);

  return ldi;
}

/*
 * Get the loop nesting graph of the whole program
 * 1. Get all loops as nodes
 *   1.1. Add edges to existing nesting relations
 * 2. Traverse the call graph, if there is a call from function A to function B
 *   2.1. Get the subedges from A to B
 *   2.2. For each subedge E <call_inst -> function B> , get the most inner loop
 * L of the call_inst 2.3. Add an edge from L to all outermost loops in function
 * B (may or must based on the subedge type)
 */
LoopNestingGraph *Noelle::getLoopNestingGraphForProgram() {

  /*
   * Fetch the list of functions of the module.
   */
  auto fm = this->getFunctionsManager();
  auto mainFunction = fm->getEntryFunction();
  assert(mainFunction != nullptr);
  auto functions = fm->getFunctionsReachableFrom(mainFunction);

  /*
   * Check if we should filter out loops.
   */
  auto filterLoops = this->checkToGetLoopFilteringInfo();

  //  add loops into the loop nesting graph
  std::vector<LoopStructure *> allLoops;
  for (auto function : functions) {
    auto allLoopsOfFunction = this->getLoopStructures(function, filterLoops);
    allLoops.insert(allLoops.end(),
                    allLoopsOfFunction->begin(),
                    allLoopsOfFunction->end());
  }
  // FIXME: is the module necessary?
  auto loopNestingGraph = new LoopNestingGraph(*this->program, allLoops);

  /*
   * Fetch the call graph.
   */
  auto callGraph = fm->getProgramCallGraph();

  /*
   * For each function A, get the loop forest
   * From the call graph, get all edges going out of A
   */
  auto forest = this->organizeLoopsInTheirNestingForest(allLoops);
  // Add existing loop nesting relation as must edges
  auto f = [&loopNestingGraph](LoopForestNode *n, uint32_t treeLevel) -> bool {
    if (n->getParent() == nullptr) {
      return false;
    }
    auto parentLoop = n->getParent()->getLoop();
    auto childLoop = n->getLoop();
    loopNestingGraph->createEdge(parentLoop, nullptr, childLoop, true);

    return false;
  };

  for (auto tree : forest->getTrees()) {
    // iterate through the tree and add each
    tree->visitPreOrder(f);
  }

  std::map<Function *, unordered_set<LoopStructure *>> outermostLoopsMap;
  // filter out all loops not outermost loop (if loop->getNestingLevel==1)
  for (auto loop : allLoops) {
    if (loop->getNestingLevel() == 1) {
      // get function
      auto fcn = loop->getFunction();
      outermostLoopsMap[fcn].insert(loop);
    }
  }

  for (auto calleeNode : callGraph->getFunctionNodes()) {
    auto calleeFunction = calleeNode->getFunction();
    if (outermostLoopsMap.find(calleeFunction) == outermostLoopsMap.end())
      continue;

    for (auto edge : calleeNode->getIncomingEdges()) {
      for (auto subEdge : edge->getSubEdges()) {
        auto caller = subEdge->getCaller();
        auto callingInst = cast<CallBase>(caller->getInstruction());
        if (auto loopNode = forest->getInnermostLoopThatContains(callingInst)) {
          auto parentLoop = loopNode->getLoop();

          // add the edges
          for (auto outermostLoop : outermostLoopsMap[calleeFunction]) {
            loopNestingGraph->createEdge(parentLoop,
                                         callingInst,
                                         outermostLoop,
                                         subEdge->isAMustCall());
          }
        }
      }
    }
  }

  return loopNestingGraph;
}

LoopDependenceInfo *Noelle::getLoopDependenceInfoForLoop(
    LoopForestNode *loopNode,
    Loop *loop,
    PDG *functionPDG,
    DominatorSummary *DS,
    ScalarEvolution *SE,
    uint32_t techniquesToDisableForLoop,
    uint32_t DOALLChunkSizeForLoop,
    uint32_t maxCores,
    std::unordered_set<LoopDependenceInfoOptimization> optimizations) {

  /*
   * Allocate the LDI.
   */
  auto ldi = new LoopDependenceInfo(functionPDG,
                                    loopNode,
                                    loop,
                                    *DS,
                                    *SE,
                                    maxCores,
                                    this->enableFloatAsReal,
                                    optimizations,
                                    this->loopAwareDependenceAnalysis,
                                    DOALLChunkSizeForLoop);

  /*
   * Set the techniques that are enabled.
   */
  auto ltm = ldi->getLoopTransformationsManager();
  auto disableTransformations = techniquesToDisableForLoop;
  switch (disableTransformations) {

    case 0:
      ltm->enableAllTransformations();
      break;

    case 1:
      ltm->disableTransformation(DSWP_ID);
      break;

    case 2:
      ltm->disableTransformation(HELIX_ID);
      break;

    case 3:
      ltm->disableTransformation(DOALL_ID);
      break;

    case 4:
      ltm->disableTransformation(DSWP_ID);
      ltm->disableTransformation(HELIX_ID);
      break;

    case 5:
      ltm->disableTransformation(DSWP_ID);
      ltm->disableTransformation(DOALL_ID);
      break;

    case 6:
      ltm->disableTransformation(HELIX_ID);
      ltm->disableTransformation(DOALL_ID);
      break;

    default:
      abort();
  }

  return ldi;
}

bool Noelle::isLoopHot(LoopStructure *loopStructure, double minimumHotness) {

  /*
   * Fetch the profiles.
   */
  auto hot = this->getProfiles();
  assert(hot != nullptr);

  /*
   * Check if the profiles are available
   */
  if (!hot->isAvailable()) {
    return true;
  }

  auto hotness = hot->getDynamicTotalInstructionCoverage(loopStructure);
  return hotness >= minimumHotness;
}

bool Noelle::isFunctionHot(Function *function, double minimumHotness) {

  /*
   * Fetch the profiles.
   */
  auto hot = this->getProfiles();
  assert(hot != nullptr);

  /*
   * Check if the profiles are available
   */
  if (!hot->isAvailable()) {
    return true;
  }

  auto hotness = hot->getDynamicTotalInstructionCoverage(function);
  return hotness >= minimumHotness;
}

void Noelle::filterOutLoops(std::vector<LoopStructure *> &loops,
                            std::function<bool(LoopStructure *)> filter) {

  /*
   * Tag the loops that need to be removed.
   */
  std::vector<uint64_t> toDelete{};
  uint64_t currentIndex = 0;
  for (auto loop : loops) {
    if (filter(loop)) {
      toDelete.insert(toDelete.begin(), currentIndex);
    }
    currentIndex++;
  }

  /*
   * Remove the loops.
   */
  for (auto index : toDelete) {
    loops.erase(loops.begin() + index);
  }

  return;
}

void Noelle::filterOutLoops(noelle::LoopForest *f,
                            std::function<bool(LoopStructure *)> filter) {

  /*
   * Iterate over the trees and find the nodes to delete.
   */
  std::vector<noelle::LoopForestNode *> toDelete{};
  for (auto tree : f->getTrees()) {
    auto myF = [&filter, &toDelete](noelle::LoopForestNode *n,
                                    uint32_t l) -> bool {
      auto ls = n->getLoop();
      if (filter(ls)) {
        toDelete.push_back(n);
      }
      return false;
    };
    tree->visitPreOrder(myF);
  }

  /*
   * Delete the nodes.
   */
  for (auto n : toDelete) {
    delete n;
  }

  return;
}

LoopForest *Noelle::getProgramLoopsNestingForest(void) {

  /*
   * Fetch all the loops
   */
  auto allLoops = this->getLoopStructures();

  /*
   * Organize the loops into a forest
   */
  auto forest = this->organizeLoopsInTheirNestingForest(*allLoops);

  return forest;
}

LoopForest *Noelle::organizeLoopsInTheirNestingForest(
    std::vector<LoopStructure *> const &loops) {

  /*
   * Compute the dominators.
   */
  std::unordered_map<Function *, DominatorSummary *> doms{};
  for (auto loop : loops) {
    auto f = loop->getFunction();
    if (doms.find(f) != doms.end()) {
      continue;
    }
    doms[f] = this->getDominators(f);
  }

  /*
   * Compute the forest.
   */
  auto n = new noelle::LoopForest(loops, doms);

  /*
   * Free the memory.
   */
  for (auto pair : doms) {
    delete pair.second;
  }

  return n;
}

std::function<std::vector<Function *>(std::set<Function *> fns)> Noelle::
    fetchFunctionsSorting(void) {
  std::function<std::vector<Function *>(std::set<Function *> fns)> s;

  /*
   * Fetch the functions manager.
   */
  auto functionsManager = this->getFunctionsManager();
  assert(functionsManager != nullptr);

  /*
   * Check if we have the profiles.
   * If we do, then we sort the functions from the hottest to the coldest.
   */
  auto prof = this->getProfiles();
  if (prof->isAvailable()) {

    /*
     * Set the order to be the coverage one.
     */
    s = [functionsManager](
            std::set<Function *> fns) -> std::vector<Function *> {
      std::vector<Function *> o;
      for (auto f : fns) {
        o.push_back(f);
      }

      functionsManager->sortByHotness(o);

      return o;
    };

  } else {

    /*
     * Fetch the loops using the default order.
     */
    s = [](std::set<Function *> fns) -> std::vector<Function *> {
      std::vector<Function *> o;
      for (auto f : fns) {
        o.push_back(f);
      }
      return o;
    };
  }

  return s;
}

} // namespace llvm::noelle
