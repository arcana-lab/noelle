/*
 * Copyright 2016 - 2021  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DerivedUser.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/IRBuilder.h"

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/LoopDependenceInfo.hpp"
#include "noelle/core/PDG.hpp"
#include "noelle/core/SCC.hpp"
#include "noelle/core/SCCDAG.hpp"
#include "noelle/core/Noelle.hpp"
#include "noelle/core/MetadataManager.hpp"
#include "HeuristicsPass.hpp"
#include "DSWP.hpp"
#include "DOALL.hpp"
#include "HELIX.hpp"

namespace llvm::noelle {

  class Parallelizer : public ModulePass {
    public:

      Parallelizer ();

      bool doInitialization (Module &M) override ;

      bool runOnModule (Module &M) override ;

      void getAnalysisUsage (AnalysisUsage &AU) const override ;

      /*
       * Class fields
       */
      static char ID;

    private:

      /*
       * Fields
       */
      bool forceParallelization;
      bool forceNoSCCPartition;

      /*
       * Methods
       */
      bool parallelizeLoop (
        LoopDependenceInfo *LDI,
        Noelle &par,
        Heuristics *h
      );

      std::vector<LoopDependenceInfo *> getLoopsToParallelize (Module &M, Noelle &par) ;

      bool collectThreadPoolHelperFunctionsAndTypes (Module &M, Noelle &par) ;

      /*
       * Debug utilities
       */
      void printLoop (Loop *loop);
  };

}
