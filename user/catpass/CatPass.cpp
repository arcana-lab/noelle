#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "PDG.hpp"
#include "PDGAnalysis.hpp"

using namespace llvm;

namespace llvm {
  struct PDGUser : public ModulePass {
    static char ID;

    PDGUser() : ModulePass{ID} {}

    bool doInitialization (Module &M) override {
      return false;
    }

    bool runOnModule (Module &M) override {
      errs() << "PDGUser for " << M.getName() << "\n";

      /*
       * Fetch the PDG.
       */
      auto graph = getAnalysis<PDGAnalysis>().getPDG();

      /*
       * Transform the code.
       */
      for (auto &f : M){
        errs() << "Function: " << f.getName() << "\n";

        for (auto &bb : f){
          for (auto &i : bb){
              errs() << "Instruction: " << i << "\n";
          }
        }
      }

      return false;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<PDGAnalysis>();
      AU.setPreservesAll();

      return ;
    }
  };
}

// Next there is code to register your pass to "opt"
char llvm::PDGUser::ID = 0;
static RegisterPass<PDGUser> X("PDGUser", "Program Dependence Graph .dot file printer");

// Next there is code to register your pass to "clang"
static PDGUser * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new PDGUser());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new PDGUser());}});// ** for -O0
