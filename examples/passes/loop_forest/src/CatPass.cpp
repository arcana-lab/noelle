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
