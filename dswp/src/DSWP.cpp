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

      Function *stageHandler, *queuePushTemporary, *queuePopTemporary, *printReachedIter;
      FunctionType *stageType;
      Type *queueType;
      IntegerType *int8, *int32, *int64;

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
        int8 = IntegerType::get(M.getContext(), 8);
        int32 = IntegerType::get(M.getContext(), 32);
        int64 = IntegerType::get(M.getContext(), 64);
        queuePushTemporary = M.getFunction("queuePush");
        queuePopTemporary = M.getFunction("queuePop");
        stageHandler = M.getFunction("stageHandler");
        printReachedIter = M.getFunction("printReachedIter");

        auto stageFunction = M.getFunction("stageExecuter");
        auto stageArgType = stageFunction->arg_begin()->getType();
        stageType = cast<FunctionType>(cast<PointerType>(stageArgType)->getElementType());

        queueType = queuePushTemporary->arg_begin()->getType();

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
        std::vector<Instruction *> bodyInst, otherInst;
        for (auto bbi = loop->block_begin(); bbi != loop->block_end(); ++bbi)
        {
          BasicBlock *bb = *bbi;

          /*
           * Categorize branch, conditional, and induction variable instructions as 'other' instructions
           */
          for (auto ii = bb->begin(); ii != bb->end(); ++ii)
          {
            Instruction *i = &*ii;
            if (TerminatorInst::classof(i) || CmpInst::classof(i))
            {
              otherInst.push_back(i);
              continue;
            }
            bodyInst.push_back(i);
          }
        }

        return make_pair(bodyInst, otherInst);
      }

      SCCDG *extractLoopIterationSCCDG(LoopDependenceInfo *LDI)
      {
        auto loop = LDI->loop;
        auto sccSubgraph = LDI->loopBodySCCDG;

        Instruction *iterationInst = nullptr;
        auto headerBr = cast<BranchInst>(loop->getHeader()->getTerminator());
        for (auto &op : cast<Instruction>(headerBr->getCondition())->operands())
        {
          if (auto opI = dyn_cast<Instruction>(op))
          {
            iterationInst = opI;
            break;
          }
        }
        assert(iterationInst != nullptr);

        for (auto sccNode : make_range(sccSubgraph->begin_nodes(), sccSubgraph->end_nodes()))
        {
          if (sccNode->getT()->isInGraph(iterationInst)) return sccSubgraph->extractSCCIntoGraph(sccNode);
        }
        assert(1 == 2);
        return nullptr;
      }

      bool applyDSWP (LoopDependenceInfo *LDI)
      {
        std::vector<std::unique_ptr<StageInfo>> stages;

        printSCCs(LDI->loopBodySCCDG);

        /*
         * Extract loop SCC directly concerned with loop iteration
         */
        LDI->loopIterationSCCDG = extractLoopIterationSCCDG(LDI);

        printSCCs(LDI->loopIterationSCCDG);

        /*
         * Create the pipeline stages.
         */
        if (!locateNStageSCCLoop(LDI, stages)) return false;
        for (auto &stage : stages) createPipelineStageFromSCC(LDI, stage);

        /*
         * Create the switcher that will decide whether or not we will execute the parallelized loop.
         */
        auto pipelineBB = createTheLoopSwitcher(LDI, stages);
        if (pipelineBB == nullptr)
        {
          for (auto &stage : stages) stage->sccStage->eraseFromParent();
          return false;
        }

        /*
         * Link the parallelized loop within the original function that includes the sequential loop.
         */
        linkParallelizedLoop(LDI, pipelineBB);
        LDI->func->print(errs() << "Final function:\n"); errs() << "\n";

        return true;
      }

      bool collectLoopInternalDependents(LoopDependenceInfo *LDI, std::vector<std::unique_ptr<StageInfo>> &stages, unordered_map<SCC *, StageInfo *> &sccToStage)
      {
        LDI->internalDependentInstCount = 0;
        for (auto scc : make_range(LDI->loopBodySCCDG->begin_nodes(), LDI->loopBodySCCDG->end_nodes()))
        {
          for (auto sccEdge : make_range(scc->begin_outgoing_edges(), scc->end_outgoing_edges()))
          {
            auto sccPair = sccEdge->getNodePair();
            auto fromSCC = sccPair.first->getT();
            auto toSCC = sccPair.second->getT();

            for (auto instructionEdge : make_range(sccEdge->begin_sub_edges(), sccEdge->end_sub_edges()))
            {
              /*
               * ASSUMPTION 3: There aren't memory data dependences
               */
              if (instructionEdge->isMemoryDependence()) return false;

              auto fromStage = sccToStage[fromSCC];
              auto toStage = sccToStage[toSCC];

              fromStage->outgoingSCCEdges.push_back(instructionEdge);
              toStage->incomingSCCEdges.push_back(instructionEdge);
              fromStage->edgeToQueueMap[instructionEdge] = LDI->internalDependentInstCount;
              toStage->edgeToQueueMap[instructionEdge] = LDI->internalDependentInstCount++;
            }
          }
        }
        return true;
      }

      bool collectLoopExternalDependents(LoopDependenceInfo *LDI, std::vector<std::unique_ptr<StageInfo>> &stages)
      {
        LDI->externalDependentInstCount = 0;
        for (auto nodeI : LDI->loopDG->externalNodePairs())
        {
          auto externalNode = nodeI.second;

          auto addDependentStagesWith = [&](Instruction *internalInst) -> void {
            for (auto &stage : stages)
            {
              if (!stage->scc->isInternal(internalInst)) continue;
              auto externalInst = externalNode->getT();
              stage->externalDependencyToEnvMap[externalInst] = LDI->externalDependentInstCount++;
              stage->outgoingDependentMap[internalInst] = externalInst;
            }
          };

          /*
           * Check if loop-external instruction has incoming/outgoing nodes within one of the stages
           */
          for (auto incomingNode : make_range(externalNode->begin_incoming_nodes(), externalNode->end_incoming_nodes()))
          {
            addDependentStagesWith(incomingNode->getT());
          }
          for (auto outgoingNode : make_range(externalNode->begin_outgoing_nodes(), externalNode->end_outgoing_nodes()))
          {
            addDependentStagesWith(outgoingNode->getT());
          }
        }
        return true;
      }

      bool locateNStageSCCLoop(LoopDependenceInfo *LDI, std::vector<std::unique_ptr<StageInfo>> &stages)
      {
        auto loop = LDI->loop;
        auto sccSubgraph = LDI->loopBodySCCDG;

        if (!sccSubgraph->isPipeline()) return false;

        unordered_map<SCC *, StageInfo *> sccToStage;
        for (auto sccNode : make_range(sccSubgraph->begin_nodes(), sccSubgraph->end_nodes()))
        {
          auto scc = sccNode->getT();
          auto stage = std::make_unique<StageInfo>();
          stage->scc = scc;
          sccToStage[scc] = stage.get();
          stages.push_back(std::move(stage));
        }

        return collectLoopInternalDependents(LDI, stages, sccToStage) && collectLoopExternalDependents(LDI, stages);
      }

      void cloneLoopInstForStage(LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        auto &iCloneMap = *stageInfo->iCloneMap;
        for (auto sccI = stageInfo->scc->begin_internal_node_map(); sccI != stageInfo->scc->end_internal_node_map(); ++sccI)
        {
          auto I = sccI->first;
          iCloneMap[I] = I->clone();
        }

        auto loopIterationSCC = LDI->loopIterationSCCDG->getEntryNode()->getT();
        for (auto node : make_range(loopIterationSCC->begin_nodes(), loopIterationSCC->end_nodes()))
        {
          auto I = node->getT();
          iCloneMap[I] = I->clone();
        }

        /*
         * IMPROVEMENT: Do not copy every compare and branch present in the original loop
         */
        for (auto &I : LDI->otherInstOfLoop)
        {
          iCloneMap[I] = I->clone();
        }
      }

      void storeAndLoadExternalDependents(LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        IRBuilder<> entryBuilder(stageInfo->entryBlock);
        IRBuilder<> exitBuilder(stageInfo->exitBlock);
        auto envArg = &*(stageInfo->sccStage->arg_begin());
        auto arrayPtrType = PointerType::getUnqual(ArrayType::get(PointerType::getUnqual(int8), LDI->externalDependentInstCount));
        auto arrayIndexValue = cast<Value>(ConstantInt::get(int64, 0));
        auto envAlloca = entryBuilder.CreateBitCast(envArg, arrayPtrType);

        auto createEnvPtrFromDep = [&](Instruction *externalDependency, IRBuilder<> builder) -> Value * {
          auto envIndex = stageInfo->externalDependencyToEnvMap[externalDependency];
          auto envIndexValue = cast<Value>(ConstantInt::get(int64, envIndex));
          return builder.CreateInBoundsGEP(envAlloca, ArrayRef<Value*>({ arrayIndexValue, envIndexValue }));
        };

        for (auto dependencyPair : stageInfo->outgoingDependentMap)
        {
          auto envPtr = createEnvPtrFromDep(dependencyPair.second, exitBuilder);
          auto envVarPtr = exitBuilder.CreateLoad(envPtr);
          auto envVarCast = exitBuilder.CreateBitCast(envVarPtr, PointerType::getUnqual(int32));
          auto outgoingDependency = (*stageInfo->iCloneMap)[dependencyPair.first];
          exitBuilder.CreateStore(outgoingDependency, envVarCast);
        }

        for (auto dependencyPair : stageInfo->incomingDependentMap)
        {
          auto envPtr = createEnvPtrFromDep(dependencyPair.second, entryBuilder);
          auto envVarCast = entryBuilder.CreateBitCast(entryBuilder.CreateLoad(envPtr), PointerType::getUnqual(int32));
          auto incomingValue = entryBuilder.CreateLoad(envVarCast);

          Value *originalDepValue = cast<Value>(dependencyPair.first);
          for (auto &depOp : (*stageInfo->iCloneMap)[dependencyPair.first]->operands())
          {
            if (depOp != originalDepValue) continue;
            depOp.set(incomingValue);
          }
        }
      }

      void createPipelineQueueing(LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        IRBuilder<> entryBuilder(stageInfo->entryBlock);
        auto argIter = stageInfo->sccStage->arg_begin();
        auto queueArg = cast<Value>(&*(++argIter));
        
        auto queuesPtrType = PointerType::getUnqual(ArrayType::get(PointerType::getUnqual(int8), LDI->internalDependentInstCount));
        auto arrayIndexValue = cast<Value>(ConstantInt::get(int64, 0));
        auto queuesArray = entryBuilder.CreateBitCast(queueArg, queuesPtrType);

        auto getQueuePtrFromEdge = [&](DGEdge<Instruction> *edge) -> Value * {
          auto queueIndex = stageInfo->edgeToQueueMap[edge];
          auto queueIndexValue = cast<Value>(ConstantInt::get(int64, queueIndex));
          auto queuePtr = entryBuilder.CreateInBoundsGEP(queuesArray, ArrayRef<Value*>({ arrayIndexValue, queueIndexValue }));
          auto queueCast = entryBuilder.CreateBitCast(queuePtr, PointerType::getUnqual(queueType));
          return entryBuilder.CreateLoad(queueCast);
        };

        /*
         * Locate clone of outgoing instruction, create queue push call
         */
        for (auto edge : stageInfo->outgoingSCCEdges)
        {
          auto outgoingI = edge->getNodePair().first->getT();
          auto outgoingClone = (*stageInfo->iCloneMap)[outgoingI];
          auto queueCallArgs = ArrayRef<Value*>({ getQueuePtrFromEdge(edge), cast<Value>(outgoingClone) });

          auto valuePush = new OutgoingPipelineInfo();
          stageInfo->valuePushQueues.push_back(valuePush);
          valuePush->valueInstruction = outgoingClone;
          valuePush->pushQueueCall = entryBuilder.CreateCall(queuePushTemporary, queueCallArgs);
        }

        /*
         * Create a queue pop and load for each unique dependent in previous SCCs
         */
        for (auto edge : stageInfo->incomingSCCEdges)
        {
          auto outgoingI = edge->getNodePair().first->getT();
          if (stageInfo->valuePopQueuesMap.find(outgoingI) != stageInfo->valuePopQueuesMap.end()) continue;

          auto valuePop = new IncomingPipelineInfo();
          stageInfo->valuePopQueuesMap[outgoingI] = valuePop;
          valuePop->popStorage = entryBuilder.CreateAlloca(int32);
          valuePop->loadStorage = entryBuilder.CreateLoad(valuePop->popStorage);
          auto queueCallArgs = ArrayRef<Value*>({ getQueuePtrFromEdge(edge), cast<Value>(valuePop->popStorage) });
          valuePop->popQueueCall = entryBuilder.CreateCall(queuePopTemporary, queueCallArgs);
        }

        /*
         * Replace use of dependents in previous SCCs with the appropriate queue pop and load 
         */
        for (auto edge : stageInfo->incomingSCCEdges)
        {
          auto nodePair = edge->getNodePair();
          auto outgoingI = nodePair.first->getT();
          auto incomingI = nodePair.second->getT();
          auto userInstruction = (*stageInfo->iCloneMap)[incomingI];
          auto valuePop = stageInfo->valuePopQueuesMap[outgoingI];
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

      void createAndPopulateLoopBBForStage(LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        auto M = LDI->func->getParent();
        auto &iCloneMap = *stageInfo->iCloneMap;
        auto &bbCloneMap = *stageInfo->bbCloneMap;

        /*
         * Assign stage entry block as "clone" of loop header
         */
        bbCloneMap[LDI->loop->getLoopPreheader()] = stageInfo->entryBlock;
        
        for (auto bbi = LDI->loop->block_begin(); bbi != LDI->loop->block_end(); ++bbi) {
          auto bb = *bbi;
          /*
           * FIX: Create basic blocks for induction variables and the SCC in question instead of all the loop's basic blocks
           */
          auto cloneBB = BasicBlock::Create(M->getContext(), "", stageInfo->sccStage);
          IRBuilder<> builder(cloneBB);

          for (auto &I : *bb) {
            auto cloneIter = iCloneMap.find(&I);
            if (cloneIter == iCloneMap.end()) continue;
            iCloneMap[&I] = builder.Insert(cloneIter->second);
          }

          bbCloneMap[bb] = cloneBB;
        }
      }

      void mapClonedOperands(LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        auto &iCloneMap = *stageInfo->iCloneMap;
        auto &bbCloneMap = *stageInfo->bbCloneMap;
        
        IRBuilder<> entryBuilder(stageInfo->entryBlock);
        entryBuilder.CreateBr(bbCloneMap[LDI->loop->getHeader()]);

        /*
         * IMPROVEMENT: Ignore special cases upfront. If a clone of a general case is not found, abort with a corresponding error 
         */
        for (auto ii = iCloneMap.begin(); ii != iCloneMap.end(); ++ii) {
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
                auto iCloneIter = iCloneMap.find(opI);
                if (iCloneIter != iCloneMap.end()) {
                  op.set(iCloneMap[opI]);
                }
                continue;
              }
              // Check for constants etc... abort
            }

            for (auto &bb : phiI->blocks()) {

              /*
               * Handle replacing basic blocks
               */
              phiI->setIncomingBlock(phiI->getBasicBlockIndex(bb), bbCloneMap[bb]);
            }
            continue;
          }

          for (auto &op : cloneInstruction->operands()) {
            auto opV = op.get();
            if (auto opI = dyn_cast<Instruction>(opV)) {
              auto iCloneIter = iCloneMap.find(opI);
              if (iCloneIter != iCloneMap.end()) {
                op.set(iCloneMap[opI]);
              }
              continue;
            }
            if (auto opB = dyn_cast<BasicBlock>(opV)) {
              auto bbCloneIter = bbCloneMap.find(opB);
              if (bbCloneIter != bbCloneMap.end()) {
                op.set(bbCloneIter->second);
              } else {
                // Operand pointed to original exiting block
                op.set(stageInfo->exitBlock);
              }
              continue;
            }
            // Add cases such as constants where no clone needs to exist. Abort with an error if no such type is found
          }
        }
      }

      void insertPipelineQueueing(LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        /*
         * Insert queue pops + loads at start of loop body
         */
        Instruction *popBeforeInst;
        auto headerBB = (*stageInfo->bbCloneMap)[LDI->loop->getHeader()];
        for (auto succToHeader : make_range(succ_begin(headerBB), succ_end(headerBB))) {
          if (stageInfo->exitBlock != succToHeader) {
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

      void createPipelineStageFromSCC(LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        auto M = LDI->func->getParent();
        
        stageInfo->sccStage = cast<Function>(M->getOrInsertFunction("", stageType));
        stageInfo->entryBlock = BasicBlock::Create(M->getContext(), "", stageInfo->sccStage);
        stageInfo->exitBlock = BasicBlock::Create(M->getContext(), "", stageInfo->sccStage);

        /*
         * Clone loop instructions in given SCC or non-loop-body 
         */
        unordered_map<Instruction *, Instruction *> cloneMap;
        stageInfo->iCloneMap = &cloneMap;

        cloneLoopInstForStage(LDI, stageInfo);
        storeAndLoadExternalDependents(LDI, stageInfo);
        createPipelineQueueing(LDI, stageInfo);

        /*
         * Clone loop basic blocks that are used by given SCC / non-loop-body basic blocks
         */
        unordered_map<BasicBlock*, BasicBlock*> bbCloneMap;
        stageInfo->bbCloneMap = &bbCloneMap;

        createAndPopulateLoopBBForStage(LDI, stageInfo);
        
        /*
         * Map clones' operands to cloned versions of those operands
         */
        mapClonedOperands(LDI, stageInfo);
        insertPipelineQueueing(LDI, stageInfo);

        IRBuilder<> exitBuilder(stageInfo->exitBlock);
        // exitBuilder.CreateCall(printReachedIter, ArrayRef<Value*>({ cast<Value>(outgoingDependency) }));
        exitBuilder.CreateRetVoid();
        stageInfo->sccStage->print(errs() << "Function printout:\n"); errs() << "\n";
      }

      BasicBlock * createTheLoopSwitcher (LoopDependenceInfo *LDI, std::vector<std::unique_ptr<StageInfo>> &stages)
      {
        auto M = LDI->func->getParent();
        auto pipelineBB = BasicBlock::Create(M->getContext(), "", LDI->func);
        IRBuilder<> builder(pipelineBB);

        auto baseArrayIndex = cast<Value>(ConstantInt::get(int64, 0));

        /*
         * Create empty environment array
         * ASSUMPTION: All the types of environment variables are int32
         */
        auto envArrayType = ArrayType::get(PointerType::getUnqual(int8), LDI->externalDependentInstCount);
        auto envAlloca = cast<Value>(builder.CreateAlloca(envArrayType));
        std::vector<Value*> envVarPtrs;
        for (int i = 0; i < LDI->externalDependentInstCount; ++i)
        {
          auto envVarPtr = builder.CreateAlloca(int32);
          envVarPtrs.push_back(envVarPtr);
          auto envIndex = cast<Value>(ConstantInt::get(int64, i));
          auto depInEnvPtr = builder.CreateInBoundsGEP(envAlloca, ArrayRef<Value*>({ baseArrayIndex, envIndex }));
          auto depCast = builder.CreateBitCast(depInEnvPtr, PointerType::getUnqual(PointerType::getUnqual(int32)));
          builder.CreateStore(envVarPtr, depCast);
        }

        /*
         * Insert incoming dependents for stages into the environment array
         */
        for (auto &stage : stages)
        {
          for (auto dependencyPair : stage->incomingDependentMap)
          {
            auto externalDependency = dependencyPair.second;
            auto envIndex = stage->externalDependencyToEnvMap[externalDependency];
            builder.CreateStore(externalDependency, envVarPtrs[envIndex]);
          }
        }

        /*
         * Pass the environment and stage function pointers to the stage handler
         */
        auto stagesArrayType = ArrayType::get(PointerType::getUnqual(int8), stages.size());
        auto stagesAlloca = cast<Value>(builder.CreateAlloca(stagesArrayType));
        auto stagePtrType = PointerType::getUnqual(stages[0]->sccStage->getType());
        for (int i = 0; i < stages.size(); ++i)
        {
          auto &stage = stages[i];
          auto stageIndex = cast<Value>(ConstantInt::get(int64, i));
          auto stagePtr = builder.CreateInBoundsGEP(stagesAlloca, ArrayRef<Value*>({ baseArrayIndex, stageIndex }));
          auto stageCast = builder.CreateBitCast(stagePtr, stagePtrType);
          builder.CreateStore(stage->sccStage, stageCast);
        }

        /*
         * Create empty queues array to be used by stages
         * ASSUMPTION: All the queues hold int32 variables
         */
        auto queuesArrayType = ArrayType::get(PointerType::getUnqual(int8), LDI->internalDependentInstCount);
        auto queuesAlloca = cast<Value>(builder.CreateAlloca(queuesArrayType));
        auto queuesPtr = cast<Value>(builder.CreateBitCast(queuesAlloca, PointerType::getUnqual(int8)));
        auto queuesCount = cast<Value>(ConstantInt::get(int32, LDI->internalDependentInstCount));

        /*
         * Call the stage handler with the environment, queues array, and stages array
         */
        auto envPtr = cast<Value>(builder.CreateBitCast(envAlloca, PointerType::getUnqual(int8)));
        auto stagesPtr = cast<Value>(builder.CreateBitCast(stagesAlloca, PointerType::getUnqual(int8)));
        auto stagesCount = cast<Value>(ConstantInt::get(int32, stages.size()));
        builder.CreateCall(stageHandler, ArrayRef<Value*>({ envPtr, queuesPtr, stagesPtr, stagesCount, queuesCount }));

        /*
         * Extract the outgoing dependents for each stage
         */
        for (auto &stage : stages)
        {
          for (auto dependencyPair : stage->outgoingDependentMap)
          {
            auto externalDependency = dependencyPair.second;
            auto envIndex = cast<Value>(ConstantInt::get(int64, stage->externalDependencyToEnvMap[externalDependency]));
            auto depInEnvPtr = builder.CreateInBoundsGEP(envAlloca, ArrayRef<Value*>({ baseArrayIndex, envIndex }));
            auto envVarCast = builder.CreateBitCast(builder.CreateLoad(depInEnvPtr), PointerType::getUnqual(int32));
            auto envVar = builder.CreateLoad(envVarCast);

            /*
             * ASSUMPTION: Dependents are PHI Nodes
             */
            if (auto depPHI = dyn_cast<PHINode>(externalDependency))
            {
              depPHI->addIncoming(envVar, pipelineBB);
              continue;
            }
            pipelineBB->eraseFromParent();
            return nullptr;
          }
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
        errs() << "\nExternal SCCs\n";
        for (auto sccI = sccSubgraph->begin_external_node_map(); sccI != sccSubgraph->end_external_node_map(); ++sccI) {
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
