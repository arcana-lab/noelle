/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Parallelizer.hpp"

namespace llvm::noelle {

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
  errs() << "Parallelizer: Start\n";

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
   * Collect information about C++ code we link parallelized loops with.
   */
  errs() << "Parallelizer:  Analyzing the module " << M.getName() << "\n";
  if (!collectThreadPoolHelperFunctionsAndTypes(M, noelle)) {
    errs() << "Parallelizer:    ERROR: I could not find the runtime within the module\n";
    return false;
  }

  /*
   * Fetch all the loops we want to parallelize.
   */
  errs() << "Parallelizer:  Fetching the program loops\n";
  auto programLoops = noelle.getLoopStructures();
  if (programLoops->size() == 0){
    errs() << "Parallelizer:    There is no loop to consider\n";

    /*
     * Free the memory.
     */
    delete programLoops;

    errs() << "Parallelizer: Exit\n";
    return false;
  }
  errs() << "Parallelizer:    There are " << programLoops->size() << " loops in the program we are going to consider\n";

  auto forest = noelle.organizeLoopsInTheirNestingForest(*programLoops);
  delete programLoops;

  /*
   * Determine the parallelization order from the metadata.
   */
  auto mm = noelle.getMetadataManager();
  std::map<uint32_t, LoopDependenceInfo*> loopParallelizationOrder;
  for (auto tree : forest->getTrees()) {
    auto selector = [&noelle, &mm, &loopParallelizationOrder](StayConnectedNestedLoopForestNode *n, uint32_t treeLevel) -> bool {
      auto ls = n->getLoop();
      if (!mm->doesHaveMetadata(ls, "noelle.parallelizer.looporder")) {
        return false;
      }
      auto parallelizationOrderIndex = std::stoi(mm->getMetadata(ls, "noelle.parallelizer.looporder"));
      auto optimizations = { LoopDependenceInfoOptimization::MEMORY_CLONING_ID, LoopDependenceInfoOptimization::THREAD_SAFE_LIBRARY_ID};
      auto ldi = noelle.getLoop(ls, optimizations);      
      loopParallelizationOrder[parallelizationOrderIndex] = ldi;
      return false;
    };
    tree->visitPreOrder(selector);
  }

  /*
   * Parallelize the loops in order.
   */
  auto modified = false;
  std::unordered_map<BasicBlock *, bool> modifiedBBs{};
  for (auto indexLoopPair : loopParallelizationOrder) {
    auto ldi = indexLoopPair.second;
    
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
    auto loopIsParallelized = this->parallelizeLoop(ldi, noelle, heuristics);

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
  for (auto indexLoopPair : loopParallelizationOrder){
    delete indexLoopPair.second;
  }

  errs() << "Parallelizer: Exit\n";
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

}

// Next there is code to register your pass to "opt"
char llvm::noelle::Parallelizer::ID = 0;
static RegisterPass<Parallelizer> X("parallelizer", "Automatic parallelization of sequential code");

// Next there is code to register your pass to "clang"
static Parallelizer * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
    if(!_PassMaker){ PM.add(_PassMaker = new Parallelizer());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
    if(!_PassMaker){ PM.add(_PassMaker = new Parallelizer());}});// ** for -O0
