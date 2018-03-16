#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/AssumptionCache.h"

#include "llvm/IR/Mangler.h"
#include "llvm/IR/IRBuilder.h"

#include "../include/LoopDependenceInfo.hpp"
#include "../include/PipelineInfo.hpp"
#include "PDG.hpp"
#include "SCC.hpp"
#include "SCCDG.hpp"
#include "PDGAnalysis.hpp"

#include <unordered_map>

using namespace llvm;

namespace llvm {

  struct DSWP : public ModulePass {
    public:
      static char ID;

      Function *queuePushTemporary, *queuePopTemporary, *stageHandler, *printReached, *printReachedIter;
      FunctionType *stageType;
      IntegerType *int32;

      DSWP() : ModulePass{ID} {}

      bool doInitialization (Module &M) override
      {
        return false;
      }

      bool runOnModule (Module &M) override
      {
        errs() << "DSWP for " << M.getName() << "\n";

        /*
         * Helpers for pipeline stage generation
         */
        int32 = IntegerType::get(M.getContext(), 32);
        queuePushTemporary = M.getFunction("queuePush");
        queuePopTemporary = M.getFunction("queuePop");
        stageHandler = M.getFunction("parallelizeHandler");
        printReached = M.getFunction("printReached");
        printReachedIter = M.getFunction("printReachedIter");
        stageType = cast<FunctionType>(cast<PointerType>(stageHandler->arg_begin()->getType())->getElementType());

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

      void getAnalysisUsage(AnalysisUsage &AU) const override
      {
        AU.addRequired<PDGAnalysis>();
        AU.addRequired<AssumptionCacheTracker>();
        AU.addRequired<DominatorTreeWrapperPass>();
        AU.addRequired<LoopInfoWrapperPass>();
        AU.addRequired<ScalarEvolutionWrapperPass>();
        return ;
      }

    private:
      LoopDependenceInfo *fetchLoopToParallelize (Module &M, PDG *graph)
      {
        /* 
         * ASSUMPTION 1: One function in the entire program.
         * Fetch the entry point of the program.
         */
        auto entryFunction = M.getFunction("main");

        /*
         * Fetch the loops.
         */
        auto &LI = getAnalysis<LoopInfoWrapperPass>(*entryFunction).getLoopInfo();
        auto &DT = getAnalysis<DominatorTreeWrapperPass>(*entryFunction).getDomTree();
        auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*entryFunction).getSE();

        auto funcPDG = graph->createFunctionSubgraph(*entryFunction);

        /*
         * ASSUMPTION 2: One loop in the entire function 
         * Choose the loop to parallelize.
         */
        for (auto loopIter : LI)
        {
          auto loop = &*loopIter;
          auto instPair = divideLoopInstructions(loop);
          return new LoopDependenceInfo(entryFunction, LI, DT, SE, loop, funcPDG, instPair.first, instPair.second);
        }

        return nullptr;
      }

      std::pair<std::vector<Instruction *>, std::vector<Instruction *>> divideLoopInstructions(Loop *loop)
      {
        auto phiIV = loop->getCanonicalInductionVariable();
        assert(phiIV != nullptr);

        std::vector<Instruction *> bodyInst, otherInst;
        for (auto bbi = loop->block_begin(); bbi != loop->block_end(); ++bbi)
        {
          BasicBlock *bb = *bbi;
          auto isLatchBB = loop->isLoopLatch(bb);

          /*
           * Categorize branch, conditional, and induction variable instructions as 'other' instructions
           */
          for (auto ii = bb->begin(); ii != bb->end(); ++ii)
          {
            Instruction *i = &*ii;
            if (isLatchBB || TerminatorInst::classof(i) || CmpInst::classof(i) || phiIV == i)
            {
              otherInst.push_back(i);
              continue;
            }
            bodyInst.push_back(i);
          }
        }

        return make_pair(bodyInst, otherInst);
      }

