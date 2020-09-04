/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Noelle.hpp"
#include "PDGAnalysis.hpp"
#include "HotProfiler.hpp"
#include "Architecture.hpp"
#include "StayConnectedNestedLoopForest.hpp"

namespace llvm::noelle {

std::vector<LoopStructure *> * Noelle::getLoopStructures (
    Function *function
    ) {
  return this->getLoopStructures(function, this->minHot);
}

std::vector<LoopStructure *> * Noelle::getLoopStructures (
    Function *function,
    double minimumHotness
    ) {

  /*
   * Check if the function has loops.
   */
  auto allLoops = new std::vector<LoopStructure *>();
  auto& LI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();
  if (std::distance(LI.begin(), LI.end()) == 0){
    return allLoops;
  }

  /*
   * Fetch all loops of the current function.
   */
  auto loops = LI.getLoopsInPreorder();
  for (auto loop : loops){

    /*
     * Check if the loop is hot enough.
     */
    auto loopStructure = new LoopStructure{loop};
    if (!isLoopHot(loopStructure, minimumHotness)) {
      delete loopStructure;
      continue;
    }

    /*
     * Allocate the loop wrapper.
     */
    allLoops->push_back(loopStructure);
  }

  return allLoops;
}

std::vector<LoopStructure *> * Noelle::getLoopStructures (void) {
  return this->getLoopStructures(this->minHot);
}

std::vector<LoopStructure *> * Noelle::getLoopStructures (
    double minimumHotness
    ) {

  auto profiles = this->getProfiles();
  auto allLoops = new std::vector<LoopStructure *>();

  /*
   * Fetch the list of functions of the module.
   */
  auto mainFunction = this->getEntryFunction();
  assert(mainFunction != nullptr);
  auto functions = this->getModuleFunctionsReachableFrom(this->program, mainFunction);

  /*
   * Check if we should filter out loops.
   */
  auto filterLoops = this->checkToGetLoopFilteringInfo();

  /*
   * Append loops of each function.
   */
  auto nextLoopIndex = 0;
  if (this->verbose >= Verbosity::Maximal){
    errs() << "Parallelizer: Filter out cold code\n" ;
  }
  for (auto function : *functions){

    /*
     * Check if the function is hot.
     */
    if (!isFunctionHot(function, minimumHotness)){
      errs() << "Parallelizer:  Disable \"" << function->getName() << "\" as cold function\n";
      continue ;
    }

    /*
     * Check if the function has loops.
     */
    auto& LI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();
    if (std::distance(LI.begin(), LI.end()) == 0){
      continue ;
    }

    /*
     * Consider all loops of the current function.
     */
    auto loops = LI.getLoopsInPreorder();
    for (auto loop : loops){
      auto currentLoopIndex = nextLoopIndex++;

      /*
       * Check if the loop is hot enough.
       */
      auto loopStructure = new LoopStructure{loop};
      auto loopHeader = loopStructure->getHeader();
      if (!isLoopHot(loopStructure, minimumHotness)){
        errs() << "Parallelizer:  Disable loop \"" << currentLoopIndex << "\" as cold code\n";
        delete loopStructure;
        continue ;
      }

      // TODO: Print out more information than just loop hotness, perhaps the loop header label
      // errs() << "Parallelizer:  Loop hotness = " << hotness << "\n" ;

      /*
       * Check if we have to filter loops.
       */
      if (!filterLoops){

        /*
         * Allocate the loop wrapper.
         */
        allLoops->push_back(loopStructure);
        this->loopHeaderToLoopIndexMap.insert(std::make_pair(loopHeader, currentLoopIndex));
        continue ;
      }

      /*
       * We need to filter loops.
       *
       * Check if more than one thread is assigned to the current loop.
       * If that's the case, then we have to enable that loop.
       */
      auto maximumNumberOfCoresForTheParallelization = this->loopThreads[currentLoopIndex];
      if (maximumNumberOfCoresForTheParallelization <= 1){

        /*
         * Only one thread has been assigned to the current loop.
         * Hence, the current loop will not be parallelized.
         *
         * Jump to the next loop.
         */
        delete loopStructure;
        continue ;
      }

      /*
       * Safety code.
       */
      if (currentLoopIndex >= loopThreads.size()){
        errs() << "ERROR: the 'INDEX_FILE' file isn't correct. There are more than " << loopThreads.size() << " loops available in the program\n";
        abort();
      }

      /*
       * The current loop needs to be considered as specified by the user.
       */
      allLoops->push_back(loopStructure);
      this->loopHeaderToLoopIndexMap.insert(std::make_pair(loopHeader, currentLoopIndex));
    }
  }

  delete functions;

  return allLoops;
}

LoopDependenceInfo * Noelle::getLoop (
    LoopStructure *loop
    ) {
  return getLoop(loop, {});
}

LoopDependenceInfo * Noelle::getLoop (
    LoopStructure *loop,
    std::unordered_set<LoopDependenceInfoOptimization> optimizations
    ) {

  /*
   * Fetch the the function dependence graph, post dominators, and scalar evolution
   */
  auto header = loop->getHeader();
  auto function = header->getParent();
  auto funcPDG = this->getFunctionDependenceGraph(function);
  auto DS = this->getDominators(function);

  /*
   * Fetch the llvm loop corresponding to the loop structure
   */
  auto& LI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();
  auto& SE = getAnalysis<ScalarEvolutionWrapperPass>(*function).getSE();
  auto llvmLoop = LI.getLoopFor(header);

  /*
   * Check of loopIndex provided is within bounds
   */
  if (this->loopHeaderToLoopIndexMap.find(header) == this->loopHeaderToLoopIndexMap.end()){
    auto ldi = new LoopDependenceInfo(funcPDG, llvmLoop, *DS, SE, this->maxCores, {}, this->loopAA, this->loopAwareDependenceAnalysis);

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
    auto ldi = new LoopDependenceInfo(funcPDG, llvmLoop, *DS, SE, this->maxCores, optimizations, this->loopAA, this->loopAwareDependenceAnalysis);

    delete DS;
    return ldi;
  }

  /*
   * Ensure loop configurables exist for this loop index
   */
  if (loopIndex >= this->loopThreads.size()){
    errs() << "ERROR: the 'INDEX_FILE' file isn't correct. There are more than " << this->loopThreads.size()
      << " loops available in the program\n";
    abort();
  }

  auto maximumNumberOfCoresForTheParallelization = this->loopThreads[loopIndex];
  assert(maximumNumberOfCoresForTheParallelization > 1
      && "Noelle: passed user a filtered loop yet it only has max cores <= 1");

  auto ldi = getLoopDependenceInfoForLoop(
      llvmLoop,
      funcPDG,
      DS,
      &SE,
      this->techniquesToDisable[loopIndex],
      this->DOALLChunkSize[loopIndex],
      maximumNumberOfCoresForTheParallelization
      );

  delete DS;
  return ldi;
}

std::vector<LoopDependenceInfo *> * Noelle::getLoops (
    Function *function
    ){
  auto v = this->getLoops(function, this->minHot);

  return v;
}

std::vector<LoopDependenceInfo *> * Noelle::getLoops (
    Function *function,
    double minimumHotness
    ){

  /*
   * Fetch the profiles.
   */
  auto profiles = this->getProfiles();

  /*
   * Allocate the vector of loops.
   */
  auto allLoops = new std::vector<LoopDependenceInfo *>();

  /*
   * Check if the function is hot.
   */
  if (!isFunctionHot(function, minimumHotness)){
    return allLoops ;
  }

  /*
   * Fetch the loop analysis.
   */
  auto& LI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();

  /*
   * Check if the function has loops.
   */
  if (std::distance(LI.begin(), LI.end()) == 0){
    return allLoops;
  }

  /*
   * Fetch the function dependence graph.
   */
  auto funcPDG = this->getFunctionDependenceGraph(function);

  /*
   * Fetch the post dominators and scalar evolutions
   */
  auto DS = this->getDominators(function);
  auto& SE = getAnalysis<ScalarEvolutionWrapperPass>(*function).getSE();

  /*
   * Fetch all loops of the current function.
   */
  auto loops = LI.getLoopsInPreorder();

  /*
   * Consider these loops.
   */
  for (auto loop : loops){

    /*
     * Check if the loop is hot enough.
     */
    LoopStructure loopS{loop};
    if (!isLoopHot(&loopS, minimumHotness)){
      continue ;
    }

    /*
     * Allocate the loop wrapper.
     */
    auto ldi = new LoopDependenceInfo(funcPDG, loop, *DS, SE, this->maxCores, {}, this->loopAA, this->loopAwareDependenceAnalysis);
    allLoops->push_back(ldi);
  }

  /*
   * Free the memory.
   */
  delete DS ;

  return allLoops;
}

std::vector<LoopDependenceInfo *> * Noelle::getLoops (void){
  auto v = this->getLoops(this->minHot);

  return v;
}

std::vector<LoopDependenceInfo *> * Noelle::getLoops (
    double minimumHotness
    ){

  /*
   * Fetch the profiles.
   */
  auto profiles = this->getProfiles();

  /*
   * Allocate the vector of loops.
   */
  auto allLoops = new std::vector<LoopDependenceInfo *>();

  /*
   * Fetch the list of functions of the module.
   */
  auto mainFunction = this->getEntryFunction();
  assert(mainFunction != nullptr);
  auto functions = this->getModuleFunctionsReachableFrom(this->program, mainFunction);

  /*
   * Check if we should filter out loops.
   */
  auto filterLoops = this->checkToGetLoopFilteringInfo();

  /*
   * Append loops of each function.
   */
  auto nextLoopIndex = 0;
  if (this->verbose >= Verbosity::Maximal){
    errs() << "Parallelizer: Filter out cold code\n" ;
  }
  for (auto function : *functions){

    /*
     * Check if the function is hot.
     */
    if (!isFunctionHot(function, minimumHotness)){
      errs() << "Parallelizer:  Disable \"" << function->getName() << "\" as cold function\n";
      continue ;
    }

    /*
     * Fetch the loop analysis.
     */
    auto& LI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();

    /*
     * Check if the function has loops.
     */
    if (std::distance(LI.begin(), LI.end()) == 0){
      continue ;
    }

    /*
     * Fetch the function dependence graph.
     */
    auto funcPDG = this->getFunctionDependenceGraph(function);

    /*
     * Fetch the post dominators and scalar evolutions
     */
    auto DS = this->getDominators(function);
    auto& SE = getAnalysis<ScalarEvolutionWrapperPass>(*function).getSE();

    /*
     * Fetch all loops of the current function.
     */
    auto loops = LI.getLoopsInPreorder();

    /*
     * Consider these loops.
     */
    for (auto loop : loops){
      auto currentLoopIndex = nextLoopIndex++;

      /*
       * Check if the loop is hot enough.
       */
      LoopStructure loopS{loop};
      if (!isLoopHot(&loopS, minimumHotness)){
        errs() << "Parallelizer:  Disable loop \"" << currentLoopIndex << "\" as cold code\n";
        continue ;
      }

      // TODO: Print out more information than just loop hotness, perhaps the loop header label
      // errs() << "Parallelizer:  Loop hotness = " << hotness << "\n" ;

      /*
       * Check if we have to filter loops.
       */
      if (!filterLoops){

        /*
         * Allocate the loop wrapper.
         */
        auto ldi = new LoopDependenceInfo(funcPDG, loop, *DS, SE, this->maxCores, {}, this->loopAA, this->loopAwareDependenceAnalysis);

        allLoops->push_back(ldi);
        continue ;
      }

      /*
       * We need to filter loops.
       *
       * Check if more than one thread is assigned to the current loop.
       * If that's the case, then we have to enable that loop.
       */
      auto maximumNumberOfCoresForTheParallelization = this->loopThreads[currentLoopIndex];
      if (maximumNumberOfCoresForTheParallelization <= 1){

        /*
         * Only one thread has been assigned to the current loop.
         * Hence, the current loop will not be parallelized.
         *
         * Jump to the next loop.
         */
        continue ;
      }

      /*
       * Safety code.
       */
      if (this->hasReadFilterFile && currentLoopIndex >= loopThreads.size()){
        errs() << "ERROR: the 'INDEX_FILE' file isn't correct. There are more than " << loopThreads.size() << " loops available in the program\n";
        abort();
      }

      auto ldi = getLoopDependenceInfoForLoop(
          loop,
          funcPDG,
          DS,
          &SE,
          this->techniquesToDisable[currentLoopIndex],
          this->DOALLChunkSize[currentLoopIndex],
          maximumNumberOfCoresForTheParallelization
          );

      /*
       * The current loop needs to be considered as specified by the user.
       */
      allLoops->push_back(ldi);
    }

    /*
     * Free the memory.
     */
    delete DS;
  }

  /*
   * Free the memory.
   */
  delete functions;

  return allLoops;
}

uint32_t Noelle::getNumberOfProgramLoops (void) {
  return this->getNumberOfProgramLoops(this->minHot);
}

uint32_t Noelle::getNumberOfProgramLoops (
    double minimumHotness
    ){
  uint32_t counter = 0;

  /*
   * Fetch the profiles.
   */
  auto profiles = this->getProfiles();

  /*
   * Fetch the list of functions of the module.
   */
  auto mainFunction = this->getEntryFunction();
  assert(mainFunction != nullptr);
  auto functions = this->getModuleFunctionsReachableFrom(this->program, mainFunction);

  /*
   * Check if we should filter out loops.
   */
  auto filterLoops = this->checkToGetLoopFilteringInfo();

  /*
   * Append loops of each function.
   */
  auto currentLoopIndex = 0;
  for (auto function : *functions){

    /*
     * Fetch the loop analysis.
     */
    auto& LI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();

    /*
     * Check if the function has loops.
     */
    if (std::distance(LI.begin(), LI.end()) == 0){
      continue ;
    }

    /*
     * Check if the function is hot.
     */
    if (!isFunctionHot(function, minimumHotness)){
      continue ;
    }

    /*
     * Fetch all loops of the current function.
     */
    auto loops = LI.getLoopsInPreorder();

    /*
     * Consider these loops.
     */
    for (auto loop : loops){

      /*
       * Check if the loop is hot enough.
       */
      LoopStructure loopStructure{loop};
      if (!isLoopHot(&loopStructure, minimumHotness)) {
        currentLoopIndex++;
        continue ;
      }

      /*
       * Check if we have to filter loops.
       */
      if (!filterLoops){
        counter++;
        currentLoopIndex++;
        continue ;
      }

      /*
       * We need to filter loops.
       *
       * Check if more than one thread is assigned to the current loop.
       * If that's the case, then we have to enable that loop.
       */
      auto maximumNumberOfCoresForTheParallelization = loopThreads[currentLoopIndex];
      if (maximumNumberOfCoresForTheParallelization <= 1){

        /*
         * Only one thread has been assigned to the current loop.
         * Hence, the current loop will not be parallelized.
         */
        currentLoopIndex++;

        /*
         * Jump to the next loop.
         */
        continue ;
      }

      /*
       * The current loop has more than one core assigned to it.
       * Therefore, we need to parallelize this loop.
       * In other words, the current loop needs to be considered as specified by the user.
       */
      counter++;
      currentLoopIndex++;
    }
  }

  /*
   * Free the memory.
   */
  delete functions ;

  return counter;
}

bool Noelle::checkToGetLoopFilteringInfo (void) {

  /*
   * Check the name of the file that lists the loops to consider.
   * Check that the file hasn't been read already
   */
  if (!this->filterFileName || this->hasReadFilterFile){
    return false;
  }

  /*
   * We need to filter out loops.
   *
   * Open the file that specifies which loops to keep.
   */
  auto indexBuf = MemoryBuffer::getFileAsStream(this->filterFileName);
  if (auto ec = indexBuf.getError()){
    errs() << "Failed to read INDEX_FILE = \"" << this->filterFileName << "\":" << ec.message() << "\n";
    abort();
  }

  /*
   * Read the file.
   */
  auto fileAsString = indexBuf.get()->getBuffer().str();
  std::stringstream indexString{fileAsString};

  /*
   * Parse the file
   */
  auto filterLoops = false;
  while (indexString.peek() != EOF){
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

    /*
     * Skip
     */
    this->fetchTheNextValue(indexString);
    this->fetchTheNextValue(indexString);
    this->fetchTheNextValue(indexString);

    /*
     * If the loop needs to be parallelized, then we enable it.
     */
    if (  (shouldBeParallelized)    &&
        (cores >= 2)              ){
      this->loopThreads.push_back(cores);
      this->techniquesToDisable.push_back(technique);
      this->DOALLChunkSize.push_back(DOALLChunkFactor);

    } else{
      this->loopThreads.push_back(1);
      this->techniquesToDisable.push_back(0);
      this->DOALLChunkSize.push_back(0);
    }
  }

  this->hasReadFilterFile = true;
  return filterLoops;
}

void Noelle::sortByHotness (std::vector<LoopDependenceInfo *> & loops) {

  /*
   * Fetch the profiles.
   */
  auto hot = this->getProfiles();

  /*
   * Define the order between loops.
   */
  auto compareLoops = [hot] (LoopDependenceInfo *a, LoopDependenceInfo *b) -> bool {
    auto aLS = a->getLoopStructure();
    auto bLS = b->getLoopStructure();
    auto aInsts = hot->getTotalInstructions(aLS);
    auto bInsts = hot->getTotalInstructions(bLS);

    return aInsts >= bInsts;
  };

  /*
   * Sort the loops.
   */
  std::sort(loops.begin(), loops.end(), compareLoops);

  return ;
}

void Noelle::sortByHotness (std::vector<LoopStructure *> & loops) {

  /*
   * Fetch the profiles.
   */
  auto hot = this->getProfiles();

  /*
   * Define the order between loops.
   */
  auto compareLoops = [hot] (LoopStructure *a, LoopStructure *b) -> bool {
    auto aInsts = hot->getTotalInstructions(a);
    auto bInsts = hot->getTotalInstructions(b);
    return aInsts >= bInsts;
  };

  /*
   * Sort the loops.
   */
  std::sort(loops.begin(), loops.end(), compareLoops);

  return;
}


void Noelle::sortByStaticNumberOfInstructions (std::vector<LoopDependenceInfo *> & loops) {

  /*
   * Define the order between loops.
   */
  auto compareLoops = [] (LoopDependenceInfo *a, LoopDependenceInfo *b) -> bool {
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

  return ;
}

LoopDependenceInfo * Noelle::getLoopDependenceInfoForLoop (
    Loop *loop,
    PDG *functionPDG,
    DominatorSummary *DS,
    ScalarEvolution *SE,
    uint32_t techniquesToDisableForLoop,
    uint32_t DOALLChunkSizeForLoop,
    uint32_t maxCores
    ) {

  auto ldi = new LoopDependenceInfo(functionPDG, loop, *DS, *SE, maxCores, {}, this->loopAA, this->loopAwareDependenceAnalysis);

  /*
   * Set the loop constraints specified by INDEX_FILE.
   *
   * DOALL chunk size is the one defined by INDEX_FILE + 1. This is because chunk size must start from 1.
   */
  ldi->DOALLChunkSize = DOALLChunkSizeForLoop + 1;

  /*
   * Set the techniques that are enabled.
   */
  auto disableTransformations = techniquesToDisableForLoop;
  switch (disableTransformations){

    case 0:
      ldi->enableAllTransformations();
      break ;

    case 1:
      ldi->disableTransformation(DSWP_ID);
      break ;

    case 2:
      ldi->disableTransformation(HELIX_ID);
      break ;

    case 3:
      ldi->disableTransformation(DOALL_ID);
      break ;

    case 4:
      ldi->disableTransformation(DSWP_ID);
      ldi->disableTransformation(HELIX_ID);
      break ;

    case 5:
      ldi->disableTransformation(DSWP_ID);
      ldi->disableTransformation(DOALL_ID);
      break ;

    case 6:
      ldi->disableTransformation(HELIX_ID);
      ldi->disableTransformation(DOALL_ID);
      break ;

    default:
      abort();
  }

  return ldi;
}

bool Noelle::isLoopHot (LoopStructure *loopStructure, double minimumHotness) {
  if (!profiles->isAvailable()) {
    return true;
  }

  auto hotness = profiles->getDynamicTotalInstructionCoverage(loopStructure);
  return hotness >= minimumHotness;
}

bool Noelle::isFunctionHot (Function *function, double minimumHotness) {
  if (!profiles->isAvailable()) {
    return true;
  }

  auto hotness = profiles->getDynamicTotalInstructionCoverage(function);
  return hotness >= minimumHotness;
}

void Noelle::filterOutLoops (
    std::vector<LoopStructure *> & loops,
    std::function<bool (LoopStructure *)> filter
    ) {

  /*
   * Tag the loops that need to be removed.
   */
  std::vector<uint64_t> toDelete{};
  uint64_t currentIndex = 0;
  for (auto loop : loops){
    if (filter(loop)){
      toDelete.insert(toDelete.begin(), currentIndex);
    }
    currentIndex++;
  }

  /*
   * Remove the loops.
   */
  for (auto index : toDelete){
    loops.erase(loops.begin() + index);
  }

  return ;
}

void Noelle::filterOutLoops (
  noelle::StayConnectedNestedLoopForest *f, 
  std::function<bool (LoopStructure *)> filter
  ) {

  /*
   * Iterate over the trees and find the nodes to delete.
   */
  std::vector<noelle::StayConnectedNestedLoopForestNode *> toDelete{};
  for (auto tree : f->getTrees()){
    auto myF = [&filter, &toDelete](noelle::StayConnectedNestedLoopForestNode *n, uint32_t l) -> bool {
      auto ls = n->getLoop();
      if (filter(ls)){
        toDelete.push_back(n);
      }
      return false;
    };
    tree->visitPreOrder(myF);
  }

  /*
   * Delete the nodes.
   */
  for (auto n : toDelete){
    delete n;
  }

  return ;
}

noelle::StayConnectedNestedLoopForest * Noelle::organizeLoopsInTheirNestingForest (
  std::vector<LoopStructure *> const & loops
  ) {

  /*
   * Compute the dominators.
   */
  std::unordered_map<Function *, DominatorSummary *> doms{};
  for (auto loop : loops){
    auto f = loop->getFunction();
    if (doms.find(f) != doms.end()){
      continue ;
    }
    doms[f] = this->getDominators(f);
  }

  /*
   * Compute the forest.
   */
  auto n = new noelle::StayConnectedNestedLoopForest(loops, doms);

  /*
   * Free the memory.
   */
  for (auto pair : doms){
    delete pair.second;
  }

  return n;
}

}
