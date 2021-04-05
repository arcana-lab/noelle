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
    bool printDFA = false;
    bool printProfile = false;

    CAT() : ModulePass(ID) {}

    bool doInitialization (Module &M) override {
      return false;
    }

    bool runOnModule (Module &M) override {

      /*
       * Fetch NOELLE
       */
      auto& noelle = getAnalysis<Noelle>();
      errs() << "The program has " << noelle.numberOfProgramInstructions() << " instructions\n";

      /*
       * Fetch the entry point.
       */
      auto fm = noelle.getFunctionsManager();
      auto mainF = fm->getEntryFunction();

      /*
       * Data flow analysis
       */
      if (this->printDFA){
        auto dfa = noelle.getDataFlowAnalyses();
        auto dfr = dfa.runReachableAnalysis(mainF);
        errs() << "Data flow reachable analysis\n";
        for (auto& inst : instructions(mainF)){
          errs() << " Next are the instructions reachable from " << inst << "\n";
          auto& outSet = dfr->OUT(&inst);
          for (auto reachInst : outSet){
            errs() << "   " << *reachInst << "\n";
          }
        }

        /*
         * New data flow analysis
         *
         * Fetch the hottest loop.
         */
        noelle.sortByHotness(*loops);
        auto loop = (*loops)[0];
        auto loopStructure = loop->getLoopStructure();
        auto loopHeader = loopStructure->getHeader();
        auto loopFunction = loopStructure->getFunction();

        /*
         * Fetch the data flow engine.
         */
        auto dfe = noelle.getDataFlowEngine();

        /*
         * Define the data flow equations
         */
        auto computeGEN = [loopStructure](Instruction *i, DataFlowResult *df) {
          if (!loopStructure->isIncluded(i)){
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
        auto customDfr = dfe.applyBackward(loopFunction, computeGEN, computeKILL, computeIN, computeOUT);

        /*
         * Print
         */
        for (auto bb : loopStructure->getBasicBlocks()){
          for (auto &inst : *bb){
            auto reachableInstsWithinIteration = customDfr->OUT(&inst);
            errs() << " Next are the " << reachableInstsWithinIteration.size() << " loop instructions reachable from " << inst << "\n";
            for (auto reachInst : reachableInstsWithinIteration){
              errs() << "   " << *reachInst << "\n";
            }
          }
        }
      }

      /*
       * Call graph.
       */
      errs() << "Program call graph\n";
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
        errs() << " The function \"" << f->getName() << "\" invokes the following functions:\n";
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