      void collectLoopExternalDependents(LoopDependenceInfo *LDI, std::vector<StageInfo *> &stages)
      {
        for (auto nodeI : LDI->loopDG->externalNodePairs())
        {
          auto externalNode = nodeI.second;
          for (auto sccNodeI : make_range(externalNode->begin_incoming_nodes(), externalNode->end_incoming_nodes()))
          {
            auto internalInst = sccNodeI->getNode();
            for (auto stage : stages)
            {
              if (!stage->scc->isInternal(internalInst)) continue;
              stage->outgoingDependentMap[internalInst] = externalNode->getNode();
            }
          }
        }
      }

      bool applyDSWP (LoopDependenceInfo *LDI)
      {
        StageInfo outSCCStage, inSCCStage;
        std::vector<StageInfo *> stages = { &outSCCStage, &inSCCStage };
        
        if (!locateTwoSCCStageLoop(LDI, stages)) return false;

        createPipelineStageFromSCC(LDI, stages[0]);
        createPipelineStageFromSCC(LDI, stages[1]);

        auto pipelineBB = createParallelizedFunctionExecution(LDI, stages);
        if (pipelineBB == nullptr) {
          stages[0]->sccStage->eraseFromParent();
          stages[1]->sccStage->eraseFromParent();
          return false;
        }

        linkParallelizedLoop(LDI, pipelineBB);
        LDI->func->print(errs() << "Final function:\n"); errs() << "\n";

        return true;
      }

      bool locateTwoSCCStageLoop(LoopDependenceInfo *LDI, std::vector<StageInfo *> &stages)
      {
        auto loop = LDI->loop;
        auto sccSubgraph = LDI->sccBodyDG;

        /*
         * ASSUMPTION 3: Loop trip count is known.
         */
        if (LDI->SE.getSmallConstantTripCount(loop) == 0) return false;

        /*
         * ASSUMPTION 4: There are only 2 SCC within the loop's body
         */
        if (sccSubgraph->numInternalNodes() != 2) return false;

        /*
         * ASSUMPTION 5: You only have one variable across the two SCCs
         */
        if (std::distance(sccSubgraph->begin_edges(), sccSubgraph->end_edges()) != 1) return false;

        DGEdge<SCC> *sccEdge = *(sccSubgraph->begin_edges());
        auto sccPair = sccEdge->getNodePair();
        auto outSCC = stages[0]->scc = sccPair.first->getNode();
        auto inSCC = stages[1]->scc = sccPair.second->getNode();

        /*
         * ASSUMPTION 6: You only have one dependency for the variable across the two SCCs 
         */
        if (outSCC->numExternalNodes() != 1 || inSCC->numExternalNodes() != 1) return false;

        DGEdge<Instruction> *instEdge = *(outSCC->begin_external_node_map()->second->begin_incoming_edges());
        assert(instEdge == *(inSCC->begin_external_node_map()->second->begin_outgoing_edges()));

        /*
         * ASSUMPTION 7: There aren't memory data dependences
         */
        if (instEdge->isMemoryDependence()) return false;

        stages[0]->outgoingSCCEdges = {instEdge};
        stages[1]->incomingSCCEdges = {instEdge};
        collectLoopExternalDependents(LDI, stages);

        /* 
         * ASSUMPTION 8: You only have one dependency per SCC from inside to outside the loop
         */
        if (stages[0]->outgoingDependentMap.size() != 1 || stages[1]->outgoingDependentMap.size() != 1) return false;

        return true;
      }

      void cloneLoopInstForStage(LoopDependenceInfo *LDI, StageInfo *stageInfo, unordered_map<Instruction *, Instruction *> &cloneMap)
      {
        for (auto sccI = stageInfo->scc->begin_internal_node_map(); sccI != stageInfo->scc->end_internal_node_map(); ++sccI)
        {
          auto I = sccI->first;
          auto newI = I->clone();
          cloneMap[I] = newI;
        }

        /*
         * ASSUMPTION: All instructions outside of SCCs are related to the loop's induction variable that controls the loop
         */
        for (auto &I : LDI->otherInstOfLoop)
        {
          auto newI = I->clone();
          cloneMap[I] = newI;
        }
      }

