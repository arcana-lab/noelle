#include "DSWP.hpp"

using namespace llvm;

/*
 * Options of the DSWP pass.
 */
static cl::opt<bool> ForceParallelization("dswp-force", cl::ZeroOrMore, cl::Hidden, cl::desc("Force the parallelization"));
static cl::opt<bool> ForceNoSCCPartition("dswp-no-scc-merge", cl::ZeroOrMore, cl::Hidden, cl::desc("Force no SCC merging when parallelizing"));
static cl::opt<int> Verbose("dswp-verbose", cl::ZeroOrMore, cl::Hidden, cl::desc("Verbose output (0: disabled, 1: minimal, 2: stage outline 3: maximal)"));

DSWP::DSWP()
  :
  ModulePass{ID}, 
  forceParallelization{false},
  forceNoSCCPartition{false},
  verbose{Verbosity::Disabled}
  {

  return ;
}

bool DSWP::doInitialization (Module &M) {
  this->forceParallelization |= (ForceParallelization.getNumOccurrences() > 0);
  this->forceNoSCCPartition |= (ForceNoSCCPartition.getNumOccurrences() > 0);
  this->verbose = static_cast<Verbosity>(Verbose.getValue());

  return false; 
}

bool DSWP::runOnModule (Module &M) {

  /*
   * Fetch the outputs of the passes we rely on.
   */
  auto& parallelizationFramework = getAnalysis<Parallelization>();
  auto heuristics = getAnalysis<HeuristicsPass>().getHeuristics();

  /*
   * Collect some information.
   */
  errs() << "DSWP: Analyzing the module " << M.getName() << "\n";
  if (!collectThreadPoolHelperFunctionsAndTypes(M, parallelizationFramework)) {
    errs() << "DSWP utils not included!\n";
    return false;
  }

  /*
   * Fetch all the loops we want to parallelize.
   */
  auto loopsToParallelize = this->getLoopsToParallelize(M, parallelizationFramework);
  errs() << "DSWP:  There are " << loopsToParallelize.size() << " loops to parallelize\n";
  for (auto loop : loopsToParallelize){
    errs() << "DSWP:    Function \"" << loop->function->getName() << "\"\n";
    errs() << "DSWP:    Try to parallelize the loop \"" << *loop->header->getFirstNonPHI() << "\"\n";
    errs() << "\n";
  }

  /*
   * Parallelize the loops selected.
   */
  errs() << "DSWP:  Parallelize all " << loopsToParallelize.size() << " loops, one at a time\n";
  auto modified = false;
  for (auto loop : loopsToParallelize){

    /*
     * Parallelize the current loop with DSWP.
     */
    modified |= this->parallelizeLoop(loop, parallelizationFramework, heuristics);

    /*
     * Free the memory.
     */
    delete loop;
  }

  return modified;
}

void DSWP::getAnalysisUsage (AnalysisUsage &AU) const {
  AU.addRequired<PDGAnalysis>();
  AU.addRequired<Parallelization>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<HeuristicsPass>();

  return ;
}

// Next there is code to register your pass to "opt"
char llvm::DSWP::ID = 0;
static RegisterPass<DSWP> X("DSWP", "DSWP parallelization");

// Next there is code to register your pass to "clang"
static DSWP * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new DSWP());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new DSWP());}});// ** for -O0
