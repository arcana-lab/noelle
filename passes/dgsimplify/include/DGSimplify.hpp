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

#include <iostream>
#include <fstream>

using namespace llvm;

namespace llvm {
  struct DGSimplify : public ModulePass {
  public:
    static char ID;

    DGSimplify()
      : ModulePass{ID}, fnsAffected{}, orderedFns{},
        preOrderedCalls{}, preOrderedLoops{} {}

    ~DGSimplify() ;

    bool doInitialization (Module &M) override ;

    bool runOnModule (Module &M) override ;

    void getAnalysisUsage(AnalysisUsage &AU) const override ;

  private:
    void collectAllFunctionsInCallGraph (Module &M, std::set<Function *> &funcSet) ;

    bool inlineCallsInFunctionsWithMassiveSCCs (std::set<Function *> &funcSet) ;

    bool checkToInlineCallInFunction (PDG *fdg, Function &F) ;

    /*
     * Function and loop order tracking
     */
    void collectInDepthOrderFnsCallsAndLoops (Function *main);
    void collectPreOrderedLoopsFor (Function *F) ;

    /*
     * NOTE(angelo): Naive assumption is that for function count n,
     *  there are O(n) calls, and O(1) loops per function.
     * To aim for O(1) lookup, ordered functions are mapped over,
     *  with values (ordered indices) updated manually, whereas
     *  loops are simply vectorized.
     */
    std::unordered_map<CallInst *, int> preOrderedCalls;
    std::unordered_map<Function *, std::vector<Loop *> *> preOrderedLoops;

    /*
     * Inline tracking
     */
    bool inlineFunctionCall (Function *F, CallInst *call) ;

    std::set<Function *> fnsAffected;
    std::vector<Function *> depthOrderedFns;
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

