#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <algorithm>

#include "arcana/noelle/core/NoellePass.hpp"

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
    auto &noelle = getAnalysis<NoellePass>().getNoelle();
    errs() << "The program has " << noelle.numberOfProgramInstructions()
           << " instructions\n";

    /*
     * Fetch the entry point.
     */
    auto fm = noelle.getFunctionsManager();
    auto mainF = fm->getEntryFunction();

    /*
     * Fetch the data flow engine.
     */
    auto dfe = noelle.getDataFlowEngine();

    /*
     * Define the data flow equations
     */
    auto computeGEN = [](Instruction *i, DataFlowResult *df) {
      if (!isa<LoadInst>(i)) {
        return;
      }
      auto &gen = df->GEN(i);
      gen.insert(i);
      return;
    };
    auto computeKILL = [](Instruction *, DataFlowResult *) { return; };
    auto computeOUT = [](Instruction *inst,
                         Instruction *successor,
                         std::set<Value *> &OUT,
                         DataFlowResult *df) {
      auto &inS = df->IN(successor);
      OUT.insert(inS.begin(), inS.end());
      return;
    };
    auto computeIN =
        [](Instruction *inst, std::set<Value *> &IN, DataFlowResult *df) {
          auto &genI = df->GEN(inst);
          auto &outI = df->OUT(inst);
          IN.insert(outI.begin(), outI.end());
          IN.insert(genI.begin(), genI.end());
          return;
        };

    /*
     * Run the data flow analysis
     */
    auto customDfr = dfe.applyBackward(mainF,
                                       computeGEN,
                                       computeKILL,
                                       computeIN,
                                       computeOUT);

    /*
     * Print
     */
    for (auto &inst : instructions(mainF)) {
      if (!isa<LoadInst>(&inst)) {
        continue;
      }
      auto insts = customDfr->OUT(&inst);
      errs() << " Next are the " << insts.size() << " instructions ";
      errs() << "that could read the value loaded by " << inst << "\n";
      for (auto possibleInst : insts) {
        errs() << "   " << *possibleInst << "\n";
      }
    }

    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<NoellePass>();
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
