#include <algorithm>

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "arcana/noelle/core/Noelle.hpp"
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
    auto &noelle = getAnalysis<Noelle>();

    /*
     * Fetch the loop forest.
     */
    auto loopStructures = noelle.getLoopStructures();
    auto loopForest = noelle.organizeLoopsInTheirNestingForest(*loopStructures);

    /*
     * Fetch main
     */
    auto fm = noelle.getFunctionsManager();
    auto mainF = fm->getEntryFunction();

    /*
     * Check the loop that contains each instruction of main.
     */
    errs() << "Check loops that contain instructions in main\n";
    for (auto &inst : instructions(mainF)) {
      errs() << "  Instruction: " << inst << "\n";

      /*
       * Fetch the loop.
       */
      auto loop = loopForest->getInnermostLoopThatContains(&inst);
      if (loop == nullptr) {
        errs() << "    The instruction does not belong in any loop\n";
        continue;
      }

      errs() << "    The instruction belongs to a loop\n";
    }

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

      /*
       * Print the tree.
       */
      this->printTree(loopTree);
      errs() << "\n";
    }

    return false;
  }

  void printTree(LoopTree *n) {

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
      this->printTree(c);
    }

    return;
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
