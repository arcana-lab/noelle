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

      auto sccIterator = [sccManager](SCC *scc) -> bool {
        /*
         * Check if @scc is a single instruction
         */
        if (!scc->hasCycle()) {
          return false;
        }

        /*
         * We found a new SCC.
         */
        errs() << "   New SCC\n";

        /*
         * Print the instructions that compose the SCC.
         */
        errs() << "     Instructions:\n";
        auto mySCCIter = [](Instruction *i) -> bool {
          errs() << "       " << *i << "\n";
          return false;
        };
        scc->iterateOverInstructions(mySCCIter);

        return false;
      };

      SCCDAG->iterateOverSCCs(sccIterator);
    }
    errs() << "\n";

    /*
     * Iterate over all loops,
     * and compute the LoopDependenceInfo only for those that we care.
     */
    for (auto l : *loopStructures) {
      if (l->getNestingLevel() > 1) {
        continue;
      }

      /*
       * Get the LoopDependenceInfo
       */
      auto ldi = noelle.getLoop(l);
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
