#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <algorithm>

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
       * Fetch the entry point.
       */
      auto fm = noelle.getFunctionsManager();
      auto mainF = fm->getEntryFunction();

      /*
       * Call graph.
       */
      auto pcf = fm->getProgramCallGraph();
      for (auto node : pcf->getFunctionNodes()){

        /*
         * Fetch the next program's function.
         */
        auto f = node->getFunction();
        if (f->empty()){
          continue ;
        }

        /*
         * Fetch the outgoing edges.
         */
        auto outEdges = node->getOutgoingEdges();
        if (outEdges.size() == 0){
          errs() << " The function \"" << f->getName() << "\" has no calls\n";
          continue ;
        }

        /*
         * Print the outgoing edges.
         */
        if (pcf->doesItBelongToASCC(f)){
          errs() << " The function \"" << f->getName() << "\" is involved in an SCC\n";
        }
        errs() << " The function \"" << f->getName() << "\"";
        errs() << " invokes the following functions:\n";
        for (auto callEdge : outEdges){
          auto calleeNode = callEdge->getCallee();
          auto calleeF = calleeNode->getFunction();
          errs() << "   [" ;
          if (callEdge->isAMustCall()){
            errs() << "must";
          } else {
            errs() << "may";
          }
          errs() << "] \"" << calleeF->getName() << "\"\n";

          /*
           * Print the sub-edges.
           */
          for (auto subEdge : callEdge->getSubEdges()){
            auto callerSubEdge = subEdge->getCaller();
            errs() << "     [";
            if (subEdge->isAMustCall()){
              errs() << "must";
            } else {
              errs() << "may";
            }
            errs() << "] " << *callerSubEdge->getInstruction() << "\n";
          }
        }
      }

      /*
       * Fetch the islands.
       */
      errs() << "Islands of the program call graph\n";
      auto islands = pcf->getIslands();
      auto islandOfMain = islands[mainF];
      for (auto& F : M){
        auto islandOfF = islands[&F];
        if (islandOfF != islandOfMain){
          errs() << " Function " << F.getName() << " is not in the same island of main\n";
        }
      }

      /*
       * Fetch the SCCCAG
       */
      auto sccCAG = pcf->getSCCCAG();
      auto mainNode = pcf->getFunctionNode(mainF);
      auto sccOfMain = sccCAG->getNode(mainNode);

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