      void createPipelineQueueing(StageInfo *stageInfo, Value *queueArg, unordered_map<Instruction *, Instruction *> &cloneMap, IRBuilder<> entryBuilder)
      {

        /*
         * Locate clone of outgoing instruction, create queue push call
         */
        for (auto edge : stageInfo->outgoingSCCEdges)
        {
          auto outgoingI = edge->getNodePair().first->getNode();
          auto valuePush = new OutgoingPipelineInfo();
          stageInfo->valuePushQueues.push_back(valuePush);
          valuePush->valueInstruction = cloneMap[outgoingI];
          auto queueCallArgs = ArrayRef<Value*>({ queueArg, static_cast<Value *>(valuePush->valueInstruction) });
          valuePush->pushQueueCall = entryBuilder.CreateCall(queuePushTemporary, queueCallArgs);
        }

        /*
         * Locate clone of incoming instruction, create queue pop call and load, and point instruction to the load
         */
        for (auto edge : stageInfo->incomingSCCEdges)
        {
          auto outgoingI = edge->getNodePair().first->getNode();
          auto incomingI = edge->getNodePair().second->getNode();

          auto valuePopIter = stageInfo->valuePopQueuesMap.find(outgoingI);
          IncomingPipelineInfo *valuePop;
          if (valuePopIter == stageInfo->valuePopQueuesMap.end())
          {
            valuePop = new IncomingPipelineInfo();
            stageInfo->valuePopQueuesMap[outgoingI] = valuePop;
            valuePop->popStorage = entryBuilder.CreateAlloca(int32);
            valuePop->loadStorage = entryBuilder.CreateLoad(valuePop->popStorage);
            auto queueCallArgs = ArrayRef<Value*>({ queueArg, static_cast<Value *>(valuePop->popStorage) });
            valuePop->popQueueCall = entryBuilder.CreateCall(queuePopTemporary, queueCallArgs);
          }
          else
          {
            valuePop = valuePopIter->second;
          }

          auto userInstruction = cloneMap[incomingI];
          valuePop->userInstructions.push_back(userInstruction);

          /*
           * FIX: Use edge to identify dependent operand
           */
          for (auto useI = incomingI->op_begin(); useI != incomingI->op_end(); ++useI)
          {
            if (auto opI = dyn_cast<Instruction>(useI->get()))
            {
              if (opI != outgoingI) continue;
              userInstruction->setOperand(useI->getOperandNo(), valuePop->loadStorage);
            }
          }
        }
      }

      void createAndPopulateLoopBBForStage(LoopDependenceInfo *LDI, StageInfo *stageInfo,
        unordered_map<Instruction *, Instruction *> &cloneMap,
        unordered_map<BasicBlock*, BasicBlock*> &bbCloneMap)
      {
        auto M = LDI->func->getParent();
        for (auto bbi = LDI->loop->block_begin(); bbi != LDI->loop->block_end(); ++bbi) {
          auto bb = *bbi;
          /*
           * FIX: Create basic blocks for induction variables and the SCC in question instead of all the loop's basic blocks
           */
          auto cloneBB = BasicBlock::Create(M->getContext(), bb->getName(), stageInfo->sccStage);
          IRBuilder<> builder(cloneBB);

          for (auto &I : *bb) {
            auto cloneIter = cloneMap.find(&I);
            if (cloneIter == cloneMap.end()) continue;
            cloneMap[&I] = builder.Insert(cloneIter->second);
          }

          bbCloneMap[bb] = cloneBB;
        }
      }

