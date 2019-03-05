/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
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

#include "LoopDependenceInfo.hpp"
#include "PDG.hpp"
#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "PDGAnalysis.hpp"
#include "Parallelization.hpp"
#include "HeuristicsPass.hpp"
#include "DSWP.hpp"
#include "DOALL.hpp"
#include "HELIX.hpp"
#include "Techniques.hpp"

#include <unordered_map>
#include <set>
#include <queue>
#include <deque>

namespace llvm {

  struct Parallelizer : public ModulePass {
    public:

      /*
       * Class fields
       */
      static char ID;

      /*
       * Object fields
       */
      Function *printReachedI, *printPushedP, *printPulledP;

      /*
       * Methods
       */
      Parallelizer();
      bool doInitialization (Module &M) override ;
      bool runOnModule (Module &M) override ;
      void getAnalysisUsage (AnalysisUsage &AU) const override ;

    private:

      /*
       * Fields
       */
      bool forceParallelization;
      bool forceNoSCCPartition;
      Verbosity verbose;
      double minHot;
      Techniques techniques;

      /*
       * Methods
       */
      bool parallelizeLoop (
        LoopDependenceInfo *LDI,
        Parallelization &par,
        DSWP &dswp,
        DOALL &doall,
        HELIX &helix,
        Heuristics *h
      );
      std::vector<LoopDependenceInfo *> getLoopsToParallelize (Module &M, Parallelization &par);
      bool collectThreadPoolHelperFunctionsAndTypes (Module &M, Parallelization &par) ;

      /*
       * Debug utilities
       */
      void printLoop (Loop *loop);
  };

}
