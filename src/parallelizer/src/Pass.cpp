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
static cl::opt<bool> ForceParallelization("noelle-parallelizer-force", cl::ZeroOrMore, cl::Hidden, cl::desc("Force the parallelization"));
static cl::opt<bool> ForceNoSCCPartition("dswp-no-scc-merge", cl::ZeroOrMore, cl::Hidden, cl::desc("Force no SCC merging when parallelizing"));

Parallelizer::Parallelizer()
  :
  ModulePass{ID}, 
  forceParallelization{false},
  forceNoSCCPartition{false}
  {

  return ;
}

bool Parallelizer::doInitialization (Module &M) {
  this->forceParallelization = (ForceParallelization.getNumOccurrences() > 0);
  this->forceNoSCCPartition = (ForceNoSCCPartition.getNumOccurrences() > 0);

  return false; 
}

bool Parallelizer::runOnModule (Module &M) {

  /*
   * Fetch the outputs of the passes we rely on.
   */
  auto& noelle = getAnalysis<Noelle>();
  auto heuristics = getAnalysis<HeuristicsPass>().getHeuristics(noelle);

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
    this->forceParallelization,
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
  auto programLoops = noelle.getLoopStructures();
  errs() << "Parallelizer:  There are " << programLoops->size() << " loops in the program we are going to consider\n";

  /*
   * Compute the nesting forest.
   */
  auto forest = noelle.organizeLoopsInTheirNestingForest(*programLoops);
  delete programLoops ;

  /*
   * Filter out loops that are not worth parallelizing.
   */
  errs() << "Parallelizer:  Filter out loops not worth considering\n";
  auto filter = [this, forest, profiles](LoopStructure *ls) -> bool{

    /*
     * Fetch the loop ID.
     */
    auto loopID = ls->getID();

    /*
     * Check if the latency of each loop invocation is enough to justify the parallelization.
     */
    auto averageInstsPerInvocation = profiles->getAverageTotalInstructionsPerInvocation(ls);
    auto averageInstsPerInvocationThreshold = 2000;
    if (  true
          && (!this->forceParallelization)
          && (averageInstsPerInvocation < averageInstsPerInvocationThreshold)
      ){
      errs() << "Parallelizer:    Loop " << loopID << " has " << averageInstsPerInvocation << " number of instructions per loop invocation\n";
      errs() << "Parallelizer:      It is too low. The threshold is " << averageInstsPerInvocationThreshold << "\n";

      /*
       * Remove the loop.
       */
      return true;
    }

    /*
     * Check the number of iterations per invocation.
     */
    auto averageIterations = profiles->getAverageLoopIterationsPerInvocation(ls);
    auto averageIterationThreshold = 12;
    if (  true
          && (!this->forceParallelization)
          && (averageIterations < averageIterationThreshold)
      ){
      errs() << "Parallelizer:    Loop " << loopID << " has " << averageIterations << " number of iterations on average per loop invocation\n";
      errs() << "Parallelizer:      It is too low. The threshold is " << averageIterationThreshold << "\n";

      /*
       * Remove the loop.
       */
      return true;
    }

    return false;
  };
  noelle.filterOutLoops(forest, filter);

  /*
   * Print the loops.
   */
  auto trees = forest->getTrees();
  errs() << "Parallelizer:  There are " << trees.size() << " loop nesting trees in the program\n";
  for (auto tree : trees){

    /*
     * Print the root.
     */
    auto loopStructure = tree->getLoop();
    auto loopID = loopStructure->getID();

    /*
     * Print the tree.
     */
    auto printTree = [profiles](noelle::StayConnectedNestedLoopForestNode *n, uint32_t treeLevel) {

      /*
       * Fetch the loop information.
       */
      auto loopStructure = n->getLoop();
      auto loopID = loopStructure->getID();
      auto loopFunction = loopStructure->getFunction();
      auto loopHeader = loopStructure->getHeader();

      /*
       * Compute the print prefix.
       */
      std::string prefix{"Parallelizer:    "};
      for (auto i = 1 ; i < treeLevel; i++){
        prefix.append("  ");
      }

      /*
       * Print the loop.
       */
      errs() << prefix << "ID: " << loopID << " (" << treeLevel << ")\n";
      errs() << prefix << "  Function: \"" << loopFunction->getName() << "\"\n";
      errs() << prefix << "  Loop: \"" << *loopHeader->getFirstNonPHI() << "\"\n";
      errs() << prefix << "  Loop nesting level: " << loopStructure->getNestingLevel() << "\n";

      /*
       * Check if there are profiles.
       */
      if (!profiles->isAvailable()){
        return false;
      }

      /*
       * Print the coverage of this loop.
       */
      auto hotness = profiles->getDynamicTotalInstructionCoverage(loopStructure) * 100;
      errs() << prefix << "  Hotness = " << hotness << " %\n"; 
      auto averageInstsPerInvocation = profiles->getAverageTotalInstructionsPerInvocation(loopStructure);
      errs() << prefix << "  Average instructions per invocation = " << averageInstsPerInvocation << " %\n"; 
      auto averageIterations = profiles->getAverageLoopIterationsPerInvocation(loopStructure);
      errs() << prefix << "  Average iterations per invocation = " << averageIterations << " %\n"; 
      errs() << prefix << "\n";

      return false;
    };
    tree->visitPreOrder(printTree);
  }

  /*
   * Parallelize the loops selected.
   *
   * Parallelize the loops starting from the outermost to the inner ones.
   * This is accomplished by having sorted the loops above.
   */
  auto modified = false;
  std::unordered_map<BasicBlock *, bool> modifiedBBs{};
  for (auto tree : forest->getTrees()){

    /*
     * Select the loops to parallelize.
     */
    auto loopsToParallelize = this->selectTheOrderOfLoopsToParallelize(noelle, profiles, tree);

    /*
     * Parallelize the loops.
     */
    for (auto ldi : loopsToParallelize){

      /*
       * Check if we can parallelize this loop.
       */
      auto ls = ldi->getLoopStructure();
      auto safe = true;
      for (auto bb : ls->getBasicBlocks()){
        if (modifiedBBs[bb]){
          safe = false;
          break ;
        }
      }
      auto loopID = ls->getID();
      if (!safe){
        errs() << "Parallelizer:    Loop " << loopID << " cannot be parallelized because one of its parent has been parallelized already\n";
        continue ;
      }

      /*
       * Parallelize the current loop.
       */
      auto loopIsParallelized = this->parallelizeLoop(ldi, noelle, dswp, doall, helix, heuristics);

      /*
       * Keep track of the parallelization.
       */
      if (loopIsParallelized){
        errs() << "Parallelizer:    Loop " << loopID << " has been parallelized\n";
        modified = true;
        for (auto bb : ls->getBasicBlocks()){
          modifiedBBs[bb] = true;
        }
      }
    }

    /*
     * Free the memory.
     */
    for (auto loop : loopsToParallelize){
      delete loop;
    }
  }

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
