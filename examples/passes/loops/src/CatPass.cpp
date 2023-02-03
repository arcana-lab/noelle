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
     * Fetch the entry point.
     */
    auto fm = noelle.getFunctionsManager();
    auto mainF = fm->getEntryFunction();

    /*
     * fetch the loops with all their abstractions
     * (e.g., loop dependence graph, sccdag)
     */
    auto loopStructures = noelle.getLoopStructures();
    // auto loopStructures = noelle.getLoopStructures(mainF);

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
       * Induction variables.
       */
      errs() << " Induction variables\n";
      auto IVM = loop->getInductionVariableManager();
      auto GIV = IVM->getLoopGoverningInductionVariable(*LS);
      if (GIV != nullptr) {
        errs() << "   GIV: " << *GIV->getLoopEntryPHI() << "\n";
      }
      auto IVs = IVM->getInductionVariables(*LS);
      for (auto IV : IVs) {
        errs() << "   IV: " << *IV->getLoopEntryPHI() << "\n";
      }

      /*
       * Invariants.
       */
      errs() << " Invariants\n";
      auto IM = loop->getInvariantManager();
      for (auto inv : IM->getLoopInstructionsThatAreLoopInvariants()) {
        errs() << "   " << *inv << "\n";
      }

      /*
       * Trip count.
       */
      if (loop->doesHaveCompileTimeKnownTripCount()) {
        errs() << " Trip count = " << loop->getCompileTimeTripCount() << "\n";
      }

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

        /*
         * Fetch the SCC information.
         */
        auto sccInfo = sccManager->getSCCAttrs(scc);
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

    /*
     * Fetch the loop forest.
     */
    auto loopForest = noelle.organizeLoopsInTheirNestingForest(*loopStructures);

    /*
     * Define the iterator that will print all nodes of a tree.
     */
    std::function<void(LoopForestNode *)> printTree =
        [&printTree](LoopForestNode *n) {
          /*
           * Print the current node.
           */
          auto l = n->getLoop();
          for (auto i = 1; i < l->getNestingLevel(); i++) {
            errs() << "-";
          }
          errs() << "-> ";
          errs() << "[ " << l->getFunction()->getName() << " ] "
                 << *l->getEntryInstruction() << "\n";

          /*
           * Print the children
           */
          for (auto c : n->getDescendants()) {
            printTree(c);
          }

          return;
        };

    /*
     * Iterate over the trees that compose the forest.
     */
    errs() << "Printing the loop forest\n";
    for (auto loopTree : loopForest->getTrees()) {

      /*
       * Fetch the root of the current tree.
       */
      auto rootLoop = loopTree->getLoop();
      errs() << "======= Tree with root " << *rootLoop->getEntryInstruction()
             << "\n";
      printTree(loopTree);
      errs() << "\n";
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
