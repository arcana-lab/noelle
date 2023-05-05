#include <algorithm>

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "noelle/core/Noelle.hpp"
#include "noelle/core/InductionVariableSCC.hpp"
#include "noelle/core/ReductionSCC.hpp"
#include "noelle/core/LoopIterationSCC.hpp"
#include "noelle/core/LoopCarriedUnknownSCC.hpp"
#include "noelle/core/MemoryClonableSCC.hpp"

using namespace llvm::noelle;

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
     * Fetch the metadata manager.
     */
    auto mm = noelle.getMetadataManager();

    /*
     * Check metadata attached to functions.
     */
    for (auto &F : M) {
      auto annotations = mm->getSourceCodeAnnotations(&F);
      if (annotations.size() == 0) {
        continue;
      }
      errs() << "Function \"" << F.getName()
             << "\" has the following annotations:\n";
      for (auto annotation : annotations) {
        errs() << "  \"" << annotation << "\"\n";
      }
    }

    /*
     * Check metadata attached to variables.
     */
    for (auto &F : M) {
      for (auto &inst : instructions(F)) {
        auto var = dyn_cast<AllocaInst>(&inst);
        if (var == nullptr) {
          continue;
        }
        auto annotations = mm->getSourceCodeAnnotations(var);
        if (annotations.size() == 0) {
          continue;
        }
        errs() << "Function \"" << F.getName() << "\"\n";
        errs() << "     has variable \"" << *var
               << "\" with the following annotations:\n";
        for (auto annotation : annotations) {
          errs() << "       \"" << annotation << "\"\n";
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
