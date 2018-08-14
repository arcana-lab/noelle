#include "Parallelizer.hpp"

using namespace llvm;

/*
 * Options of the Parallelizer pass.
 */
static cl::opt<bool> ForceParallelization("dswp-force", cl::ZeroOrMore, cl::Hidden, cl::desc("Force the parallelization"));
static cl::opt<bool> ForceNoSCCPartition("dswp-no-scc-merge", cl::ZeroOrMore, cl::Hidden, cl::desc("Force no SCC merging when parallelizing"));
static cl::opt<int> Verbose("dswp-verbose", cl::ZeroOrMore, cl::Hidden, cl::desc("Verbose output (0: disabled, 1: minimal, 2: stage outline 3: maximal)"));

Parallelizer::Parallelizer()
  :
  ModulePass{ID}, 
  forceParallelization{false},
  forceNoSCCPartition{false},
  verbose{Verbosity::Disabled}
  {

  return ;
}

bool Parallelizer::doInitialization (Module &M) {
  this->verbose = static_cast<Verbosity>(Verbose.getValue());
  this->forceParallelization |= (ForceParallelization.getNumOccurrences() > 0);
  this->forceNoSCCPartition |= (ForceNoSCCPartition.getNumOccurrences() > 0);

  return false; 
}

bool Parallelizer::runOnModule (Module &M) {

  /*
   * Fetch the outputs of the passes we rely on.
   */
  auto& parallelizationFramework = getAnalysis<Parallelization>();
  auto heuristics = getAnalysis<HeuristicsPass>().getHeuristics();

  /*
   * Allocate the parallelization techniques.
   */
  DSWP dswp{M, this->forceParallelization, !this->forceNoSCCPartition, this->verbose};
  DOALL doall{M, this->verbose};

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
  auto loopsToParallelize = this->getLoopsToParallelize(M, parallelizationFramework);
  errs() << "Parallelizer:  There are " << loopsToParallelize.size() << " loops to parallelize\n";
  for (auto loop : loopsToParallelize){
    errs() << "Parallelizer:    Function \"" << loop->function->getName() << "\"\n";
    errs() << "Parallelizer:    Try to parallelize the loop \"" << *loop->header->getFirstNonPHI() << "\"\n";
    errs() << "\n";
  }

  /*
   * Parallelize the loops selected.
   */
  errs() << "Parallelizer:  Parallelize all " << loopsToParallelize.size() << " loops, one at a time\n";
  auto modified = false;
  for (auto loop : loopsToParallelize){

    /*
     * Parallelize the current loop with Parallelizer.
     */
    modified |= this->parallelizeLoop(loop, parallelizationFramework, dswp, doall, heuristics);

    /*
     * Free the memory.
     */
    delete loop;
  }

  return modified;
}

void Parallelizer::getAnalysisUsage (AnalysisUsage &AU) const {
  AU.addRequired<PDGAnalysis>();
  AU.addRequired<Parallelization>();
  AU.addRequired<HeuristicsPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();

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
