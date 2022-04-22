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
      Heuristics *h
      ){
    auto prefix = "Parallelizer: parallelizerLoop: " ;

    /*
     * Assertions.
     */
    assert(LDI != nullptr);
    assert(h != nullptr);

    /*
     * Allocate the parallelization techniques.
     */
    DSWP dswp{
      par,
      this->forceParallelization,
      !this->forceNoSCCPartition
    };
    DOALL doall{
      par
    };
    HELIX helix{
      par,
      this->forceParallelization
    };

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
    assert(par.verifyCode());

    /*
     * Print
     */
    if (verbose != Verbosity::Disabled) {
      errs() << prefix << "Start\n";
      errs() << prefix << "  Function = \"" << loopFunction->getName() << "\"\n";
      errs() << prefix << "  Loop " << LDI->getID() << " = \"" << *loopHeader->getFirstNonPHI() << "\"\n";
      errs() << prefix << "  Nesting level = " << loopStructure->getNestingLevel() << "\n";
      errs() << prefix << "  Number of threads to extract = " << LDI->getLoopTransformationsManager()->getMaximumNumberOfCores() << "\n";
    }

    /*
     * Parallelize the loop.
     */
    auto codeModified = false;
    auto ltm = LDI->getLoopTransformationsManager();
    ParallelizationTechnique *usedTechnique = nullptr;
    if (  true
        && par.isTransformationEnabled(DOALL_ID)
        && ltm->isTransformationEnabled(DOALL_ID)
        && doall.canBeAppliedToLoop(LDI, h)
       ){

      /*
       * Apply DOALL.
       */
      codeModified = doall.apply(LDI, h);
      usedTechnique = &doall;

    } else if ( true
        && par.isTransformationEnabled(HELIX_ID)
        && ltm->isTransformationEnabled(HELIX_ID)
        && helix.canBeAppliedToLoop(LDI, h)   
        ){

      /*
       * Apply HELIX
       */
      codeModified = helix.apply(LDI, h);

      auto function = helix.getTaskFunction();
      auto &LI = getAnalysis<LoopInfoWrapperPass>(*function).getLoopInfo();
      auto& PDT = getAnalysis<PostDominatorTreeWrapperPass>(*function).getPostDomTree();
      auto& SE = getAnalysis<ScalarEvolutionWrapperPass>(*function).getSE();

      if (par.getVerbosity() >= Verbosity::Maximal) {
        errs() << "HELIX:  Constructing task dependence graph\n";
      }

      auto taskFunctionDG = helix.constructTaskInternalDependenceGraphFromOriginalLoopDG(LDI, PDT);

      if (par.getVerbosity() >= Verbosity::Maximal) {
        errs() << "HELIX:  Constructing task loop dependence info\n";
      }

      auto DS = par.getDominators(function);
      auto l = LI.getLoopsInPreorder()[0];
      auto newLoops = par.getLoopStructures(function, 0);
      auto newForest = par.organizeLoopsInTheirNestingForest(*newLoops);
      auto newLoopNode = newForest->getInnermostLoopThatContains(l->getHeader());
      assert(newLoopNode != nullptr);
      auto newLDI = new LoopDependenceInfo(taskFunctionDG, newLoopNode, l, *DS, SE, par.getCompilationOptionsManager()->getMaximumNumberOfCores(), par.canFloatsBeConsideredRealNumbers());
      newLDI->copyParallelizationOptionsFrom(LDI);

      codeModified = helix.apply(newLDI, h);
      usedTechnique = &helix;

    } else if ( true
        && par.isTransformationEnabled(DSWP_ID)
        && ltm->isTransformationEnabled(DSWP_ID)
        && dswp.canBeAppliedToLoop(LDI, h)
        ) {

      /*
       * Apply DSWP.
       */
      codeModified = dswp.apply(LDI, h);
      usedTechnique = &dswp;
    }

    /*
     * Check if the loop has been parallelized.
     */
    if (!codeModified){
      errs() << prefix << "  The loop has not been parallelized\n";
      errs() << prefix << "Exit\n";
      return false;
    }

    /*
     * Fetch the environment array where the exit block ID has been stored.
     */
    assert(usedTechnique != nullptr);
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
      errs() << prefix << "  Link the parallelize loop\n";
    }
    auto exitIndex = ConstantInt::get(par.int64, LDI->getEnvironment()->indexOfExitBlockTaken());
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
    assert(par.verifyCode());
    // if (verbose >= Verbosity::Maximal) {
    //   loopFunction->print(errs() << "Final printout:\n"); errs() << "\n";
    // }
    if (verbose != Verbosity::Disabled) {
      errs() << prefix << "  The loop has been parallelized\n";
      errs() << prefix << "Exit\n";
    }

    return true;
  }
}