      void mapClonedOperands(unordered_map<Instruction *, Instruction *> cloneMap, unordered_map<BasicBlock*, BasicBlock*> bbCloneMap, BasicBlock *exitBB)
      {
        /*
         * IMPROVEMENT: Ignore special cases upfront. If a clone of a general case is not found, abort with a corresponding error 
         */
        for (auto ii = cloneMap.begin(); ii != cloneMap.end(); ++ii) {
          auto cloneInstruction = ii->second;

          /*
           * Replacing operands/basic block pointers of PHINode with clones
           */
          if (auto phiI = dyn_cast<PHINode>(cloneInstruction)) {
            for (auto &op : phiI->operands()) {

              /*
               * Handle replacing operands
               */
              if (auto opI = dyn_cast<Instruction>(op)) {
                auto iCloneIter = cloneMap.find(opI);
                if (iCloneIter != cloneMap.end()) {
                  op.set(cloneMap[opI]);
                }
                continue;
              }
              // Check for constants etc... abort
            }

            for (auto &bb : phiI->blocks()) {

              /*
               * Handle replacing basic blocks
               */
              auto basicBlockIndex = phiI->getBasicBlockIndex(bb);
              phiI->setIncomingBlock(basicBlockIndex, bbCloneMap[bb]);
            }
            continue;
          }

          for (auto &op : cloneInstruction->operands()) {
            auto opV = op.get();
            if (auto opI = dyn_cast<Instruction>(opV)) {
              auto iCloneIter = cloneMap.find(opI);
              if (iCloneIter != cloneMap.end()) {
                op.set(cloneMap[opI]);
              }
              continue;
            }
            if (auto opB = dyn_cast<BasicBlock>(opV)) {
              auto bbCloneIter = bbCloneMap.find(opB);
              if (bbCloneIter != bbCloneMap.end()) {
                op.set(bbCloneIter->second);
              } else {
                // Operand pointed to original exiting block
                op.set(exitBB);
              }
              continue;
            }
            // Add cases such as constants where no clone needs to exist. Abort with an error if no such type is found
          }
        }
      }

      void insertPipelineQueueing(StageInfo *stageInfo, unordered_map<Instruction *, Instruction *> &cloneMap,
        unordered_map<BasicBlock*, BasicBlock*> bbCloneMap,
        BasicBlock *headerBB, BasicBlock *exitBB)
      {
        /*
         * Insert queue pops + loads at start of loop body
         */
        Instruction *popBeforeInst;
        for (auto succToHeader : make_range(succ_begin(headerBB), succ_end(headerBB))) {
          if (exitBB != succToHeader) {
            popBeforeInst = &*succToHeader->begin();
          }
        }

        for (auto valuePopIter : stageInfo->valuePopQueuesMap) {
          auto valuePop = valuePopIter.second;
          valuePop->popQueueCall->moveBefore(popBeforeInst);
          valuePop->loadStorage->moveBefore(popBeforeInst);
        }

        /*
         * Insert queue pushes right after instruction that computes the pushed variable
         */
        for (auto valuePush : stageInfo->valuePushQueues) {
          auto valInst = valuePush->valueInstruction;
          auto valueBBIter = valInst->getParent()->end();
          while (&*valueBBIter != valInst) --valueBBIter;
          valuePush->pushQueueCall->moveBefore(&*(++valueBBIter));
        }
      }

