#include <algorithm>

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "arcana/noelle/core/NoellePass.hpp"
#include "arcana/noelle/core/InductionVariableSCC.hpp"
#include "arcana/noelle/core/ReductionSCC.hpp"
#include "arcana/noelle/core/LoopIterationSCC.hpp"
#include "arcana/noelle/core/LoopCarriedUnknownSCC.hpp"
#include "arcana/noelle/core/MemoryClonableSCC.hpp"

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

    /*
     * fetch the loops with all their abstractions
     * (e.g., loop dependence graph, sccdag)
     */
    auto loopStructures = noelle.getLoopStructures();

    /*
     * Print loop induction variables and invariant.
     */
    for (auto LS : *loopStructures) {

      /*
       * Print the first instruction the loop executes.
       */
      auto entryInst = LS->getEntryInstruction();
      errs() << "Loop " << *entryInst << "\n";

      /*
       * Fetch the LoopDependenceInfo
       */
      auto loop = noelle.getLoopContent(LS);

      /*
       * Print some information about the loop.
       */
      errs() << " Function = " << LS->getFunction()->getName() << "\n";
      errs() << " Nesting level = " << LS->getNestingLevel() << "\n";

      /*
       * Fetch the induction variables manager
       */
      auto IVM = loop->getInductionVariableManager();

      /*
       * Iterate over all induction variables.
       */
      errs() << " Induction variables\n";
      for (auto IV : IVM->getInductionVariables()) {

        /*
         * Print the main PHI of the IV
         */
        errs() << "   IV: " << *IV->getLoopEntryPHI() << "\n";

        /*
         * Print the type of the variable that behaves like an IV
         */
        auto t = IV->getType();
        errs() << "     Type of the IV: " << *t << "\n";

        /*
         * Print the start value of the IV
         */
        auto s = IV->getStartValue();
        errs() << "     Start value = " << *s << "\n";

        /*
         * Print the sequence of computation steps that computes the delta that
         * is applied to each update of the IV.
         */
        errs()
            << "     Sequence of computation steps that computes the delta that is applied to each update of the IV:\n";
        for (auto stepValueComputationInstruction :
             IV->getComputationOfStepValue()) {
          errs() << "       " << *stepValueComputationInstruction << "\n";
        }

        /*
         * Print the SCC
         */
        auto scc = IV->getSCC();
        errs() << "     SCC has " << scc->numberOfInstructions()
               << " number of instructions\n";

        /*
         * Print the PHIs
         */
        errs() << "     PHIs that compose the SCC of the IV\n";
        auto phis = IV->getPHIs();
        for (auto phi : phis) {
          errs() << "       " << *phi << "\n";
        }
      }

      /*
       * Print the loop governing induction variable
       */
      auto GIV = IVM->getLoopGoverningInductionVariable();
      if (GIV != nullptr) {
        errs() << "   The loop has a loop governing IV\n";

        /*
         * Look at the GIV as a normal IV
         */
        auto IV = GIV->getInductionVariable();
        errs() << "     GIV seen as an IV: " << *IV->getLoopEntryPHI() << "\n";

        /*
         * Print the exit condition
         */
        auto exitCondition = GIV->getExitConditionValue();
        errs() << "     Exit condition = " << *exitCondition << "\n";

        /*
         * Print the value to compare against the exit condition.
         */
        auto valueToCompare = GIV->getValueToCompareAgainstExitConditionValue();
        errs()
            << "     Evolving value to compare = " << *valueToCompare << "\n";

        /*
         * Print the comparison
         */
        auto cmpInst = GIV->getHeaderCompareInstructionToComputeExitCondition();
        errs() << "     Comparison = " << *cmpInst << "\n";
      }
    }
    errs() << "\n";

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
