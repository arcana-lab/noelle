#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <algorithm>

#include "noelle/core/Noelle.hpp"

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
     * Fetch the PDG
     */
    auto PDG = noelle.getProgramDependenceGraph();

    /*
     * Fetch the FDG of "main"
     */
    auto fm = noelle.getFunctionsManager();
    auto mainF = fm->getEntryFunction();
    auto FDG = PDG->createFunctionSubgraph(*mainF);

    /*
     * Iterate over the dependences
     */
    auto iterF = [](Value *src, DGEdge<Value, Value> *dep) -> bool {
      errs() << "   " << *src << " ";
      if (dep->isMustDependence()) {
        errs() << " MUST ";
      } else {
        errs() << " MAY ";
      }
      if (isa<ControlDependence<Value, Value>>(dep)) {
        errs() << " CONTROL ";
      }
      if (isa<DataDependence<Value, Value>>(dep)) {
        errs() << " DATA ";
        if (dep->isRAWDependence()) {
          errs() << " RAW ";
        }
        if (dep->isWARDependence()) {
          errs() << " WAR ";
        }
        if (dep->isWAWDependence()) {
          errs() << " WAW ";
        }
      }
      if (isa<MemoryDependence<Value, Value>>(dep)) {
        errs() << " MEMORY ";
      }

      errs() << "\n";
      return false;
    };

    for (auto &inst : instructions(mainF)) {
      errs() << "Instruction \"" << inst << "\" depends on\n";
      FDG->iterateOverDependencesTo(&inst, true, true, true, iterF);
    }

    for (auto &inst : instructions(mainF)) {
      errs() << "Instruction \"" << inst << "\" outgoing dependences\n";
      FDG->iterateOverDependencesFrom(&inst, true, true, true, iterF);
    }

    for (auto &inst : instructions(mainF)) {
      for (auto &inst2 : instructions(mainF)) {
        for (auto dep : FDG->getDependences(&inst, &inst2)) {
          errs() << "A " << sizeof(*dep) << "\n";
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
