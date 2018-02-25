#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/IRBuilder.h"

#include "../include/LoopDependenceInfo.hpp"
#include "PDG.hpp"
#include "PDGAnalysis.hpp"

#include <unordered_map>

using namespace llvm;

namespace llvm {

  struct DSWP : public ModulePass {
    public:
      static char ID;

      DSWP() : ModulePass{ID} {}

      bool doInitialization (Module &M) override {
        return false;
      }

      bool runOnModule (Module &M) override {
        errs() << "DSWP for " << M.getName() << "\n";

        /*
         * Fetch the PDG.
         */
        auto graph = getAnalysis<PDGAnalysis>().getPDG();

        /*
         * Fetch the loop to parallelize
         */
        auto loopDI = fetchLoopToParallelize(M, graph);
        if (loopDI == nullptr){
          return false;
        }

        /*
         * Parallelize the loop
         */
        auto modified = applyDSWP(M, loopDI);

        delete loopDI;

        return modified;
      }

      void getAnalysisUsage(AnalysisUsage &AU) const override {
        AU.addRequired<PDGAnalysis>();
        AU.addRequired<LoopInfoWrapperPass>();
        AU.addRequired<ScalarEvolutionWrapperPass>();

        return ;
      }

    private:
      LoopDependenceInfo *fetchLoopToParallelize (Module &M, PDG *graph){

        /* 
         * ASSUMPTION 1: One function in the entire program.
         * Fetch the entry point of the program.
         */
        auto entryFunction = M.getFunction("main");

        /*
         * Fetch the loops.
         */
        
        auto &LI = getAnalysis<LoopInfoWrapperPass>(*entryFunction).getLoopInfo();

        /*
         * ASSUMPTION 2: One loop in the entire function 
         * Choose the loop to parallelize.
         */
        
        for (auto loopIter : LI){
          auto loop = &*loopIter;
          return new LoopDependenceInfo(LI, loop, graph->createLoopsSubgraph(LI));
        }

        return nullptr;
      }

      bool applyDSWP (Module &M, LoopDependenceInfo *LDI){
        errs() << "Applying DSWP on loop\n";

        auto loop = LDI->loop;
        auto sccSubgraph = LDI->sccDG;
        auto& SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
        auto tripCount = SE.getSmallConstantTripCount(loop);

        /*
         * ASSUMPTION 3: Loop trip count is known.
         * ASSUMPTION 4: Loop trip count is 1000 or less.
         */
        if (tripCount <= 0 || tripCount > 1000) return false;

        /*
         * ASSUMPTION 5: There are only 2 SCC within the loop
         */
        std::pair<SCC *, SCC *> outInSCC;
        
        /*
         * ASSUMPTION 6: You only have one variable across SCCs
         * ASSUMPTION 7: You have no dependencies from outside instructions 
         */
        if (std::distance(sccSubgraph->begin_edges(), sccSubgraph->end_edges()) > 1) return false;

        errs() << "Grabbing single edge between the two SCCs\n";

        DGEdge<SCC> *edge = *(sccSubgraph->begin_edges());

        /*
         * ASSUMPTION 8: There aren't memory data dependences
         */
        if (edge->isMemoryDependence()) return false;

        /*
         * Build functions from each SCC
         */
        auto sccPair = edge->getNodePair();
        auto outSCC = sccPair.first->getNode();
        auto inSCC = sccPair.second->getNode();

        /*
         * Attribute instructions to their SCCs
         */
        std::unordered_map<Instruction *, SCC *> instSCCMap;

        for (auto nodeI = outSCC->begin_nodes(); nodeI != outSCC->end_nodes(); ++nodeI) {
          instSCCMap[(*nodeI)->getNode()] = outSCC;
        }
        for (auto nodeI = inSCC->begin_nodes(); nodeI != inSCC->end_nodes(); ++nodeI) {
          instSCCMap[(*nodeI)->getNode()] = inSCC;
        }

        /*
         * ASSUMPTION 9: No function in the module is named "outSCC" or "inSCC"
         * TODO: Identify scc edge variable, add AttributeList to function creation for the variable
         */
        Function *outF = static_cast<Function *>(M.getOrInsertFunction("outSCC",IntegerType::get(M.getContext(), 8)));
        Function *inF = static_cast<Function *>(M.getOrInsertFunction("outSCC",IntegerType::get(M.getContext(), 8)));
  
        BasicBlock* outBB= BasicBlock::Create(M.getContext(), "entry", outF);
        IRBuilder<> outBuilder(outBB);
        BasicBlock* inBB= BasicBlock::Create(M.getContext(), "entry", inF);
        IRBuilder<> inBuilder(inBB);

        /*
         * Add instructions to appropriate SCC basic blocks
         * TODO: Add push and pop instructions for the variable
         */
        for (auto bbi = loop->block_begin(); bbi != loop->block_end(); ++bbi){
          BasicBlock *bb = *bbi;
          for (auto &I : *bb) {
            if (instSCCMap[&I] == outSCC) {
              outBuilder.Insert(&I);
            } else if (instSCCMap[&I] == inSCC) {
              inBuilder.Insert(&I);
            } else {
              // Should not be here. TODO: throw abort.
            }
          }
        }

        return true;
      }
  };

}

// Next there is code to register your pass to "opt"
char llvm::DSWP::ID = 0;
static RegisterPass<DSWP> X("DSWP", "DSWP parallelization");

// Next there is code to register your pass to "clang"
static DSWP * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new DSWP());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new DSWP());}});// ** for -O0
