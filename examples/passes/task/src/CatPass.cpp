#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "noelle/core/Noelle.hpp"
#include "noelle/core/Task.hpp"

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
     * Check if we have profiles.
     */
    auto hot = noelle.getProfiles();
    if (!hot->isAvailable()) {
      errs() << "The profiler isn't available\n";
      return false;
    }

    /*
     * Fetch the loops with all their abstractions
     * (e.g., loop dependence graph, sccdag)
     */
    auto loops = noelle.getLoopStructures();
    if (loops->size() == 0) {
      errs() << "The program has no loops\n";
      return false;
    }

    /*
     * Fetch the hottest loop.
     */
    noelle.sortByHotness(*loops);
    auto hottestLoop = loops->front();
    auto entryInst = hottestLoop->getEntryInstruction();
    errs() << "Hottest loop " << *entryInst << "\n";
    errs() << " Function = " << hottestLoop->getFunction()->getName() << "\n";
    errs() << " Nesting level = " << hottestLoop->getNestingLevel() << "\n";

    /*
     * Define the signature of the task.
     */
    auto tm = noelle.getTypesManager();
    auto funcArgTypes = ArrayRef<Type *>({ tm->getVoidPointerType() });
    auto taskSignature =
        FunctionType::get(tm->getVoidType(), funcArgTypes, false);

    /*
     * Create an empty task.
     */
    auto t = new Task(taskSignature, M);

    /*
     * Define the body.
     */
    auto filter = [](Instruction *i) -> bool { return true; };
    t->cloneAndAddBasicBlocks(hottestLoop->getBasicBlocks(), filter);

    /*
     * Adjust the data flows within the clones within the task body.
     */
    // TODO

    return true;
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
