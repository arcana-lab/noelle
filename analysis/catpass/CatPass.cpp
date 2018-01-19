#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

//#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Dominators.h"
//#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "llvm/Analysis/DomPrinter.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Analysis/DOTGraphTraitsPass.h"
#include "llvm/Analysis/PostDominators.h"

using namespace llvm;

namespace llvm {
  struct PDGAnalysis : public ModulePass {
    static char ID;

    PDGAnalysis() : ModulePass{ID} {
    }

    bool doInitialization (Module &M) override {
      errs() << "PDGAnalysis at \"doInitialization\"\n" ;
      return false;
    }

    bool runOnModule (Module &M) override {
      errs() << "PDGAnalysis at \"runOnModule\"\n" ;
      for (auto& F : M){
        DominatorTree domTree = DominatorTree(F);
        for (auto &B : F) {
          TerminatorInst *I = B.getTerminator();
          for (auto i = 0; i < I->getNumSuccessors(); ++i) {
            errs() << domTree.dominates(I, I->getSuccessor(i)) << "\n";
          }
        }
      }

      return false;
    }
  };
}

// Next there is code to register your pass to "opt"
char PDGAnalysis::ID = 0;
static RegisterPass<PDGAnalysis> X("PDGAnalysis", "Computing the Program Dependence Graph");

// Next there is code to register your pass to "clang"
static PDGAnalysis * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new PDGAnalysis());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new PDGAnalysis());}});// ** for -O0
