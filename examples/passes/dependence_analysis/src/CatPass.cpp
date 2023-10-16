#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <algorithm>

#include "noelle/core/Noelle.hpp"

using namespace llvm::noelle;

namespace {

class MyDependenceAnalysis : public DependenceAnalysis {
public:
  MyDependenceAnalysis()
    : DependenceAnalysis("Example of data dependence analysis"),
      c{ 0 } {}

  bool canThereBeAMemoryDataDependence(Instruction *fromInst,
                                       Instruction *toInst,
                                       Function &f) override {
    errs() << this->prefix << "canThereBeAMemoryDataDependence: Function "
           << f.getName() << "\n";
    errs() << this->prefix << "canThereBeAMemoryDataDependence:   From "
           << *fromInst << "\n";
    errs() << this->prefix << "canThereBeAMemoryDataDependence:   To "
           << *toInst << "\n";
    errs() << this->prefix << "canThereBeAMemoryDataDependence:\n";

    c++;

    return true;
  }

  bool canThereBeAMemoryDataDependence(Instruction *fromInst,
                                       Instruction *toInst,
                                       LoopStructure &loop) override {
    auto entryInst = loop.getEntryInstruction();
    auto f = fromInst->getFunction();
    errs() << this->prefix
           << "canThereBeAMemoryDataDependence: Loop at nesting level "
           << loop.getNestingLevel() << ": " << *entryInst << "\n";
    errs() << this->prefix << "canThereBeAMemoryDataDependence:   In function "
           << f->getName() << "\n";
    errs()
        << this->prefix << "canThereBeAMemoryDataDependence:   Dependence from "
        << *fromInst << "\n";
    errs() << this->prefix << "canThereBeAMemoryDataDependence:   to "
           << *toInst << "\n";
    errs() << this->prefix << "canThereBeAMemoryDataDependence:\n";

    c++;

    return true;
  }

  ~MyDependenceAnalysis() {
    errs() << "The API has been invoked " << this->c << " times\n";
  }

private:
  std::string prefix;
  uint64_t c;
};

struct CAT : public ModulePass {
  static char ID;

  CAT() : ModulePass(ID) {}

  bool doInitialization(Module &M) override {
    return false;
  }

  bool runOnModule(Module &M) override {
    errs() << "Example: Start\n";

    /*
     * Fetch NOELLE
     */
    errs() << "Example:   Fetch NOELLE\n";
    auto &noelle = getAnalysis<Noelle>();

    /*
     * Register my data dependence analysis.
     */
    errs() << "Example:   Register my own data dependence analysis\n";
    MyDependenceAnalysis myDepAnalysis{};
    noelle.addAnalysis(&myDepAnalysis);

    /*
     * Fetch the PDG
     */
    errs() << "Example:   Fetch the PDG\n";
    auto PDG = noelle.getProgramDependenceGraph();

    /*
     * Fetch the FDG of "main"
     */
    errs() << "Example:   Fetch the FDG of \"main\"\n";
    auto fm = noelle.getFunctionsManager();
    auto mainF = fm->getEntryFunction();
    auto FDG = PDG->createFunctionSubgraph(*mainF);

    /*
     * Fetch the LDG
     */
    errs() << "Example:   Fetch the LDG of the hottest loop\n";
    auto allLoops = noelle.getLoopStructures();
    noelle.sortByHotness(*allLoops);
    auto hottestLoop = (*allLoops)[0];
    auto ldi = noelle.getLoop(hottestLoop);

    errs() << "Example: Exit\n";
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
