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
#include "SCCDAGAttrs.hpp"

#include <set>
#include <queue>

#include <iostream>
#include <fstream>

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
      bool inlined = inlineCallsInFunctionsWithMassiveSCCs(funcToCheck);

      return inlined;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<LoopInfoWrapperPass>();
      AU.addRequired<CallGraphWrapperPass>();
      AU.addRequired<PDGAnalysis>();
      AU.addRequired<PostDominatorTreeWrapperPass>();
      AU.addRequired<ScalarEvolutionWrapperPass>();
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
          if (!F || F->empty()) continue;
          funcToTraverse.push(F);
        }
      }
    }

    bool inlineCallsInFunctionsWithMassiveSCCs (std::set<Function *> &funcSet) {
      auto &PDGA = getAnalysis<PDGAnalysis>();
      bool inlined = false;
      for (auto F : funcSet) {
        errs() << "Encountered function: " << F->getName() << "\n";

        auto fdg = PDGA.getFunctionPDG(*F);
        bool inlinedCall = checkToInlineCallInFunction(fdg, *F);
        delete fdg;

        inlined |= inlinedCall;
      }
      
      ofstream inlineInfo("dgsimplify_inlineinfo.txt");
      inlineInfo << (inlined ? "1" : "0");
      inlineInfo.close();

      if (inlined) {
        errs() << "Inlined\n";
      }
      return inlined;
    }

    /*
     * GOAL: Go through loops in function
     * If there is only one non-clonable/reducable SCC,
     * try inlining the function call in that SCC with the
     * most memory edges to other internal/external values
     */
    bool checkToInlineCallInFunction (PDG *fdg, Function &F) {
      auto& PDT = getAnalysis<PostDominatorTreeWrapperPass>(F).getPostDomTree();
      auto& LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
      auto& SE = getAnalysis<ScalarEvolutionWrapperPass>(F).getSE();
      for (auto loop : LI.getLoopsInPreorder()) {
        loop->print(errs() << "Loop:\n"); errs() << "\n";
        auto LDI = new LoopDependenceInfo(&F, fdg, loop, LI, PDT);
        auto &attrs = LDI->sccdagAttrs;
        attrs.populate(LDI->loopSCCDAG, LDI->liSummary, SE);

        int64_t maxMemEdges = 0;
        CallInst *inlineCall = nullptr;

        std::set<SCC *> sccsToCheck;
        for (auto sccNode : LDI->loopSCCDAG->getNodes()) {
          auto scc = sccNode->getT();

          scc->printMinimal(errs() << "SCC:\n") << "\n";
          if (attrs.executesCommutatively(scc)
              || attrs.executesIndependently(scc)
              || attrs.canBeCloned(scc)) {
            continue;
          }

          sccsToCheck.insert(scc);
        }

        /*
         * NOTE: if there are more than two non-trivial SCCs, then
         * there is less incentive to continue trying to inline.
         * Why 2? Because 2 is always a simple non-trivial number
         * to start a heuristic at.
         */
        if (sccsToCheck.size() > 2) continue;

        for (auto scc : sccsToCheck) {
          for (auto valNode : scc->getNodes()) {
            auto val = valNode->getT();
            if (auto call = dyn_cast<CallInst>(val)) {
              auto callF = call->getCalledFunction();
              if (!callF || callF->empty()) continue;

              auto memEdgeCount = 0;
              for (auto edge : valNode->getAllConnectedEdges()) {
                if (edge->isMemoryDependence()) memEdgeCount++;
              }

              if (memEdgeCount > maxMemEdges) {
                maxMemEdges = memEdgeCount;
                inlineCall = call;
              }
            }
          }
        }

        delete LDI;

        if (inlineCall) {
          InlineFunctionInfo IFI;
          inlineCall->print(errs() << "Inlining: "); errs() << "\n";
          if (InlineFunction(inlineCall, IFI)) {
            return true;
          }
        }
      }

      return false;
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
