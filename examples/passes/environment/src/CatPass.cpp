#include <algorithm>

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

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
     * fetch the loops with all their abstractions
     * (e.g., loop dependence graph, sccdag)
     */
    auto forest = noelle.getLoopNestingForest();

    /*
     * Print loop induction variables and invariant.
     */
    for (auto tree : forest->getTrees()) {
      for (auto loopNode : tree->getNodes()) {
        auto LS = loopNode->getLoop();

        /*
         * Print the first instruction the loop executes.
         */
        auto entryInst = LS->getEntryInstruction();
        errs() << "Loop " << *entryInst << "\n";

        /*
         * Print some information about the loop.
         */
        errs() << " Function = " << LS->getFunction()->getName() << "\n";
        errs() << " Nesting level = " << LS->getNestingLevel() << "\n";
        errs() << " This loop has " << loopNode->getNumberOfSubLoops()
               << " sub-loops (including sub-loops of sub-loops)\n";

        /*
         * Fetch the LoopDependenceInfo
         */
        auto loop = noelle.getLoopContent(LS);

        /*
         * Fetch the loop environment
         */
        auto loopEnv = loop->getEnvironment();

        /*
         * Print the number of elements that compose the environment.
         */
        errs() << " Environment of the loop is composed by " << loopEnv->size()
               << " elements\n";

        /*
         * Iterate over live-in values
         */
        errs() << "   Live-In values:\n";
        for (auto liveInIndex : loopEnv->getEnvIDsOfLiveInVars()) {
          auto p = loopEnv->getProducer(liveInIndex);
          errs() << "     Index " << liveInIndex << ": " << *p << "\n";
        }

        /*
         * Iterate over live-out values
         */
        errs() << "   Live-Out values:\n";
        for (auto liveOutIndex : loopEnv->getEnvIDsOfLiveOutVars()) {
          auto p = loopEnv->getProducer(liveOutIndex);
          errs() << "     Index " << liveOutIndex << ": " << *p << "\n";
        }

        /*
         * Iterate over producers of the elements of the environment.
         */
        errs()
            << " Producers of the elements in the environment of the loop:\n";
        for (auto liveInOrOutValue : loopEnv->getProducers()) {
          errs() << "   " << *liveInOrOutValue << "\n";
        }

        /*
         * Iterate over consumers of live-out values
         */
        errs() << "   Consumers of live-Out values:\n";
        for (auto liveOutIndex : loopEnv->getEnvIDsOfLiveOutVars()) {
          auto p = loopEnv->getProducer(liveOutIndex);
          errs() << "     Index " << liveOutIndex << ": " << *p << "\n";
          for (auto c : loopEnv->consumersOf(p)) {
            errs() << "       Consumer = " << *c << "\n";
          }
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
