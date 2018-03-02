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
          auto loopPDG = fetchLoopBodyPDG(graph, loop);
          return new LoopDependenceInfo(entryFunction, LI, loop, loopPDG);
        }

        return nullptr;
      }

      PDG *fetchLoopBodyPDG(PDG *graph, Loop *loop) {
        /*
         * ASSUMPTION: Canonical induction variable
         */
        auto phiIV = loop->getCanonicalInductionVariable();
        phiIV->print(errs() << "IV:\t");
        errs() << "\n";
        assert(phiIV != nullptr);

        std::vector<Instruction *> bodyInst;
        for (auto bbi = loop->block_begin(); bbi != loop->block_end(); ++bbi) {
          BasicBlock *bb = *bbi;
          if (loop->isLoopLatch(bb) || loop->isLoopExiting(bb)) continue;
          /*
           * Ignore branch, conditional, and induction variable instructions
           */
          for (auto ii = bb->begin(); ii != --(bb->end()); ++ii) {
            Instruction *i = &*ii;
            if (CmpInst::classof(i) || phiIV == i) continue;
            bodyInst.push_back(i);
          }
        }
        return graph->createInstListSubgraph(bodyInst);
      }

      bool applyDSWP (Module &M, LoopDependenceInfo *LDI){
        auto loop = LDI->loop;
        auto sccSubgraph = LDI->sccDG;
        
        /*
         * Loop and SCC debug printouts
         */
        errs() << "Applying DSWP on loop\n";
        for (auto bbi = loop->block_begin(); bbi != loop->block_end(); ++bbi){
          if (loop->getHeader() == *bbi) {
            errs() << "Header:\n";
          } else if (loop->isLoopLatch(*bbi)) {
            errs() << "Loop latch:\n";
          } else if (loop->isLoopExiting(*bbi)) {
            errs() << "Loop exiting:\n";
          } else {
            errs() << "Loop body:\n";
          }
          for (auto &I : **bbi) {
            I.print(errs());
            errs() << "\n";
          }
        }
        errs() << "\nInternal SCCs\n";
        for (auto sccI = sccSubgraph->begin_internal_node_map(); sccI != sccSubgraph->end_internal_node_map(); ++sccI) {
          sccI->first->print(errs());
        }
        errs() << "Number of SCCs: " << sccSubgraph->numInternalNodes() << "\n";
        for (auto edgeI = sccSubgraph->begin_edges(); edgeI != sccSubgraph->end_edges(); ++edgeI) {
          (*edgeI)->print(errs());
        }
        errs() << "Number of edges: " << std::distance(sccSubgraph->begin_edges(), sccSubgraph->end_edges()) << "\n";

        /*
         * ASSUMPTION 3: Loop trip count is known.
         * ASSUMPTION 4: Loop trip count is 10000.
         */
        auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*(LDI->func)).getSE();
        //auto tripCount = SE.getSmallConstantTripCount(loop);
        //errs() << "Trip count:\t" << tripCount << "\n";
        //if (tripCount != 10001) return false;

        /*
         * ASSUMPTION 5: There are only 2 SCC within the loop's body
         */
        //if (sccSubgraph->numInternalNodes() != 2) return false;

        /*
         * ASSUMPTION 6: You only have one variable across the two SCCs
         */
        //if (std::distance(sccSubgraph->begin_edges(), sccSubgraph->end_edges()) != 1) return false;

        errs() << "Grabbing single edge between the two SCCs\n";
        DGEdge<SCC> *edge = *(sccSubgraph->begin_edges());

        /*
         * ASSUMPTION 7: There aren't memory data dependences
         */
        //if (edge->isMemoryDependence()) return false;

        /*
         * Build functions from each SCC
         */
        auto sccPair = edge->getNodePair();
        auto outSCC = sccPair.first->getNode();
        auto inSCC = sccPair.second->getNode();

        /* 
         * ASSUMPTION 8: You have no dependencies from outside instructions
         */
        //TODO on edge
        //errs() << "No dependencies from outside, and no memory dependencies\n";

        /*
         * ASSUMPTION 9: Buffer variable is of type integer 32
         * TODO: Identify scc edge variable, add AttributeList to function creation for the variable
         */
        auto stage0Pipeline = createPipelineStageFromSCC(M, LDI, outSCC, false);
        auto stage1Pipeline = createPipelineStageFromSCC(M, LDI, inSCC, true);
        
        //stage0Pipeline->print(errs() << "Function 1:\n");
        //stage1Pipeline->print(errs() << "Function 2:\n");

        /*
         * Add instructions to appropriate SCC basic blocks
         * TODO: Add push and pop instructions for the variable
         */

        return true;
      }

      Function *createPipelineStageFromSCC(Module &M, LoopDependenceInfo *LDI, SCC *scc, bool incoming) {
        auto pipelineStage = static_cast<Function *>(M.getOrInsertFunction(incoming ? "sccStage1" : "sccStage0", IntegerType::get(M.getContext(), 32)));
        BasicBlock* bb = BasicBlock::Create(M.getContext(), "entry", pipelineStage);
        IRBuilder<> builder(bb);

        /*
         * TODO: Replace with the variable this stage computes
         */
        auto retI = builder.CreateRetVoid();

        /*
         * TODO: Clone loop and it's basic blocks as well
         * TODO: Through a clone map, bind cloned instructions to each other
         */
        unordered_map<Instruction *, Instruction *> cloneMap;
        for (auto sccI = scc->begin_internal_node_map(); sccI != scc->end_internal_node_map(); ++sccI) {
          auto I = sccI->first;
          auto newI = I->clone();
          newI->insertBefore(retI);
          cloneMap[I] = newI;
          cloneMap[newI] = I;
          //I->print(errs() << "\tOld inst:\t");
          //errs() << "\n";
        }

        for (auto sccI = scc->begin_internal_node_map(); sccI != scc->end_internal_node_map(); ++sccI) {
          auto I = sccI->first;
          auto cloneI = cloneMap[I];
          I->print(errs() << "Value:\t");
          errs() << "\n";
          for (auto &op : cloneI->operands()) {
            auto opV = op.get();
            opV->print(errs() << "Operand:\t");
            errs() << "\n";
            if (auto opI = dyn_cast<Instruction>(opV)) op.set(cloneMap[opI]);
          }
        }

        for (auto sccI = scc->begin_internal_node_map(); sccI != scc->end_internal_node_map(); ++sccI) {
          auto cloneI = cloneMap[sccI->first];
          cloneI->print(errs() << "Clone Value:\t");
          errs() << "\n";
        }

        pipelineStage->print(errs() << "Function printout:\n");
        return pipelineStage;
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
