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
  if (profiles->isAvailable()){
    auto mInsts = profiles->getTotalInstructions();
    auto fInsts = profiles->getTotalInstructions(function);
    auto hotness = ((double)fInsts) / ((double)mInsts);
    if (hotness < minimumHotness){
      return allLoops ;
    }
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
    if (profiles->isAvailable()){
      auto mInsts = profiles->getTotalInstructions();
      LoopStructure loopS{loop};
      auto lInsts = profiles->getTotalInstructions(&loopS);
      auto hotness = ((double)lInsts) / ((double)mInsts);
      if (hotness < minimumHotness){
        continue ;
      }
    }

    /*
     * Allocate the loop wrapper.
     */
    auto ldi = new LoopDependenceInfo(funcPDG, loop, *DS, SE, this->maxCores);
    allLoops->push_back(ldi);
  }

  /*
   * Free the memory.
   */
  delete DS ;

  return allLoops;
}

std::vector<LoopDependenceInfo *> * Noelle::getProgramLoops (void){
  auto v = this->getProgramLoops(this->minHot);

  return v;
}

std::vector<LoopDependenceInfo *> * Noelle::getProgramLoops (
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
  std::vector<uint32_t> loopThreads{};
  std::vector<uint32_t> DOALLChunkSize{};
  std::vector<uint32_t> techniquesToDisable{};
  auto indexFileName = getenv("INDEX_FILE");
  auto filterLoops = this->filterOutLoops(indexFileName, loopThreads, techniquesToDisable, DOALLChunkSize);

  /*
   * Append loops of each function.
   */
  auto currentLoopIndex = 0;
  if (this->verbose >= Verbosity::Maximal){
    errs() << "Parallelizer: Filter out cold code\n" ;
  }
  for (auto function : *functions){

    /*
     * Check if the function is hot.
     */
    if (profiles->isAvailable()){
      auto hotness = profiles->getDynamicTotalInstructionCoverage(function);
      if (hotness < minimumHotness){
        errs() << "Parallelizer:  Disable \"" << function->getName() << "\" as cold function\n";
        continue ;
      }
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
    auto PDG = this->getProgramDependenceGraph();
    auto funcPDG = PDG->createFunctionSubgraph(*function);

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
      if (profiles->isAvailable()){
        LoopStructure loopS{loop};
        auto hotness = profiles->getDynamicTotalInstructionCoverage(&loopS);
        if (hotness < minimumHotness){
          errs() << "Parallelizer:  Disable loop \"" << currentLoopIndex << "\" as cold code\n";
          currentLoopIndex++;
          continue ;
        }
        errs() << "Parallelizer:  Loop hotness = " << hotness << "\n" ;
      }

      /*
       * Check if we have to filter loops.
       */
      if (!filterLoops){

        /*
         * Allocate the loop wrapper.
         */
        auto ldi = new LoopDependenceInfo(funcPDG, loop, *DS, SE, this->maxCores);

        allLoops->push_back(ldi);
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
       * Safety code.
       */
      if (currentLoopIndex >= loopThreads.size()){
        errs() << "ERROR: the 'INDEX_FILE' file isn't correct. There are more than " << loopThreads.size() << " loops available in the program\n";
        abort();
      }

      /*
       * The current loop has more than one core assigned to it.
       * Therefore, we need to parallelize this loop.
       *
       * Allocate the loop wrapper.
       */
      auto ldi = new LoopDependenceInfo(funcPDG, loop, *DS, SE, maximumNumberOfCoresForTheParallelization);

      /*
       * Set the loop constraints specified by INDEX_FILE.
       *
       * DOALL chunk size is the one defined by INDEX_FILE + 1. This is because chunk size must start from 1.
       */
      ldi->DOALLChunkSize = DOALLChunkSize[currentLoopIndex] + 1;

      /*
       * Set the techniques that are enabled.
       */
      auto disableTransformations = techniquesToDisable[currentLoopIndex];
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

      /*
       * The current loop needs to be considered as specified by the user.
       */
      allLoops->push_back(ldi);
      currentLoopIndex++;
    }

    /*
     * Free the memory.
     */
    delete funcPDG;
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
  std::vector<uint32_t> loopThreads{};
  std::vector<uint32_t> techniquesToDisable{};
  std::vector<uint32_t> DOALLChunkSize{};
  auto indexFileName = getenv("INDEX_FILE");
  auto filterLoops = this->filterOutLoops(indexFileName, loopThreads, techniquesToDisable, DOALLChunkSize);

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
    if (profiles->isAvailable()){
      auto hotness = profiles->getDynamicTotalInstructionCoverage(function);
      if (hotness <= minimumHotness){
        continue ;
      }
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
       if (profiles->isAvailable()){
        LoopStructure loopS{loop};
        auto hotness = profiles->getDynamicTotalInstructionCoverage(&loopS);
        if (hotness <= minimumHotness){
          currentLoopIndex++;
          continue ;
        }
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

bool Noelle::filterOutLoops (
  char *fileName,
  std::vector<uint32_t>& loopThreads,
  std::vector<uint32_t>& techniquesToDisable,
  std::vector<uint32_t>& DOALLChunkSize
  ){

  /*
   * Check the name of the file that lists the loops to consider.
   */
  if (!fileName){
    return false;
  }

  /*
   * We need to filter out loops.
   *
   * Open the file that specifies which loops to keep.
   */
  auto indexBuf = MemoryBuffer::getFileAsStream(fileName);
  if (auto ec = indexBuf.getError()){
    errs() << "Failed to read INDEX_FILE = \"" << fileName << "\":" << ec.message() << "\n";
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
      loopThreads.push_back(cores);
      techniquesToDisable.push_back(technique);
      DOALLChunkSize.push_back(DOALLChunkFactor);

    } else{
      loopThreads.push_back(1);
      techniquesToDisable.push_back(0);
      DOALLChunkSize.push_back(0);
    }
  }
  
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
