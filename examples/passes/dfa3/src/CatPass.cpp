#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <algorithm>

#include "noelle/core/Noelle.hpp"

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
       * Fetch the entry point.
       */
      auto fm = noelle.getFunctionsManager();
      auto mainF = fm->getEntryFunction();

      /*
       * New data flow analysis
       *
       * Fetch the hottest loop.
       */
      auto loops = noelle.getLoopStructures();
      noelle.sortByHotness(*loops);
      auto loop = (*loops)[0];
      auto entryInst = loop->getEntryInstruction();
      errs() << "Loop:\n";
      errs() << " Function: " << loop->getFunction()->getName() << "\n";
      errs() << " Entry instruction: " << *entryInst << "\n";
      auto loopHeader = loop->getHeader();
      auto loopFunction = loop->getFunction();

      /*
       * Fetch the data flow engine.
       */
      auto dfe = noelle.getDataFlowEngine();

      /*
       * Define the data flow equations
       */
      auto computeGEN = [loop](Instruction *i, DataFlowResult *df) {
        if (!loop->isIncluded(i)){
          return ;
        }
        auto& gen = df->GEN(i);
        gen.insert(i);
        return ;
      };
      auto computeKILL = [](Instruction *, DataFlowResult *) {
        return ;
      };
      auto computeOUT = [loopHeader](std::set<Value *>& OUT, Instruction *succ, DataFlowResult *df) {

        /*
         * Check if the successor is the header.
         * In this case, we do not propagate the reachable instructions.
         * We do this because we are interested in understanding the reachability of instructions within a single iteration.
         */
        auto succBB = succ->getParent();
        if (succ == &*loopHeader->begin()) {
          return ;
        }

        /*
         * Propagate the data flow values.
         */
        auto& inS = df->IN(succ);
        OUT.insert(inS.begin(), inS.end());
        return ;
      } ;
      auto computeIN = [](std::set<Value *>& IN, Instruction *inst, DataFlowResult *df) {
        auto& genI = df->GEN(inst);
        auto& outI = df->OUT(inst);
        IN.insert(outI.begin(), outI.end());
        IN.insert(genI.begin(), genI.end());
        return ;
      };

      /*
       * Run the data flow analysis
       */
      errs() << "Data flow reachable analysis within loop iteration\n";
      auto customDfr = dfe.applyBackward(
        loopFunction, 
        computeGEN, 
        computeKILL, 
        computeIN, 
        computeOUT
        );

      /*
       * Print
       */
      for (auto bb : loop->getBasicBlocks()){
        for (auto &inst : *bb){
          auto reachableInstsWithinIteration = customDfr->OUT(&inst);
          errs() << " Next are the " << reachableInstsWithinIteration.size() << " loop instructions reachable from " << inst << "\n";
          for (auto reachInst : reachableInstsWithinIteration){
            errs() << "   " << *reachInst << "\n";
          }
        }
      }

      return false;
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
