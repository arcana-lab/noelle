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
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/AssumptionCache.h"

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
        AU.addRequired<AssumptionCacheTracker>();
        AU.addRequired<DominatorTreeWrapperPass>();
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
          return new LoopDependenceInfo(entryFunction, LI, loop, graph->createLoopsSubgraph(LI));
        }

        return nullptr;
      }

      bool applyDSWP (Module &M, LoopDependenceInfo *LDI){
        auto loop = LDI->loop;
        auto sccSubgraph = LDI->sccDG;
        
        /*
        errs() << "Applying DSWP on loop\n";
        for (auto bbi = loop->block_begin(); bbi != loop->block_end(); ++bbi){
          for (auto &I : **bbi) {
            I.print(errs());
            errs() << "\n";
          }
        }
        for (auto sccI = sccSubgraph->begin_internal_node_map(); sccI != sccSubgraph->end_internal_node_map(); ++sccI) {
          sccI->first->print(errs());
        }
        errs() << sccSubgraph->numInternalNodes() << "\n";
        */

        auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*(LDI->func)).getSE();
        //auto tripCount = SE.getSmallConstantTripCount(loop);
        //auto maxTripCount = SE.getSmallConstantMaxTripCount(loop);

        /*
         * ASSUMPTION 3: Loop trip count is known.
         * ASSUMPTION 4: Loop trip count is 1000.
         */
        //errs() << "Trip count:\t" << tripCount << "\nMax trip count:\t" << maxTripCount << "\n";
        // if (tripCount != 10000) return false;

        /*
         * ASSUMPTION 5: There are only 2 SCC within the loop
         */
        // if (sccSubgraph->numInternalNodes() != 2) return false;

        /*
         * ASSUMPTION 7: You only have one variable across SCCs
         */
        for (auto edgeI = sccSubgraph->begin_edges(); edgeI != sccSubgraph->end_edges(); ++edgeI) {
          (*edgeI)->print(errs());
        }
        errs() << "Number of edges: " << std::distance(sccSubgraph->begin_edges(), sccSubgraph->end_edges()) << "\n";

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
         * ASSUMPTION 6: You have no dependencies from outside instructions 
         */
        //TODO on edge

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
         * ASSUMPTION 10: Buffer variable is of type integer 32
         * TODO: Identify scc edge variable, add AttributeList to function creation for the variable
         */
        auto stage0Pipeline = static_cast<Function *>(M.getOrInsertFunction("outSCC",IntegerType::get(M.getContext(), 8)));
        auto stage1Pipeline = static_cast<Function *>(M.getOrInsertFunction("inSCC",IntegerType::get(M.getContext(), 8)));
  
        BasicBlock* outBB= BasicBlock::Create(M.getContext(), "entry", stage0Pipeline);
        IRBuilder<> outBuilder(outBB);
        BasicBlock* inBB= BasicBlock::Create(M.getContext(), "entry", stage1Pipeline);
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

      Function *createPipelineStageFromSCC(LoopDependenceInfo *LDI, SCC *scc) {
        return nullptr;
      }

      Function * createPipelineFromSCCDG(LoopDependenceInfo *LDI, std::vector<Function *> &stages) {
        //TODO: Return a function that carries out the pipeline
      }

      void linkParallelizedLoop(LoopDependenceInfo *LDI, Function *parallelizedLoop) {
        //TODO: Alter loop header to call parallelized loop and redirect terminator inst to exit bb
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
