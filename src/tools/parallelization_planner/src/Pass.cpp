/*
 * Copyright 2016 - 2022 Kevin McAfee, Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Planner.hpp"

namespace llvm::noelle {

/*
 * Options of the Planner pass.
 */
static cl::opt<bool> ForceParallelizationPlanner("noelle-parallelizer-force", cl::ZeroOrMore, cl::Hidden, cl::desc("Force the parallelization"));

Planner::Planner()
  :
    ModulePass{ID}, 
    forceParallelization{false}
{

  return ;
}

bool Planner::doInitialization (Module &M) {
  this->forceParallelization = (ForceParallelizationPlanner.getNumOccurrences() > 0);

  return false; 
}

bool Planner::runOnModule (Module &M) {
  errs() << "Planner: Start\n";

  /*
   * Fetch the outputs of the passes we rely on.
   */
  auto& noelle = getAnalysis<Noelle>();

  /*
   * Fetch the profiles.
   */
  auto profiles = noelle.getProfiles();

  /*
   * Fetch the verbosity level.
   */
  auto verbosity = noelle.getVerbosity();


  /*
   * Fetch all the loops we want to parallelize.
   */
  errs() << "Planner:  Fetching the program loops\n";
  auto programLoops = noelle.getLoopStructures();
  if (programLoops->size() == 0){
    errs() << "Planner:    There is no loop to consider\n";

    /*
     * Free the memory.
     */
    delete programLoops;

    errs() << "Planner: Exit\n";
    return false;
  }
  errs() << "Planner:    There are " << programLoops->size() << " loops in the program we are going to consider\n";

  /*
   * Compute the nesting forest.
   */
  auto forest = noelle.organizeLoopsInTheirNestingForest(*programLoops);
  delete programLoops ;

  /*
   * Filter out loops that are not worth parallelizing.
   */
  if (!this->forceParallelization){
    this->removeLoopsNotWorthParallelizing(noelle, profiles, forest);
  }

  /*
   * Plan parallelization of the loops selected.
   *
   * Parallelize the loops starting from the outermost to the inner ones.
   * This is accomplished by having sorted the loops above.
   */
  auto modified = false;
  uint32_t parallelizationOrderIndex = 0;
  auto mm = noelle.getMetadataManager();
  for (auto tree : forest->getTrees()){
    /*
     * Select the loops to parallelize.
     */
    auto loopsToParallelize = this->selectTheOrderOfLoopsToParallelize(noelle, profiles, tree);

    /*
     * Attach metadata representing the loop's order in the parallelization plan to each loop we are considering.
     */
    for (auto ldi : loopsToParallelize) {
      auto ls = ldi->getLoopStructure();
      auto ldiParallelizationOrderIndex = std::to_string(parallelizationOrderIndex++);
      mm->addMetadata(ls, "noelle.parallelizer.looporder", ldiParallelizationOrderIndex);
      modified = true;
    }

    /*
     * Free the memory.
     */
    for (auto loop : loopsToParallelize){
      delete loop;
    }
  }

  errs() << "Planner: Exit\n";
  return modified;
}

void Planner::getAnalysisUsage (AnalysisUsage &AU) const {

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

  return ;
}

}

// Next there is code to register your pass to "opt"
char llvm::noelle::Planner::ID = 0;
static RegisterPass<Planner> X("planner", "Automatic parallelization planner");

// Next there is code to register your pass to "clang"
static Planner * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
    if(!_PassMaker){ PM.add(_PassMaker = new Planner());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
    if(!_PassMaker){ PM.add(_PassMaker = new Planner());}});// ** for -O0
