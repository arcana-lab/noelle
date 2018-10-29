#include "Parallelizer.hpp"

using namespace llvm;

std::vector<DSWPLoopDependenceInfo *> Parallelizer::getLoopsToParallelize (Module &M, Parallelization &par){
  std::vector<DSWPLoopDependenceInfo *> loopsToParallelize;

  /*
   * Define the allocator of loop structures.
   */
  auto allocatorOfLoopStructures = [] (Function *f, PDG *fG, Loop *l, LoopInfo &li, PostDominatorTree &pdt) -> LoopDependenceInfo * {
    auto ldi = new DSWPLoopDependenceInfo(f, fG, l, li, pdt);
    return ldi;
  };

  /*
   * Collect all loops included in the module.
   */
  auto allLoops = par.getModuleLoops(&M, allocatorOfLoopStructures);

  /*
   * Consider all loops to parallelize.
   */
  for (auto loop : *allLoops){
    auto dswpLoop = (DSWPLoopDependenceInfo *)(loop);
    loopsToParallelize.push_back(dswpLoop);
  }

  /*
   * Free the memory.
   */
  delete allLoops;

  return loopsToParallelize;
}

