/*
 * Copyright 2020  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "EnablersManager.hpp"
#include "HotProfiler.hpp"
#include "LoopDistribution.hpp"
#include <unordered_map>

using namespace llvm;

EnablersManager::EnablersManager()
  :
  ModulePass{ID}, 
  minHot{0}
  {

  return ;
}

bool EnablersManager::runOnModule (Module &M) {
  errs() << "EnablersManager: Start\n";

  /*
   * Fetch the outputs of the passes we rely on.
   */
  auto& parallelizationFramework = getAnalysis<Parallelization>();
  auto& profiles = getAnalysis<HotProfiler>().getHot();
  auto& loopDist = getAnalysis<LoopDistribution>();

  /*
   * Fetch all the loops we want to parallelize.
   */
  auto loopsToParallelize = parallelizationFramework.getModuleLoops(&M, this->minHot);
  errs() << "EnablersManager:  There are " << loopsToParallelize->size() << " loops to consider\n";
  for (auto loop : *loopsToParallelize){
    errs() << "EnablersManager:    Function: \"" << loop->function->getName() << "\"\n";
    errs() << "EnablersManager:    Loop: \"" << *loop->header->getFirstNonPHI() << "\"\n";
    if (profiles.isAvailable()){
      auto& profiles = getAnalysis<HotProfiler>().getHot();
      auto mInsts = profiles.getModuleInstructions();

      auto& LI = getAnalysis<LoopInfoWrapperPass>(*loop->function).getLoopInfo();
      auto loopInsts = profiles.getLoopInstructions(LI.getLoopFor(loop->header));
      auto hotness = ((double)loopInsts) / ((double)mInsts);
      hotness *= 100;
      errs() << "EnablersManager:      Hotness = " << hotness << " %\n"; 
    }
  }

  /*
   * Parallelize the loops selected.
   */
  errs() << "EnablersManager:  Try to improve all " << loopsToParallelize->size() << " loops, one at a time\n";
  auto modified = false;
  std::unordered_map<Function *, bool> modifiedFunctions;
  for (auto loop : *loopsToParallelize){

    /*
     * Fetch the function that contains the current loop.
     */
    auto f = loop->function;

    /*
     * Check if we have already modified the function.
     */
    if (modifiedFunctions[f]){
      errs() << "EnablersManager:   The current loop belongs to the function " << f->getName() << " , which has already been modified.\n" ;
      continue ;
    }

    /*
     * Improve the current loop.
     */
    modifiedFunctions[f] |= this->applyEnablers(loop, parallelizationFramework, loopDist);
    modified |= modifiedFunctions[f];
  }

  /*
   * Free the memory.
   */
  delete loopsToParallelize;

  errs() << "EnablersManager: Exit\n";
  return modified;
}
