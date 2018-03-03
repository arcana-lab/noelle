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
        auto modified = applyDSWP(loopDI);

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
        auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*entryFunction).getSE();

        /*
         * ASSUMPTION 2: One loop in the entire function 
         * Choose the loop to parallelize.
         */
        
        for (auto loopIter : LI){
          auto loop = &*loopIter;
          auto loopPDG = graph->createLoopsSubgraph(LI);
          auto instPair = divideLoopInstructions(loop);
          return new LoopDependenceInfo(entryFunction, LI, SE, loop, loopPDG, instPair.first, instPair.second);
        }

        return nullptr;
      }

      std::pair<std::vector<Instruction *>, std::vector<Instruction *>>
      divideLoopInstructions(Loop *loop) {
        std::vector<Instruction *> bodyInst, otherInst;

        /*
         * ASSUMPTION: Canonical induction variable
         */
        auto phiIV = loop->getCanonicalInductionVariable();
        assert(phiIV != nullptr);
        //phiIV->print(errs() << "IV:\t"); errs() << "\n";

        bool nonBodyBB = false;
        for (auto bbi = loop->block_begin(); bbi != loop->block_end(); ++bbi) {
          BasicBlock *bb = *bbi;
          nonBodyBB = loop->isLoopLatch(bb);

          /*
           * Categorize all instructions in latch or exit basic blocks as 'other' instructions
           * Categorize branch, conditional, and induction variable instructions as 'other' instructions
           */
          for (auto ii = bb->begin(); ii != bb->end(); ++ii) {
            Instruction *i = &*ii;
            if (nonBodyBB || TerminatorInst::classof(i) || CmpInst::classof(i) || phiIV == i) {
              otherInst.push_back(i);
            } else {
              bodyInst.push_back(i);
            }
          }
        }

        /*
         * ASSUMPTION: One exiting block only; excluding exit block instructions
         */
        for (auto &I : *(loop->getUniqueExitBlock())) {
          otherInst.push_back(&I);
        }
        return make_pair(bodyInst, otherInst);
      }

      bool applyDSWP (LoopDependenceInfo *LDI){
        auto loop = LDI->loop;
        auto sccSubgraph = LDI->sccBodyDG;
        
        /*
         * Loop and SCC debug printouts
         */
        //printLoop(loop);
        //printSCCs(sccSubgraph);

        /*
         * ASSUMPTION 3: Loop trip count is known.
         * ASSUMPTION 4: Loop trip count is 10000.
         */
        auto tripCount = LDI->SE.getSmallConstantTripCount(loop);
        //errs() << "Trip count:\t" << tripCount << "\n";
        if (tripCount != 10001) return false;

        /*
         * ASSUMPTION 5: There are only 2 SCC within the loop's body
         */
        errs() << "Num nodes: " << sccSubgraph->numInternalNodes() << "\n";
        if (sccSubgraph->numInternalNodes() != 2) return false;

        /*
         * ASSUMPTION 6: You only have one variable across the two SCCs
         */
        errs() << "Num edges: " << std::distance(sccSubgraph->begin_edges(), sccSubgraph->end_edges()) << "\n";
        if (std::distance(sccSubgraph->begin_edges(), sccSubgraph->end_edges()) != 1) return false;
        DGEdge<SCC> *edge = *(sccSubgraph->begin_edges());

        /*
         * ASSUMPTION 7: There aren't memory data dependences
         */
        errs() << "Mem dep: " << edge->isMemoryDependence() << "\n";
        if (edge->isMemoryDependence()) return false;

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

        auto stage0Pipeline = createPipelineStageFromSCC(LDI, outSCC, false);
        auto stage1Pipeline = createPipelineStageFromSCC(LDI, inSCC, true);
        
        return true;
      }

      Function *createPipelineStageFromSCC(LoopDependenceInfo *LDI, SCC *scc, bool incoming) {
        auto M = LDI->func->getParent();
        auto loop = LDI->loop;
        auto int32 = IntegerType::get(M->getContext(), 32);
        auto funcConst = incoming ? M->getOrInsertFunction("sccStage1", int32, int32) : M->getOrInsertFunction("sccStage0", int32);
        Function * pipelineStage = static_cast<Function *>(funcConst);

        BasicBlock* entryBB = BasicBlock::Create(M->getContext(), "entry", pipelineStage);
        IRBuilder<> entryBuilder(entryBB);
        BasicBlock* exitBB = BasicBlock::Create(M->getContext(), "exit", pipelineStage);
        IRBuilder<> exitBuilder(exitBB);

        /*
         * ASSUMPTION: Variable computed is stored in a PHI node
         */
        ReturnInst *retI;
        for (auto sccI = scc->begin_internal_node_map(); sccI != scc->end_internal_node_map(); ++sccI) {
          if (auto phiI = dyn_cast<PHINode>(sccI->first)) {
            retI = exitBuilder.CreateRet((Value*)phiI);
            break;
          }
        }

        /*
         * Clone loop instructions in given SCC or non-loop-body 
         */
        unordered_map<Instruction *, Instruction *> cloneMap;
        for (auto sccI = scc->begin_internal_node_map(); sccI != scc->end_internal_node_map(); ++sccI) {
          auto I = sccI->first;
          auto newI = I->clone();
          cloneMap[I] = newI;
        }

        for (auto &I : LDI->otherInstOfLoop) {
          auto newI = I->clone();
          cloneMap[I] = newI;
        }

        if (incoming) {
          auto edge = *(scc->begin_edges());
          auto I = edge->getNodePair().first->getNode();
          // TODO: Change the clone to a pop call from the buffer (Threadpool API)
          auto newI = I->clone();
          cloneMap[I] = newI;
        }

        /*
         * Clone loop basic blocks that are used by given SCC / non-loop-body basic blocks
         */
        unordered_map<BasicBlock*, BasicBlock*> bbCloneMap;
        for (auto bbi = loop->block_begin(); bbi != loop->block_end(); ++bbi) {
          auto bb = *bbi;
          auto cloneBB = BasicBlock::Create(M->getContext(), bb->getName(), pipelineStage);
          IRBuilder<> builder(cloneBB);

          for (auto &I : *bb) {
            auto cloneI = cloneMap.find(&I);
            if (cloneI == cloneMap.end()) {
              //I.print(errs() << "DID NOT ADD:\t"); errs() << "\n";
              continue;
            }
            cloneMap[&I] = builder.Insert(cloneI->second);
          }

          bbCloneMap[bb] = cloneBB;
        }

        /*
         * Replace each clone's operand with the cloned instruction's version of the operand
         */
        for (auto ii = cloneMap.begin(); ii != cloneMap.end(); ++ii) {
          for (auto &op : ii->second->operands()) {
            auto opV = op.get();
            if (auto opI = dyn_cast<Instruction>(opV)) {
              op.set(cloneMap[opI]);
            } else if (auto opB = dyn_cast<BasicBlock>(opV)) {
              op.set(bbCloneMap[opB]);
            }
          }
        }

        pipelineStage->print(errs() << "Function printout:\n"); errs() << "\n";
        return pipelineStage;
      }

      Function * createPipelineFromSCCDG(LoopDependenceInfo *LDI, std::vector<Function *> &stages) {
        //TODO: Return a function that carries out the pipeline
      }

      void linkParallelizedLoop(LoopDependenceInfo *LDI, Function *parallelizedLoop) {
        //TODO: Alter loop header to call parallelized loop and redirect terminator inst to exit bb
      }

      void printLoop(Loop *loop) {
        errs() << "Applying DSWP on loop\n";
        auto header = loop->getHeader();
        errs() << "Number of bbs: " << std::distance(loop->block_begin(), loop->block_end()) << "\n";
        for (auto bbi = loop->block_begin(); bbi != loop->block_end(); ++bbi){
          auto bb = *bbi;
          if (header == bb) {
            errs() << "Header:\n";
          } else if (loop->isLoopLatch(bb)) {
            errs() << "Loop latch:\n";
          } else if (loop->isLoopExiting(bb)) {
            errs() << "Loop exiting:\n";
          } else {
            errs() << "Loop body:\n";
          }
          for (auto &I : *bb) {
            I.print(errs());
            errs() << "\n";
          }
        }
      }

      void printSCCs(SCCDG *sccSubgraph) {
        errs() << "\nInternal SCCs\n";
        for (auto sccI = sccSubgraph->begin_internal_node_map(); sccI != sccSubgraph->end_internal_node_map(); ++sccI) {
          sccI->first->print(errs());
        }
        errs() << "Number of SCCs: " << sccSubgraph->numInternalNodes() << "\n";
        for (auto edgeI = sccSubgraph->begin_edges(); edgeI != sccSubgraph->end_edges(); ++edgeI) {
          (*edgeI)->print(errs());
        }
        errs() << "Number of edges: " << std::distance(sccSubgraph->begin_edges(), sccSubgraph->end_edges()) << "\n";
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
