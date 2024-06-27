#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <algorithm>

#include "arcana/noelle/core/Noelle.hpp"

using namespace arcana::noelle;

namespace {

class MyCallGraphAnalysis : public arcana::noelle::CallGraphAnalysis {
public:
  MyCallGraphAnalysis()
    : CallGraphAnalysis("Example of call graph analysis"),
      c{ 0 } {}

  CallStrength canThisFunctionBeACallee(CallBase *caller,
                                        Function &potentialCallee) override {
    auto f = caller->getFunction();
    errs() << this->prefix << "canThereBeAMemoryDataDependence: Function "
           << f->getName() << "\n";
    errs() << this->prefix << "canThereBeAMemoryDataDependence:   caller "
           << *caller << "\n";
    errs() << this->prefix << "canThereBeAMemoryDataDependence:   of "
           << potentialCallee.getName() << "\n";
    errs() << this->prefix << "canThereBeAMemoryDataDependence:\n";

    c++;

    return CallStrength::CS_MAY_EXIST;
  }

  ~MyCallGraphAnalysis() {
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
    errs() << "Example:   Register my own call graph analysis\n";
    MyCallGraphAnalysis myCGAnalysis{};
    noelle.addAnalysis(&myCGAnalysis);

    /*
     * Fetch the call graph.
     */
    errs() << "Example:   Fetch the call graph\n";
    auto fm = noelle.getFunctionsManager();
    auto cg = fm->getProgramCallGraph();

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
