#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/CallGraph.h"

#include "DGBase.hpp"
#include "DGGraphTraits.hpp"
#include "PDG.hpp"
#include "PDGAnalysis.hpp"
#include "SCCDAG.hpp"

#include "llvm/ADT/GraphTraits.h"
#include "llvm/Analysis/DOTGraphTraitsPass.h"
#include "llvm/Analysis/DomPrinter.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/DOTGraphTraits.h"

#include <set>
#include <queue>

using namespace llvm;

namespace llvm {
  struct DGSimplify : public ModulePass {
  public:
    static char ID;

    DGSimplify() : ModulePass{ID} {}

    bool doInitialization (Module &M) override {
      errs() << "DGSimplify at \"doInitialization\"\n" ;
      return false;
    }

    bool runOnModule (Module &M) override {
      errs() << "DGSimplify at \"runOnModule\"\n";

      /*
       * Collect functions through call graph starting at function "main"
       */
      std::set<Function *> funcToCheck;
      collectAllFunctionsInCallGraph(M, funcToCheck);

      auto *graph = getAnalysis<PDGAnalysis>().getPDG();
      
      inlineCallsInFunctionsWithMassiveSCCs(graph, funcToCheck);

      return false;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<LoopInfoWrapperPass>();
      AU.addRequired<CallGraphWrapperPass>();
      AU.addRequired<PDGAnalysis>();
      AU.setPreservesAll();
      return ;
    }

  private:
    void collectAllFunctionsInCallGraph (Module &M, std::set<Function *> &funcSet)
    {
      auto &callGraph = getAnalysis<CallGraphWrapperPass>().getCallGraph();
      std::queue<Function *> funcToTraverse;
      funcToTraverse.push(M.getFunction("main"));
      while (!funcToTraverse.empty())
      {
        auto func = funcToTraverse.front();
        funcToTraverse.pop();
        if (funcSet.find(func) != funcSet.end()) continue;
        funcSet.insert(func);

        auto funcCGNode = callGraph[func];
        for (auto &callRecord : make_range(funcCGNode->begin(), funcCGNode->end()))
        {
          auto F = callRecord.second->getFunction();
          if (F->empty()) continue;
          funcToTraverse.push(F);
        }
      }
    }

    void inlineCallsInFunctionsWithMassiveSCCs (PDG *pdg, std::set<Function *> &funcSet) {
      errs() << "Collected the following functions:\n";
      for (auto F : funcSet) {
        errs() << "Encountered function: " << F->getName() << "\n";
      }
    }
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
