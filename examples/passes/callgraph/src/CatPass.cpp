#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <algorithm>

#include "arcana/noelle/core/Noelle.hpp"

using namespace arcana::noelle;

namespace {

struct CAT : public ModulePass {
  static char ID;

  CAT() : ModulePass(ID) {}

  bool doInitialization(Module &M) override {
    return false;
  }

  bool runOnModule(Module &M) override {

    /*
     * Fetch NOELLE
     */
    auto &noelle = getAnalysis<Noelle>();

    /*
     * Fetch the entry point.
     */
    auto fm = noelle.getFunctionsManager();
    auto mainF = fm->getEntryFunction();

    /*
     * Call graph.
     */
    auto pcf = fm->getProgramCallGraph();
    for (auto node : pcf->getFunctionNodes(true)) {

      /*
       * Fetch the next program's function.
       */
      auto f = node->getFunction();

      /*
       * Fetch the outgoing edges.
       */
      auto outEdges = pcf->getOutgoingEdges(node);
      if (outEdges.size() == 0) {
        errs() << " The function \"" << f->getName() << "\" has no calls\n";
        continue;
      }

      /*
       * Print the outgoing edges.
       */
      errs() << " The function \"" << f->getName() << "\"";
      errs() << " invokes the following functions:\n";
      for (auto callEdge : outEdges) {
        auto calleerNode = callEdge->getCaller();
        auto calleeNode = callEdge->getCallee();
        auto calleeF = calleeNode->getFunction();

        errs() << "   [";
        if (callEdge->isAMustCall()) {
          errs() << "must";
        } else {
          errs() << "may";
        }
        errs() << "] \"" << calleeF->getName() << "\"\n";

        /*
         * Print the sub-edges.
         */
        for (auto subEdge : callEdge->getSubEdges()) {
          auto callerSubEdge = subEdge->getCaller();
          errs() << "     [";
          if (subEdge->isAMustCall()) {
            errs() << "must";
          } else {
            errs() << "may";
          }
          errs() << "] " << *callerSubEdge->getInstruction() << "\n";
        }
      }
    }

    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<Noelle>();
  }
};
} // namespace

// Next there is code to register your pass to "opt"
char CAT::ID = 0;
static RegisterPass<CAT> X("CAT", "Simple user of the Noelle framework");

// Next there is code to register your pass to "clang"
static CAT *_PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
                                        [](const PassManagerBuilder &,
                                           legacy::PassManagerBase &PM) {
                                          if (!_PassMaker) {
                                            PM.add(_PassMaker = new CAT());
                                          }
                                        }); // ** for -Ox
static RegisterStandardPasses _RegPass2(
    PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
      if (!_PassMaker) {
        PM.add(_PassMaker = new CAT());
      }
    }); // ** for -O0
