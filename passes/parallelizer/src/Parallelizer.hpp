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

#include "DSWPLoopDependenceInfo.hpp"
#include "PDG.hpp"
#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "PDGAnalysis.hpp"
#include "Parallelization.hpp"
#include "HeuristicsPass.hpp"
#include "DSWP.hpp"
#include "DOALL.hpp"

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

      /*
       * Methods
       */
      bool parallelizeLoop (DSWPLoopDependenceInfo *LDI, Parallelization &par, DSWP &dswp, DOALL &doall, Heuristics *h) ;
      std::vector<DSWPLoopDependenceInfo *> getLoopsToParallelize (Module &M, Parallelization &par);
      bool collectThreadPoolHelperFunctionsAndTypes (Module &M, Parallelization &par) ;
      void mergeSingleSyntacticSugarInstrs (DSWPLoopDependenceInfo *LDI);
      void mergeBranchesWithoutOutgoingEdges (DSWPLoopDependenceInfo *LDI);
      void mergeTrivialNodesInSCCDAG (DSWPLoopDependenceInfo *LDI);
      void collectSCCDAGAttrs (DSWPLoopDependenceInfo *LDI, Heuristics *h, ScalarEvolution &SE);
      void estimateCostAndExtentOfParallelismOfSCCs (DSWPLoopDependenceInfo *LDI, Heuristics *h);
      void collectRemovableSCCsByInductionVars (DSWPLoopDependenceInfo *LDI, ScalarEvolution &SE);
      void collectRemovableSCCsBySyntacticSugarInstrs (DSWPLoopDependenceInfo *LDI);
      void collectParallelizableSingleInstrNodes (DSWPLoopDependenceInfo *LDI);

      /*
       * Debug utilities
       */
      void printSCCs (SCCDAG *sccSubgraph);
      void printLoop (Loop *loop);
  };

}
