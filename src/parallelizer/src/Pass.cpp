/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Parallelizer.hpp"
#include "HotProfiler.hpp"
#include "LoopDistribution.hpp"
#include <unordered_map>

using namespace llvm;

/*
 * Options of the Parallelizer pass.
 */
static cl::opt<bool> ForceParallelization("dswp-force", cl::ZeroOrMore, cl::Hidden, cl::desc("Force the parallelization"));
static cl::opt<bool> ForceNoSCCPartition("dswp-no-scc-merge", cl::ZeroOrMore, cl::Hidden, cl::desc("Force no SCC merging when parallelizing"));
static cl::opt<int> Verbose("noelle-verbose", cl::ZeroOrMore, cl::Hidden, cl::desc("Verbose output (0: disabled, 1: minimal, 2: maximal)"));
static cl::opt<int> MinimumHotness("noelle-min-hot", cl::ZeroOrMore, cl::Hidden, cl::desc("Minimum hotness of code to be parallelized"));
static cl::opt<bool> DisableDSWP("noelle-disable-dswp", cl::ZeroOrMore, cl::Hidden, cl::desc("Disable DSWP"));
static cl::opt<bool> DisableHELIX("noelle-disable-helix", cl::ZeroOrMore, cl::Hidden, cl::desc("Disable HELIX"));
static cl::opt<bool> DisableDOALL("noelle-disable-doall", cl::ZeroOrMore, cl::Hidden, cl::desc("Disable DOALL"));

Parallelizer::Parallelizer()
  :
  ModulePass{ID}, 
  forceParallelization{false},
  forceNoSCCPartition{false},
  verbose{Verbosity::Disabled},
  minHot{0}
  {

  return ;
}

bool Parallelizer::doInitialization (Module &M) {
  this->verbose = static_cast<Verbosity>(Verbose.getValue());
  this->minHot = ((double)(MinimumHotness.getValue())) / 100;
  this->forceParallelization |= (ForceParallelization.getNumOccurrences() > 0);
  this->forceNoSCCPartition |= (ForceNoSCCPartition.getNumOccurrences() > 0);

  /*
   * Enable parallelization techniques.
   */
  if (DisableDOALL.getNumOccurrences() == 0){
    this->enabledTechniques.insert(DOALL_ID);
  }
  if (DisableDSWP.getNumOccurrences() == 0){
    this->enabledTechniques.insert(DSWP_ID);
  }
  if (DisableHELIX.getNumOccurrences() == 0){
    this->enabledTechniques.insert(HELIX_ID);
  }

  return false; 
}

bool Parallelizer::runOnModule (Module &M) {

  /*
   * Fetch the outputs of the passes we rely on.
   */
  auto& parallelizationFramework = getAnalysis<Parallelization>();
  auto heuristics = getAnalysis<HeuristicsPass>().getHeuristics();
  auto& profiles = getAnalysis<HotProfiler>().getHot();
  auto& loopDist = getAnalysis<LoopDistribution>();

  /*
   * Allocate the parallelization techniques.
   */
  DSWP dswp{
    M,
    profiles,
    this->forceParallelization,
    !this->forceNoSCCPartition,
    this->verbose
  };
  DOALL doall{
    M,
    profiles,
    this->verbose
  };
  HELIX helix{
    M,
    profiles,
    this->verbose
  };

  /*
   * Collect information about C++ code we link parallelized loops with.
   */
  errs() << "Parallelizer: Analyzing the module " << M.getName() << "\n";
  if (!collectThreadPoolHelperFunctionsAndTypes(M, parallelizationFramework)) {
    errs() << "Parallelizer utils not included!\n";
    return false;
  }

  /*
   * Fetch all the loops we want to parallelize.
   */
  auto loopsToParallelize = parallelizationFramework.getModuleLoops(&M, this->minHot);
  errs() << "Parallelizer:  There are " << loopsToParallelize->size() << " loops to parallelize\n";
  for (auto loop : *loopsToParallelize){
    errs() << "Parallelizer:    Function: \"" << loop->function->getName() << "\"\n";
    errs() << "Parallelizer:    Loop: \"" << *loop->header->getFirstNonPHI() << "\"\n";
    if (profiles.isAvailable()){
      auto& profiles = getAnalysis<HotProfiler>().getHot();
      auto mInsts = profiles.getModuleInstructions();

      auto& LI = getAnalysis<LoopInfoWrapperPass>(*loop->function).getLoopInfo();
      auto loopInsts = profiles.getLoopInstructions(LI.getLoopFor(loop->header));
      auto hotness = ((double)loopInsts) / ((double)mInsts);
      hotness *= 100;
      errs() << "Parallelizer:      Hotness = " << hotness << " %\n"; 
    }
  }

  /*
   * Parallelize the loops selected.
   */
  errs() << "Parallelizer:  Parallelize all " << loopsToParallelize->size() << " loops, one at a time\n";
  auto modified = false;
  std::unordered_map<uint64_t, bool> modifiedLoops;
  for (auto loop : *loopsToParallelize){

    /*
     * Check if the loop can be parallelized.
     * This depends on whether the metadata (e.g., LoopDependenceInfo) are correct, which depends on whether its inner loops have been modified or not.
     */
    auto checkFunc = [&modifiedLoops](const LoopSummary &child) -> bool {

      /*
       * Fetch the ID of the subloop.
       */
      auto childID = child.getID();

      /*
       * Check if the sub-loop has been modified
       */
      if (modifiedLoops[childID]){

        /*
         * The subloop has been modified
         */
        return true;
      }

      /*
       * The subloop has not been modified
       */
      return false;
    };
    if (loop->iterateOverSubLoopsRecursively(checkFunc)){

      /*
       * A subloop has been modified.
       * Hence, we cannot trust the metadata of "loop".
       */
      continue ;
    }

    /*
     * Parallelize the current loop with Parallelizer.
     */
    auto loopID = loop->getID();
    modifiedLoops[loopID] |= this->parallelizeLoop(loop, parallelizationFramework, dswp, doall, helix, heuristics, loopDist);
    modified |= modifiedLoops[loopID];
  }
  errs() << "Parallelizer:  Parallelization complete\n";

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
  AU.addRequired<PDGAnalysis>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();

  /*
   * Parallelizations.
   */
  AU.addRequired<Parallelization>();
  AU.addRequired<HeuristicsPass>();

  /*
   * Parallelization enablers.
   */
  AU.addRequired<LoopDistribution>();

  /*
   * Profilers.
   */
  AU.addRequired<HotProfiler>();

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
