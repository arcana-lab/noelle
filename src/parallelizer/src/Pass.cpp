/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Parallelizer.hpp"

using namespace llvm;

/*
 * Options of the Parallelizer pass.
 */
static cl::opt<bool> ForceParallelization("dswp-force", cl::ZeroOrMore, cl::Hidden, cl::desc("Force the parallelization"));
static cl::opt<bool> ForceNoSCCPartition("dswp-no-scc-merge", cl::ZeroOrMore, cl::Hidden, cl::desc("Force no SCC merging when parallelizing"));
static cl::opt<bool> DisableLoopSorting("noelle-parallelizer-disable-loop-sorting", cl::ZeroOrMore, cl::Hidden, cl::desc("Disable sorting loops to parallelize"));

Parallelizer::Parallelizer()
  :
  ModulePass{ID}, 
  forceParallelization{false},
  forceNoSCCPartition{false},
  disableLoopSorting{false}
  {

  return ;
}

bool Parallelizer::doInitialization (Module &M) {
  this->forceParallelization = (ForceParallelization.getNumOccurrences() > 0);
  this->forceNoSCCPartition = (ForceNoSCCPartition.getNumOccurrences() > 0);
  this->disableLoopSorting = (DisableLoopSorting.getNumOccurrences() > 0);

  return false; 
}

bool Parallelizer::runOnModule (Module &M) {

  /*
   * Fetch the outputs of the passes we rely on.
   */
  auto& noelle = getAnalysis<Noelle>();
  auto heuristics = getAnalysis<HeuristicsPass>().getHeuristics();

  /*
   * Fetch the profiles.
   */
  auto profiles = noelle.getProfiles();

  /*
   * Fetch the verbosity level.
   */
  auto verbosity = noelle.getVerbosity();

  /*
   * Allocate the parallelization techniques.
   */
  DSWP dswp{
    M,
    *profiles,
    this->forceParallelization,
    !this->forceNoSCCPartition,
    verbosity
  };
  DOALL doall{
    M,
    *profiles,
    verbosity
  };
  HELIX helix{
    M,
    *profiles,
    verbosity
  };

  /*
   * Collect information about C++ code we link parallelized loops with.
   */
  errs() << "Parallelizer: Analyzing the module " << M.getName() << "\n";
  if (!collectThreadPoolHelperFunctionsAndTypes(M, noelle)) {
    errs() << "Parallelizer utils not included!\n";
    return false;
  }

  /*
   * Fetch all the loops we want to parallelize.
   */
  auto loopsToParallelize = noelle.getProgramLoops();
  errs() << "Parallelizer:  There are " << loopsToParallelize->size() << " loops to parallelize\n";

  /*
   * Sort them by their hotness.
   */
  if (this->disableLoopSorting){
    noelle.sortByHotness(*loopsToParallelize);
  }
  for (auto loop : *loopsToParallelize){

    /*
     * Fetch the header.
     */
    auto loopSummary = loop->getLoopStructure();
    auto loopHeader = loopSummary->getHeader();

    /*
     * Fetch the function.
     */
    auto loopFunction = loopSummary->getFunction();

    /*
     * Print information about this loop.
     */
    errs() << "Parallelizer:    ID: " << loop->getID() << "\n";
    errs() << "Parallelizer:    Function: \"" << loopFunction->getName() << "\"\n";
    errs() << "Parallelizer:    Loop: \"" << *loopHeader->getFirstNonPHI() << "\"\n";
    if (!profiles->isAvailable()){
      continue ;
    }

    /*
     * Print the coverage of this loop.
     */
    auto mInsts = profiles->getTotalInstructions();
    auto loopInsts = profiles->getTotalInstructions(loopSummary);
    auto hotness = ((double)loopInsts) / ((double)mInsts);
    hotness *= 100;
    errs() << "Parallelizer:      Hotness = " << hotness << " %\n"; 
  }

  /*
   * Parallelize the loops selected.
   *
   * Parallelize the loops starting from the outermost to the inner ones.
   * This is accomplished by having sorted the loops above.
   */
  errs() << "Parallelizer:  Parallelize " << loopsToParallelize->size() << " loops, one at a time\n";
  auto modified = false;
  std::unordered_map<BasicBlock *, bool> modifiedBBs;
  for (auto loop : *loopsToParallelize){

    /*
     * Check if we can parallelize this loop.
     */
    auto safe = true;
    auto ls = loop->getLoopStructure();
    for (auto bb : ls->getBasicBlocks()){
      if (modifiedBBs[bb]){
        safe = false;
        break ;
      }
    }
    if (!safe){
      errs() << "Parallelizer:    Loop " << loop->getID() << " cannot be parallelized because one of its parent has been parallelized already\n";
      continue ;
    }
    
    /*
     * Parallelize the current loop.
     */
    auto loopIsParallelized = this->parallelizeLoop(loop, noelle, dswp, doall, helix, heuristics);

    /*
     * Keep track of the parallelization.
     */
    if (loopIsParallelized){
      errs() << "Parallelizer:    Loop " << loop->getID() << " has been parallelized\n";
      modified = true;
      for (auto bb : ls->getBasicBlocks()){
        modifiedBBs[bb] = true;
      }
    }
  }

  /*
   * Free the memory.
   */
  delete loopsToParallelize;

  return modified;
}

void Parallelizer::getAnalysisUsage (AnalysisUsage &AU) const {

  /*
   * Analyses.
   */
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();

  /*
   * Noelle.
   */
  AU.addRequired<Noelle>();
  AU.addRequired<HeuristicsPass>();

  return ;
}

// Next there is code to register your pass to "opt"
char llvm::Parallelizer::ID = 0;
static RegisterPass<Parallelizer> X("parallelizer", "Automatic parallelization of sequential code");

// Next there is code to register your pass to "clang"
static Parallelizer * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new Parallelizer());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new Parallelizer());}});// ** for -O0
