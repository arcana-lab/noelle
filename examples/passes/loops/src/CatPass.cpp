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
       * Fetch the LoopContent
       */
      auto loop = noelle.getLoopContent(LS);
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

      auto sccIterator = [](SCC *scc) -> bool {
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
     * and compute the LoopContent only for those that we care.
     */
    for (auto l : *loopStructures) {
      if (l->getNestingLevel() > 1) {
        continue;
      }

      /*
       * Get the LoopContent
       */
      auto lc = noelle.getLoopContent(l);
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
