#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/PostDominators.h"

#include "DGBase.hpp"
#include "DGGraphTraits.hpp"
#include "PDG.hpp"
#include "PDGAnalysis.hpp"
#include "SCCDAG.hpp"

#include "LoopDependenceInfo.hpp"

#include <set>
#include <queue>

#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace llvm;

namespace llvm {

  struct DGSimplify : public ModulePass {
  public:
    static char ID;

    DGSimplify()
      : ModulePass{ID}, fnsAffected{}, parentFns{}, childrenFns{},
        loopsToCheck{}, depthOrderedFns{}, preOrderedLoops{} {}

    ~DGSimplify() ;

    bool doInitialization (Module &M) override ;

    bool runOnModule (Module &M) override ;

    void getAnalysisUsage(AnalysisUsage &AU) const override ;

  private:

    /*
     * Inlining procedure
     */
    void getLoopsToInline (std::string filename) ;
    bool registerRemainingLoops (std::string filename) ;
    bool inlineCallsInMassiveSCCsOfLoops () ;
    bool inlineCallsInMassiveSCCs (Function *F, LoopDependenceInfo *LDI) ;

    void getFunctionsToInline (std::string filename) ;
    bool registerRemainingFunctions (std::string filename) ;
    bool inlineFnsOfLoopsToCGRoot () ;

    /*
     * Inline tracking
     */
    bool canInlineWithoutRecursiveLoop (Function *parentF, Function *childF) ;
    bool inlineFunctionCall (Function *F, Function *childF, CallInst *call) ;
    void adjustOrdersAfterInline (Function *F, Function *childF, CallInst *call, LoopSummary *nextLoop) ;
    LoopSummary *getNextPreorderLoopAfter (Function *F, CallInst *call) ;

    /*
     * Function and loop order tracking
     */
    void collectFnGraph (Function *main) ;
    void collectInDepthOrderFns (Function *main);
    void createPreOrderedLoopSummariesFor (Function *F) ;
    std::vector<Loop *> *collectPreOrderedLoopsFor (Function *F, LoopInfo &LI) ;
    void addFnPairInstance (Function *parentF, Function *childF, CallInst *call) ;
    void removeFnPairInstance (Function *parentF, Function *childF, CallInst *call) ;

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
    std::unordered_map<Function *, std::unordered_map<Function *, std::set<CallInst *>>> childrenFns;
    std::vector<Function *> depthOrderedFns;
    std::set<Function *> recursiveChainEntranceFns;
    std::unordered_map<Function *, int> fnOrders;
    std::unordered_map<Function *, std::vector<LoopSummary *> *> preOrderedLoops;

    /*
     * Tracking functions that had a CallInst of theirs inlined
     */
    std::set<Function *> fnsAffected;

    /*
     * Tracking the functions and loops to affect
     */
    std::unordered_map<Function *, std::set<LoopSummary *>> loopsToCheck;
    std::set<Function *> fnsToCheck;

    /*
     * Internal structures owned by the pass
     */
    std::set<LoopSummary *> loopSummaries;
  };
}

// Next there is code to register your pass to "opt"
char llvm::DGSimplify::ID = 0;
static RegisterPass<DGSimplify> X("DGSimplify", "Dependence Graph modifier");

// Next there is code to register your pass to "clang"
static DGSimplify * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new DGSimplify());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new DGSimplify());}});// ** for -O0

