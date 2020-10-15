/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"

#include "DGBase.hpp"
#include "DGGraphTraits.hpp"
#include "PDG.hpp"
#include "PDGAnalysis.hpp"
#include "SCCDAG.hpp"
#include "LoopDependenceInfo.hpp"
#include "Noelle.hpp"
#include <fstream>

using namespace llvm;
using namespace llvm::noelle;

namespace llvm::noelle {

  class Inliner : public ModulePass {
    public:
      static char ID;

      Inliner() ;

      ~Inliner() ;

      bool doInitialization (Module &M) override ;

      bool runOnModule (Module &M) override ;

      void getAnalysisUsage(AnalysisUsage &AU) const override ;

    private:

      /*
       * Inlining procedure
       */
      void getLoopsToInline (Noelle &noelle, Hot *profiles) ;
      bool registerRemainingLoops (std::string filename) ;
      bool inlineCallsInvolvedInLoopCarriedDataDependences (Noelle &noelle, noelle::CallGraph *pcg) ;
      bool inlineCallsInvolvedInLoopCarriedDataDependencesWithinLoop (Function *F, LoopDependenceInfo *LDI, noelle::CallGraph *pcg) ;

      void getFunctionsToInline (std::string filename) ;
      bool registerRemainingFunctions (std::string filename) ;
      bool inlineFnsOfLoopsToCGRoot () ;

      /*
       * Inline tracking
       */
      bool canInlineWithoutRecursiveLoop (Function *parentF, Function *childF) ;
      bool inlineFunctionCall (Function *F, Function *childF, CallInst *call) ;
      int getNextPreorderLoopAfter (Function *F, CallInst *call) ;
      void adjustLoopOrdersAfterInline (Function *F, Function *childF, int nextLoop) ;
      void adjustFnGraphAfterInline (Function *F, Function *childF, int callInd) ;

      /*
       * Function and loop order tracking
       */
      void collectFnGraph (Function *main) ;
      void collectFnCallsAndCalled (llvm::CallGraph &CG, Function *parentF) ;
      void collectInDepthOrderFns (Function *main);
      void createPreOrderedLoopSummariesFor (Function *F) ;
      std::vector<Loop *> *collectPreOrderedLoopsFor (Function *F, LoopInfo &LI) ;
      void sortInDepthOrderFns (std::vector<Function *> &inOrder);

      /*
       * Debugging
       */
      void printFnCallGraph ();
      void printFnOrder ();
      void printFnLoopOrder (Function *F);
      void printLoopsToCheck ();
      void printFnsToCheck ();

      /*
       * Determining and maintaining depth ordering of functions and their loops
       */
      std::unordered_map<Function *, std::set<Function *>> parentFns;
      std::unordered_map<Function *, std::vector<Function *>> childrenFns;
      std::vector<Function *> depthOrderedFns;
      std::set<Function *> recursiveChainEntranceFns;
      std::unordered_map<Function *, int> fnOrders;
      std::unordered_map<Function *, std::vector<LoopStructure *> *> preOrderedLoops;

      /*
       * Tracking functions that had a CallInst of theirs inlined
       * Initially valid call graph information accurate on unaffected functions
       * Updated called function order as inlines occur
       */
      std::set<Function *> fnsAffected;
      std::unordered_map<Function *, std::vector<CallInst *>> orderedCalls;
      std::unordered_map<Function *, std::vector<Function *>> orderedCalled;

      /*
       * Tracking the functions and loops to affect
       */
      std::unordered_map<Function *, std::vector<LoopStructure *>> loopsToCheck;
      std::set<Function *> fnsToCheck;

      /*
       * Internal structures owned by the pass
       */
      std::set<LoopStructure *> loopSummaries;
      Verbosity verbose;
    };

}
