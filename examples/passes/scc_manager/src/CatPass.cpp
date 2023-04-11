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
      auto loop = noelle.getLoop(LS);
      auto loopNode = loop->getLoopHierarchyStructures();

      /*
       * Print some information about the loop.
       */
      errs() << " Function = " << LS->getFunction()->getName() << "\n";
      errs() << " Nesting level = " << LS->getNestingLevel() << "\n";
      errs() << " This loop has " << loopNode->getNumberOfSubLoops()
             << " sub-loops (including sub-loops of sub-loops)\n";

      /*
       * Dependences.
       */
      auto LDG = loop->getLoopDG();

      /*
       * SCCDAG
       */
      errs() << " SCCDAG\n";
      auto sccManager = loop->getSCCManager();
      auto SCCDAG = sccManager->getSCCDAG();

      /*
       * Check all SCCs.
       */
      for (auto scc : SCCDAG->getSCCs()) {

        /*
         * Print the instructions that compose the SCC.
         */
        errs() << "   Instructions that compose the new SCC:\n";
        auto mySCCIter = [](Instruction *i) -> bool {
          errs() << "       " << *i << "\n";
          return false;
        };
        scc->iterateOverInstructions(mySCCIter);

        /*
         * Fetch the SCC information.
         */
        auto sccInfo = sccManager->getSCCAttrs(scc);

        /*
         * Check the SCC type
         */
        if (isa<InductionVariableSCC>(sccInfo)) {
          errs()
              << "     It is due to the computation of an induction variable\n";

        } else if (isa<ReductionSCC>(sccInfo)) {
          errs() << "     It can be reduced\n";

        } else if (isa<LoopIterationSCC>(sccInfo)) {
          errs() << "     It doesn't have loop-carried dependences\n";

        } else if (isa<LoopCarriedUnknownSCC>(sccInfo)) {
          errs() << "     It must be executed sequentially\n";

        } else if (isa<MemoryClonableSCC>(sccInfo)) {
          errs()
              << "     It can run in parallel after cloning memory objects\n";

        } else {
          errs() << "     Unknown\n";
        }

        if (auto redSCC = dyn_cast<ReductionSCC>(sccInfo)) {

          /*
           * Use the APIs of this specific SCC.
           */
          errs()
              << "   Identity value = " << redSCC->getIdentityValue() << "\n";
          errs() << "   PHI accumulator = "
                 << redSCC->getPhiThatAccumulatesValuesBetweenLoopIterations()
                 << "\n";
        }
      }
    }
    errs() << "\n";

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
