/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Parallelizer.hpp"

namespace llvm::noelle {
  
  bool Parallelizer::parallelizeLoop (
    LoopDependenceInfo *LDI, 
    Noelle &par, 
    DSWP &dswp, 
    DOALL &doall, 
    HELIX &helix, 
    Heuristics *h
    ){

    /*
    * Assertions.
    */
    assert(LDI != nullptr);
    assert(h != nullptr);

    /*
    * Fetch the verbosity level.
    */
    auto verbose = par.getVerbosity();

    /*
    * Fetch the loop headers.
    */
    auto loopStructure = LDI->getLoopStructure();
    auto loopHeader = loopStructure->getHeader();
    auto loopPreHeader = loopStructure->getPreHeader();

    /*
    * Fetch the loop function.
    */
    auto loopFunction = loopStructure->getFunction();

    /*
    * Print
    */
    if (verbose != Verbosity::Disabled) {
      errs() << "Parallelizer: Start\n";
      errs() << "Parallelizer:  Function = \"" << loopFunction->getName() << "\"\n";
      errs() << "Parallelizer:  Loop " << LDI->getID() << " = \"" << *loopHeader->getFirstNonPHI() << "\"\n";
      errs() << "Parallelizer:  Nesting level = " << loopStructure->getNestingLevel() << "\n";
    }

    /*
    * Parallelize the loop.
    */
    auto codeModified = false;
    ParallelizationTechnique *usedTechnique = nullptr;
    if (  true
          && par.isTransformationEnabled(DOALL_ID)
          && LDI->isTransformationEnabled(DOALL_ID)
          && doall.canBeAppliedToLoop(LDI, par, h)
      ){

      /*
      * Apply DOALL.
      */
      doall.reset();
      codeModified = doall.apply(LDI, par, h);
      usedTechnique = &doall;

    } else if ( true
                && par.isTransformationEnabled(HELIX_ID)
                && LDI->isTransformationEnabled(HELIX_ID)
                && helix.canBeAppliedToLoop(LDI, par, h)   
      ){

      /*
      * Apply HELIX
      */
      helix.reset();
      codeModified = helix.apply(LDI, par, h);

      auto function = helix.getTaskFunction();
      auto &LI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();
      auto& DT = getAnalysis<DominatorTreeWrapperPass>(*function).getDomTree();
      auto& PDT = getAnalysis<PostDominatorTreeWrapperPass>(*function).getPostDomTree();
      auto& SE = getAnalysis<ScalarEvolutionWrapperPass>(*function).getSE();

      if (par.getVerbosity() >= Verbosity::Maximal) {
        errs() << "HELIX:  Constructing task dependence graph\n";
      }

      auto taskFunctionDG = helix.constructTaskInternalDependenceGraphFromOriginalLoopDG(LDI, PDT);

      if (par.getVerbosity() >= Verbosity::Maximal) {
        errs() << "HELIX:  Constructing task loop dependence info\n";
      }

      DominatorSummary DS{DT, PDT};
      auto l = LI.getLoopsInPreorder()[0];
      auto newLDI = new LoopDependenceInfo(taskFunctionDG, l, DS, SE, par.getMaximumNumberOfCores());
      newLDI->copyParallelizationOptionsFrom(LDI);

      codeModified = helix.apply(newLDI, par, h);
      usedTechnique = &helix;

    } else if ( true
                && par.isTransformationEnabled(DSWP_ID)
                && LDI->isTransformationEnabled(DSWP_ID)
                && dswp.canBeAppliedToLoop(LDI, par, h)
      ) {

      /*
      * Apply DSWP.
      */
      dswp.reset();
      codeModified = dswp.apply(LDI, par, h);
      usedTechnique = &dswp;
    }

    /*
    * Check if the loop has been parallelized.
    */
    if (!codeModified){
      errs() << "Parallelizer: Exit (no code modified)\n";
      return false;
    }

    /*
    * Fetch the environment array where the exit block ID has been stored.
    */
    auto envArray = usedTechnique->getEnvArray();
    assert(envArray != nullptr);

    /*
    * Fetch entry and exit point executed by the parallelized loop.
    */
    auto entryPoint = usedTechnique->getParLoopEntryPoint();
    auto exitPoint = usedTechnique->getParLoopExitPoint();
    assert(entryPoint != nullptr && exitPoint != nullptr);

    /*
    * The loop has been parallelized.
    *
    * Link the parallelized loop within the original function that includes the sequential loop.
    */
    if (verbose != Verbosity::Disabled) {
      errs() << "Parallelizer:  Link the parallelize loop\n";
    }
    auto exitIndex = cast<Value>(ConstantInt::get(par.int64, LDI->environment->indexOfExitBlock()));
    auto loopExitBlocks = loopStructure->getLoopExitBasicBlocks();
    par.linkTransformedLoopToOriginalFunction(
      loopFunction->getParent(),
      loopPreHeader,
      entryPoint,
      exitPoint, 
      envArray,
      exitIndex,
      loopExitBlocks
    );
    // if (verbose >= Verbosity::Maximal) {
    //   loopFunction->print(errs() << "Final printout:\n"); errs() << "\n";
    // }

    /*
    * Return
    */
    if (verbose != Verbosity::Disabled) {
      errs() << "Parallelizer: Exit\n";
    }
    return true;
  }
}