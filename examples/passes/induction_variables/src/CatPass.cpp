#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "Noelle.hpp"

using namespace llvm::noelle ;

namespace {

  struct CAT : public ModulePass {
    static char ID; 

    CAT() : ModulePass(ID) {}

    bool doInitialization (Module &M) override {
      return false;
    }

    bool runOnModule (Module &M) override {

      /*
       * Fetch NOELLE
       */
      auto& noelle = getAnalysis<Noelle>();

      /*
       * fetch the loops with all their abstractions 
       * (e.g., loop dependence graph, sccdag)
       */
      auto loops = noelle.getLoops();
 
      /*
       * Check if we have profiles.
       */
      auto hot = noelle.getProfiles();
      if (!hot->isAvailable()){
        errs() << "The profiler isn't available\n";
        return false;
      }

      /*
       * Fetch the hottest loop.
       */
      noelle.sortByHotness(*loops);
      auto hottestLoop = loops->front();
      auto LS = hottestLoop->getLoopStructure();
      auto entryInst = LS->getEntryInstruction();

      /*
       * Print some information about the loop.
       */
      errs() << "Hottest loop " << *entryInst << "\n";
      errs() << " Function = " << LS->getFunction()->getName() << "\n";
      errs() << " Nesting level = " << LS->getNestingLevel() << "\n";
      errs() << " This loop has " << LS->getNumberOfSubLoops() << " sub-loops (including sub-loops of sub-loops)\n";

      /*
       * Fetch the governing IV
       */
      auto IVM = hottestLoop->getInductionVariableManager();
      auto GIV_attr = IVM->getLoopGoverningIVAttribution(*LS);
      if (  false
            || (GIV_attr == nullptr)
            ){
        errs() << " The loop doesn't have a governing IV\n";
        return false;
      }
      assert(GIV_attr->isSCCContainingIVWellFormed());
      
      /*
       * Print the GIV information.
       */
      auto cond = GIV_attr->getHeaderCmpInst();
      auto IV = GIV_attr->getInductionVariable();
      auto condValue = GIV_attr->getHeaderCmpInstConditionValue();
      auto startValue = IV.getStartValue();
      errs() << " Governing induction variable\n";
      errs() << "   Condition = " << *cond << "\n";
      errs() << "   Start value = " << *startValue << "\n";
      errs() << "   Condition value = " << *condValue << "\n";
      if (!IV.isStepValueLoopInvariant()){
        errs() << "   Step value isn't constant\n";
        return false;
      }

      /*
       * Fetch the loop governing IV utility
       */
      LoopGoverningIVUtility GIVUtility(*GIV_attr);

      /*
       * Generate the code to compute the total number of iterations for the current loop invocation.
       */
      IRBuilder<> builder(LS->getPreHeader()->getTerminator());
      auto loopIterations = GIVUtility.generateCodeToComputeTheTripCount(builder);
      errs() << "   Trip count = " << *loopIterations << "\n";

      return true;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<Noelle>();
    }
  };

}

// Next there is code to register your pass to "opt"
char CAT::ID = 0;
static RegisterPass<CAT> X("CAT", "Simple user of the Noelle framework");

// Next there is code to register your pass to "clang"
static CAT * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new CAT());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new CAT()); }}); // ** for -O0
