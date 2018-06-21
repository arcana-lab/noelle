#include "DSWP.hpp"

using namespace llvm;

/*
 * Options of the DSWP pass.
 */
static cl::opt<bool> ForceParallelization("dswp-force", cl::ZeroOrMore, cl::Hidden, cl::desc("Force the parallelization"));
static cl::opt<bool> ForceNoSCCMerge("dswp-no-scc-merge", cl::ZeroOrMore, cl::Hidden, cl::desc("Force no SCC merging when parallelizing"));
static cl::opt<bool> Verbose("dswp-verbose", cl::ZeroOrMore, cl::Hidden, cl::desc("Enable verbose output"));

DSWP::DSWP()
  :
  ModulePass{ID}, 
  forceParallelization{false},
  forceNoSCCMerge{false},
  verbose{false}
  {

  return ;
}

bool DSWP::doInitialization (Module &M) {
  this->forceParallelization |= (ForceParallelization.getNumOccurrences() > 0);
  this->forceNoSCCMerge |= (ForceNoSCCMerge.getNumOccurrences() > 0);
  this->verbose |= (Verbose.getNumOccurrences() > 0);

  return false; 
}

bool DSWP::runOnModule (Module &M) {

  /*
   * Fetch the outputs of the passes we rely on.
   */
  auto& parallelizationFramework = getAnalysis<Parallelization>();

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

  /*
   * Parallelize the loops selected.
   */
  auto modified = false;
  for (auto loop : loopsToParallelize){

    /*
     * Parallelize the current loop with DSWP.
     */
    modified |= applyDSWP(loop, parallelizationFramework);

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