      void createPipelineStageFromSCC(LoopDependenceInfo *LDI, StageInfo *stageInfo)
      {
        auto M = LDI->func->getParent();
        auto loop = LDI->loop;
        
        /*
         * ASSUMPTION: Function signature is: void (QueueType *, int *)
         *
         * SIMONE: you cannot use "static_cast" within LLVM. You need to use LLVM-specific casting: cast, dyn_cast, isa
         */
        auto pipelineStage = static_cast<Function *>(M->getOrInsertFunction("", stageType));
        stageInfo->sccStage = pipelineStage;
        auto argIter = pipelineStage->arg_begin();
        auto queueArg = &*argIter;
        auto resultArg = &*(++argIter);

        BasicBlock* entryBB = BasicBlock::Create(M->getContext(), "", pipelineStage);
        BasicBlock* exitBB = BasicBlock::Create(M->getContext(), "", pipelineStage);

        /*
         * Clone loop instructions in given SCC or non-loop-body 
         */
        unordered_map<Instruction *, Instruction *> cloneMap;
        cloneLoopInstForStage(LDI, stageInfo, cloneMap);

        /*
         * ASSUMPTION: Single variable computed & used outside of loop
         */
        IRBuilder<> entryBuilder(entryBB);
        IRBuilder<> exitBuilder(exitBB);
        auto outgoingDependency = cloneMap[stageInfo->outgoingDependentMap.begin()->first];
        // exitBuilder.CreateCall(printReachedIter, ArrayRef<Value*>({ static_cast<Value *>(outgoingDependency) }));
        exitBuilder.CreateStore(outgoingDependency, resultArg);
        exitBuilder.CreateRetVoid();

        createPipelineQueueing(stageInfo, static_cast<Value *>(queueArg), cloneMap, entryBuilder);

        /*
         * Clone loop basic blocks that are used by given SCC / non-loop-body basic blocks
         */
        unordered_map<BasicBlock*, BasicBlock*> bbCloneMap;
        bbCloneMap[loop->getLoopPreheader()] = entryBB;
        createAndPopulateLoopBBForStage(LDI, stageInfo, cloneMap, bbCloneMap);
        
        /*
         * Map and branch loop preheader to entry block
         */
        auto headerBB = bbCloneMap[loop->getHeader()];
        entryBuilder.CreateBr(headerBB);

        /*
         * Map clones' operands to cloned versions of those operands
         */
        mapClonedOperands(cloneMap, bbCloneMap, exitBB);
        insertPipelineQueueing(stageInfo, cloneMap, bbCloneMap, headerBB, exitBB);

        pipelineStage->print(errs() << "Function printout:\n"); errs() << "\n";
      }

      BasicBlock *createParallelizedFunctionExecution(LoopDependenceInfo *LDI, std::vector<StageInfo *> stages)
      {
        auto M = LDI->func->getParent();
        auto pipelineBB = BasicBlock::Create(M->getContext(), "", LDI->func);
        IRBuilder<> builder(pipelineBB);

        std::vector<Value *> dependentPntrs, dependentLoads, handlerArgs;
        
        for (auto i = 0; i < stages.size(); ++i)
        {
          dependentPntrs.push_back(builder.CreateAlloca(int32));
        }

        auto ptrIndex = 0;
        for (auto stage : stages)
        {
          handlerArgs.push_back(static_cast<Value *>(stage->sccStage));
          handlerArgs.push_back(static_cast<Value *>(dependentPntrs[ptrIndex++]));
        } 

        builder.CreateCall(stageHandler, ArrayRef<Value*>(handlerArgs));
        
        for (auto i = 0; i < stages.size(); ++i)
        {
          dependentLoads.push_back(builder.CreateLoad(dependentPntrs[i]));
        }
        
        /*
         * ASSUMPTION: Dependents are PHI Nodes, one dependent per stage
         */
        auto loadIndex = 0;
        for (auto stage : stages)
        {
          if (auto depPHI = dyn_cast<PHINode>(stage->outgoingDependentMap.begin()->second))
          {
            depPHI->addIncoming(dependentLoads[loadIndex++], pipelineBB);
            continue;
          }
          pipelineBB->eraseFromParent();
          return nullptr;
        }

        /*
         * ASSUMPTION: Only one unique exiting basic block from the loop
         */
        builder.CreateBr(LDI->loop->getExitBlock());
        return pipelineBB;
      }

      void linkParallelizedLoop(LoopDependenceInfo *LDI, BasicBlock *pipelineBB) {
        auto M = LDI->func->getParent();
        auto preheader = LDI->loop->getLoopPreheader();
        auto loopSwitch = BasicBlock::Create(M->getContext(), "", LDI->func, preheader);
        IRBuilder<> loopSwitchBuilder(loopSwitch);

        auto globalBool = new GlobalVariable(*M, int32, /*isConstant=*/ false, GlobalValue::ExternalLinkage, Constant::getNullValue(int32));
        auto const0 = ConstantInt::get(int32, APInt(32, 0, false));
        auto compareInstruction = loopSwitchBuilder.CreateICmpEQ(loopSwitchBuilder.CreateLoad(globalBool), const0);
        loopSwitchBuilder.CreateCondBr(compareInstruction, pipelineBB, preheader);
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
