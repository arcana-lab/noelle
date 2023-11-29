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

      /*
       * Print some information about the loop.
       */
      errs() << "  Function = " << LS->getFunction()->getName() << "\n";
      errs() << "  Nesting level = " << LS->getNestingLevel() << "\n";

      /*
       * Fetch the invariants manager.
       */
      auto IM = loop->getInvariantManager();

      /*
       * Invariants.
       */
      errs() << "  Invariants (instruction granularity)\n";
      for (auto inv : IM->getLoopInstructionsThatAreLoopInvariants()) {
        errs() << "    " << *inv << "\n";
      }
      errs()
          << "  Invariants (instruction granularity) using a different API\n";
      for (auto inst : LS->getInstructions()) {
        auto isInstructionInvariant = IM->isLoopInvariant(inst);
        if (isInstructionInvariant) {
          errs() << "    " << *inst << "\n";
        }
      }

      /*
       * SCCDAG
       */
      auto sccManager = loop->getSCCManager();
      auto SCCDAG = sccManager->getSCCDAG();
      errs() << "  Invariants (SCC granularity)\n";
      auto sccIterator = [sccManager, IM](SCC *scc) -> bool {
        /*
         * Check if @scc is a single instruction
         */
        if (!scc->hasCycle()) {
          return false;
        }

        /*
         * Check if the SCC is invariant
         */
        auto isSCCInvariant = IM->isLoopInvariant(scc);
        if (!isSCCInvariant) {
          return false;
        }

        /*
         * We found a new SCC.
         */
        errs() << "    SCC\n";

        /*
         * Print the instructions that compose the SCC.
         */
        auto mySCCIter = [](Instruction *i) -> bool {
          errs() << "      " << *i << "\n";
          return false;
        };
        scc->iterateOverInstructions(mySCCIter);

        return false;
      };

      SCCDAG->iterateOverSCCs(sccIterator);
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
