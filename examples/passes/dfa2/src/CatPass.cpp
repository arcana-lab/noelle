#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <algorithm>

#include "Noelle.hpp"

using namespace llvm::noelle ;

namespace {

  struct CAT : public ModulePass {
    static char ID; 

    CAT() : ModulePass(ID) {}

    bool doInitialization (Module &M) override {
      return false;
    }

    bool runOnModule (Module &M) override {

      /*
       * Fetch NOELLE
       */
      auto& noelle = getAnalysis<Noelle>();
      errs() << "The program has " << noelle.numberOfProgramInstructions() << " instructions\n";

      /*
       * Fetch the entry point.
       */
      auto fm = noelle.getFunctionsManager();
      auto mainF = fm->getEntryFunction();

      /*
       * Data flow analyses
       */
      auto dfa = noelle.getDataFlowAnalyses();
      auto dfr = dfa.runReachableAnalysis(mainF);
      errs() << "Data flow reachable analysis\n";
      for (auto& inst : instructions(mainF)){
        errs() << " Next are the instructions reachable from " << inst << "\n";
        auto& outSet = dfr->OUT(&inst);
        for (auto reachInst : outSet){
          errs() << "   " << *reachInst << "\n";
        }
      }

      return false;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<Noelle>();
    }
  };
}

// Next there is code to register your pass to "opt"
char CAT::ID = 0;
static RegisterPass<CAT> X("CAT", "Simple user of the Noelle framework");

// Next there is code to register your pass to "clang"
static CAT * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new CAT());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new CAT()); }}); // ** for -O0
