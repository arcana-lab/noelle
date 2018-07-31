#include "DSWP.hpp"

using namespace llvm;

std::vector<DSWPLoopDependenceInfo *> DSWP::getLoopsToParallelize (Module &M, Parallelization &par){
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
  std::set<Function *> functionsSeen;
  for (auto loop : *allLoops){
    auto function = loop->function;
    if (functionsSeen.find(function) != functionsSeen.end()){

      /*
       * Free the memory.
       */
      // delete loop;
      // continue ;
    }
    functionsSeen.insert(function);

    auto dswpLoop = (DSWPLoopDependenceInfo *)(loop);
    loopsToParallelize.push_back(dswpLoop);
  }

  /*
   * Free the memory.
   */
  delete allLoops;

  return loopsToParallelize;
}

