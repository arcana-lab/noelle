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

#include "LoopDependenceInfo.hpp"
#include "PipelineInfo.hpp"
#include "PDG.hpp"
#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "PDGAnalysis.hpp"

#include <unordered_map>

using namespace llvm;

namespace llvm {

  struct DSWP : public ModulePass {
    public:
      static char ID;

      Function *stageDispatcher, *queuePushTemporary, *queuePopTemporary;
      FunctionType *stageType;
      Type *queueType;
      IntegerType *int8, *int32, *int64;

      DSWP() : ModulePass{ID} {}

      bool doInitialization (Module &M) override { return false; }

      bool runOnModule (Module &M) override
      {
        errs() << "DSWP for " << M.getName() << "\n";
        if (!collectThreadPoolHelperFunctionsAndTypes(M))
        {
          errs() << "DSWP utils not included!\n";
          return false;
        }

        auto graph = getAnalysis<PDGAnalysis>().getPDG();

        auto modified = false;
        std::vector<Function *> funcToModify;
        for (auto &F : M)
        {
          if (F.empty()) continue;
          if (&F == stageDispatcher || &F == queuePushTemporary || &F == queuePopTemporary) continue;
          funcToModify.push_back(&F);
        }
        for (auto F : funcToModify)
        {
          auto loopDI = fetchLoopToParallelize(*F, graph);
          if (loopDI == nullptr) return false;

          modified |= applyDSWP(loopDI);
          delete loopDI;
        }
        return modified;
      }

      bool collectThreadPoolHelperFunctionsAndTypes (Module &M)
      {
        int8 = IntegerType::get(M.getContext(), 8);
        int32 = IntegerType::get(M.getContext(), 32);
        int64 = IntegerType::get(M.getContext(), 64);

        queuePushTemporary = M.getFunction("queuePush");
        queuePopTemporary = M.getFunction("queuePop");
        if (queuePushTemporary == nullptr || queuePopTemporary == nullptr) return false;
        queueType = queuePushTemporary->arg_begin()->getType();

        /*
         * Signature: void stageDispatcher(void *env, void *queues, void *stages, int numberOfStages, int numberOfQueues)
         * Method: void stageExecuter(void (*stage)(void *, void *), void *env, void *queues) { return stage(env, queues); }
         */
        stageDispatcher = M.getFunction("stageDispatcher");
        auto stageExecuter = M.getFunction("stageExecuter");
        if (stageDispatcher == nullptr || stageExecuter == nullptr) return false;

        auto stageArgType = stageExecuter->arg_begin()->getType();
        stageType = cast<FunctionType>(cast<PointerType>(stageArgType)->getElementType());
        return true;
      }

      void getAnalysisUsage (AnalysisUsage &AU) const override
      {
        AU.addRequired<PDGAnalysis>();
        AU.addRequired<AssumptionCacheTracker>();
        AU.addRequired<DominatorTreeWrapperPass>();
        AU.addRequired<LoopInfoWrapperPass>();
        AU.addRequired<ScalarEvolutionWrapperPass>();
        return ;
      }

    private:
      LoopDependenceInfo *fetchLoopToParallelize (Function &function, PDG *graph)
      {
        /*
         * Fetch the loops.
         */
        auto &LI = getAnalysis<LoopInfoWrapperPass>(function).getLoopInfo();
        auto &DT = getAnalysis<DominatorTreeWrapperPass>(function).getDomTree();
        auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(function).getSE();

        auto funcPDG = graph->createFunctionSubgraph(function);

        /*
         * ASSUMPTION 1: One loop in the entire function 
         * Choose the loop to parallelize.
         */
        for (auto loopIter : LI)
        {
          auto loop = &*loopIter;
          auto instPair = divideLoopInstructions(loop);
          return new LoopDependenceInfo(&function, LI, DT, SE, loop, funcPDG, instPair.first, instPair.second);
        }

        return nullptr;
      }

      std::pair<std::vector<Instruction *>, std::vector<Instruction *>> divideLoopInstructions (Loop *loop)
      {
        std::vector<Instruction *> bodyInst, otherInst;
        for (auto bbi = loop->block_begin(); bbi != loop->block_end(); ++bbi)
        {
          BasicBlock *bb = *bbi;

          /*
           * Categorize branch and conditional variable instructions as 'other' instructions
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

      SCCDAG *extractLoopIterationSCCDAG (LoopDependenceInfo *LDI)
      {
        auto loop = LDI->loop;
        auto sccSubgraph = LDI->loopBodySCCDAG;

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
          if (sccNode->getT()->isInGraph(iterationInst))
          {
            return sccSubgraph->extractSCCIntoGraph(sccNode);
          }
        }
        assert(1 == 2);
        return nullptr;
      }

      bool applyDSWP (LoopDependenceInfo *LDI)
      {

        /*
         * Compute the SCCDAG of the loop given as input.
         */
        LDI->loopIterationSCCDAG = extractLoopIterationSCCDAG(LDI);
        printSCCs(LDI->loopBodySCCDAG);
        printSCCs(LDI->loopIterationSCCDAG);

        /*
         * Merge SCCs of the SCCDAG.
         */
        mergeSCCs(LDI);

        /*
         * Create the pipeline stages.
         */
        std::vector<std::unique_ptr<StageInfo>> stages;
        if (!isWorthParallelizing(LDI, stages)) return false;
        for (auto &stage : stages) createPipelineStageFromSCC(LDI, stage);

        /*
         * Create the pipeline (connecting the stages)
         */
        createPipelineFromStages(LDI, stages);
        if (LDI->pipelineBB == nullptr)
        {
          for (auto &stage : stages) stage->sccStage->eraseFromParent();
          return false;
        }

        /*
         * Link the parallelized loop within the original function that includes the sequential loop.
         */
        linkParallelizedLoopToOriginalFunction(LDI);

        return true;
      }

      void mergeSCCs (LoopDependenceInfo *LDI){

        /*
         * Merge the SCC related to a single PHI node and its use if there is only one.
         */
        //TODO

        return ;
      }

      bool collectLoopInternalDependents (LoopDependenceInfo *LDI, std::vector<std::unique_ptr<StageInfo>> &stages, unordered_map<SCC *, StageInfo *> &sccToStage)
      {
        for (auto scc : make_range(LDI->loopBodySCCDAG->begin_nodes(), LDI->loopBodySCCDAG->end_nodes()))
        {
          for (auto sccEdge : make_range(scc->begin_outgoing_edges(), scc->end_outgoing_edges()))
          {
            auto sccPair = sccEdge->getNodePair();
            auto fromSCC = sccPair.first->getT();
            auto toSCC = sccPair.second->getT();

            for (auto instructionEdge : make_range(sccEdge->begin_sub_edges(), sccEdge->end_sub_edges()))
            {
              /*
               * ASSUMPTION: There aren't memory data dependences
               */
              if (instructionEdge->isMemoryDependence()) return false;

              errs() << "Adding internal dependency:\n";
              instructionEdge->print(errs()); errs() << "\n";

              auto fromStage = sccToStage[fromSCC];
              auto toStage = sccToStage[toSCC];

              fromStage->outgoingSCCEdges.push_back(instructionEdge);
              toStage->incomingSCCEdges.push_back(instructionEdge);

              auto index = LDI->internalDependentByteLengths.size();
              fromStage->edgeToQueueMap[instructionEdge] = index;
              toStage->edgeToQueueMap[instructionEdge] = index;
              
              auto dependentInstType = instructionEdge->getNodePair().first->getT()->getType();
              LDI->internalDependentTypes.push_back(dependentInstType);

              auto bitSize = dependentInstType->getPrimitiveSizeInBits();
              if (bitSize % 8 != 0) return false;
              LDI->internalDependentByteLengths.push_back(bitSize / 8);
            }
          }
        }
        return true;
      }

      bool collectLoopExternalDependents (LoopDependenceInfo *LDI, std::vector<std::unique_ptr<StageInfo>> &stages)
      {
        for (auto nodeI : LDI->loopDG->externalNodePairs())
        {
          auto externalNode = nodeI.second;
          auto externalValue = externalNode->getT();
          auto addDependentStagesWith = [&](Instruction *internalInst, bool outgoing, Type *dependencyType) -> void {
            for (auto &stage : stages)
            {
              if (!stage->scc->isInternal(cast<Value>(internalInst))) continue;
              errs() << "Adding external dependency:\n";
              externalValue->print(errs() << "External val:\t"); errs() << "\n";
              internalInst->print(errs() << "Internal val:\t"); errs() << "\n";
              stage->externalDependencyToEnvMap[externalValue] = LDI->externalDependentTypes.size();
              auto &map = outgoing ? stage->outgoingDependentMap : stage->incomingDependentMap;
              map[internalInst] = externalValue;

              LDI->externalDependentTypes.push_back(dependencyType);
            }
          };

          /*
           * Check if loop-external instruction has incoming/outgoing nodes within one of the stages
           */
          for (auto incomingNode : make_range(externalNode->begin_incoming_nodes(), externalNode->end_incoming_nodes()))
          {
            addDependentStagesWith(cast<Instruction>(incomingNode->getT()), true, incomingNode->getT()->getType());
          }
          for (auto outgoingNode : make_range(externalNode->begin_outgoing_nodes(), externalNode->end_outgoing_nodes()))
          {
            addDependentStagesWith(cast<Instruction>(outgoingNode->getT()), false, externalNode->getT()->getType());
          }
        }
        return true;
      }

      bool configureLoopDependentStorage (LoopDependenceInfo *LDI, std::vector<std::unique_ptr<StageInfo>> &stages)
      {
        LDI->zeroIndexForBaseArray = cast<Value>(ConstantInt::get(int64, 0));
        LDI->envArrayType = ArrayType::get(PointerType::getUnqual(int8), LDI->externalDependentTypes.size());
        LDI->queueArrayType = ArrayType::get(PointerType::getUnqual(int8), LDI->internalDependentTypes.size());
        LDI->stageArrayType = ArrayType::get(PointerType::getUnqual(int8), stages.size());
        return true;
      }

      bool isWorthParallelizing (LoopDependenceInfo *LDI, std::vector<std::unique_ptr<StageInfo>> &stages)
      {
        auto loop = LDI->loop;
        auto sccSubgraph = LDI->loopBodySCCDAG;

        if (sccSubgraph->numNodes() <= 1) return false;

        unordered_map<SCC *, StageInfo *> sccToStage;
        for (auto sccNode : make_range(sccSubgraph->begin_nodes(), sccSubgraph->end_nodes()))
        {
          auto scc = sccNode->getT();
          auto stage = std::make_unique<StageInfo>();
          stage->scc = scc;
          sccToStage[scc] = stage.get();
          stages.push_back(std::move(stage));
        }

        return collectLoopInternalDependents(LDI, stages, sccToStage)
          && collectLoopExternalDependents(LDI, stages)
          && configureLoopDependentStorage(LDI, stages);
      }

      void cloneLoopInstWithinStage (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        auto &iCloneMap = *stageInfo->iCloneMap;
        for (auto sccI = stageInfo->scc->begin_internal_node_map(); sccI != stageInfo->scc->end_internal_node_map(); ++sccI)
        {
          auto I = cast<Instruction>(sccI->first);
          iCloneMap[I] = I->clone();
        }

        auto loopIterationSCC = LDI->loopIterationSCCDAG->getEntryNode()->getT();
        for (auto node : make_range(loopIterationSCC->begin_nodes(), loopIterationSCC->end_nodes()))
        {
          auto I = cast<Instruction>(node->getT());
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

      /*
       * OPTIMIZATION: Have only one location in environment for every unique incoming dependency
       */
      void linkStageWithEnvironment (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        IRBuilder<> entryBuilder(stageInfo->entryBlock);
        IRBuilder<> exitBuilder(stageInfo->exitBlock);
        auto envAlloca = entryBuilder.CreateBitCast(&*(stageInfo->sccStage->arg_begin()), PointerType::getUnqual(LDI->envArrayType));

        auto accessEnvVarFromDep = [&](Value *externalDependency, IRBuilder<> builder) -> Value * {
          auto envIndex = stageInfo->externalDependencyToEnvMap[externalDependency];
          auto envIndexValue = cast<Value>(ConstantInt::get(int64, envIndex));
          auto envPtr = builder.CreateInBoundsGEP(envAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndexValue }));
          auto envType = LDI->externalDependentTypes[envIndex];
          return builder.CreateBitCast(builder.CreateLoad(envPtr), PointerType::getUnqual(envType));
        };

        /*
         * Store (SCC -> outside of loop) dependencies within the environment array
         */
        for (auto dependencyPair : stageInfo->outgoingDependentMap)
        {
          auto envVar = accessEnvVarFromDep(dependencyPair.second, exitBuilder);
          auto outgoingDependency = (*stageInfo->iCloneMap)[dependencyPair.first];
          exitBuilder.CreateStore(outgoingDependency, envVar);
        }

        /*
         * Load (outside of loop -> SCC) dependencies from the environment array 
         */
        for (auto dependencyPair : stageInfo->incomingDependentMap)
        {
          auto envVar = accessEnvVarFromDep(dependencyPair.second, entryBuilder);
          auto incomingValue = entryBuilder.CreateLoad(envVar);

          Value *originalDepValue = cast<Value>(dependencyPair.first);
          auto cloneOfDepInst = (*stageInfo->iCloneMap)[dependencyPair.first];
          errs() << "Replacing external dependency operands of:\n";
          cloneOfDepInst->print(errs() << "\t"); errs() << "\n";
          for (auto &depOp : cloneOfDepInst->operands())
          {
            depOp->print(errs() << "\t Op:"); errs() << "\n";
            if (depOp != originalDepValue) continue;
            depOp.set(incomingValue);
          }
        }
      }

      void createQueuePushesAndPops (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        IRBuilder<> entryBuilder(stageInfo->entryBlock);
        auto argIter = stageInfo->sccStage->arg_begin();
        auto queuesArray = entryBuilder.CreateBitCast(&*(++argIter), PointerType::getUnqual(LDI->queueArrayType));

        auto getQueuePtrFromEdge = [&](DGEdge<Value> *edge) -> Value * {
          auto queueIndex = stageInfo->edgeToQueueMap[edge];
          auto queueIndexValue = cast<Value>(ConstantInt::get(int64, queueIndex));
          auto queuePtr = entryBuilder.CreateInBoundsGEP(queuesArray, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, queueIndexValue }));
          auto queueCast = entryBuilder.CreateBitCast(queuePtr, PointerType::getUnqual(queueType));
          return entryBuilder.CreateLoad(queueCast);
        };

        /*
         * Locate clone of outgoing instruction, create queue push call
         */
        for (auto edge : stageInfo->outgoingSCCEdges)
        {
          auto outgoingI = cast<Instruction>(edge->getNodePair().first->getT());
          auto outgoingClone = (*stageInfo->iCloneMap)[outgoingI];
          auto queueIndex = stageInfo->edgeToQueueMap[edge];
          auto byteLength = cast<Value>(ConstantInt::get(int64, LDI->internalDependentByteLengths[queueIndex]));
          auto outgoingType = LDI->internalDependentTypes[queueIndex];

          auto outgoingPtr = entryBuilder.CreateAlloca(outgoingType);
          auto outgoingStore = entryBuilder.CreateStore(outgoingClone, outgoingPtr);
          auto outgoingCast = cast<Value>(entryBuilder.CreateBitCast(outgoingPtr, PointerType::getUnqual(int8)));
          auto queueCallArgs = ArrayRef<Value*>({ getQueuePtrFromEdge(edge), outgoingCast, byteLength });

          auto valuePush = std::make_unique<OutgoingPipelineInfo>();
          valuePush->valueInstruction = outgoingClone;
          valuePush->valueIntoPtrStore = outgoingStore;
          valuePush->ptrToValueCast = outgoingCast;
          valuePush->pushQueueCall = entryBuilder.CreateCall(queuePushTemporary, queueCallArgs);
          stageInfo->valuePushQueues.push_back(std::move(valuePush));
        }

        /*
         * Create a queue pop and load for each unique dependent in previous SCCs
         */
        for (auto edge : stageInfo->incomingSCCEdges)
        {
          auto outgoingI = cast<Instruction>(edge->getNodePair().first->getT());

          /*
           * Only create one pop per incoming dependency. Multiple instructions can access the value of this pop
           */
          if (stageInfo->valuePopQueuesMap.find(outgoingI) != stageInfo->valuePopQueuesMap.end()) continue;

          auto valuePop = std::make_unique<IncomingPipelineInfo>();
          auto queueIndex = stageInfo->edgeToQueueMap[edge];
          auto byteLength = cast<Value>(ConstantInt::get(int64, LDI->internalDependentByteLengths[queueIndex]));
          auto outgoingType = LDI->internalDependentTypes[queueIndex];

          valuePop->popStorage = entryBuilder.CreateAlloca(outgoingType);
          valuePop->popCast = cast<Value>(entryBuilder.CreateBitCast(valuePop->popStorage, PointerType::getUnqual(int8)));
          auto queueCallArgs = ArrayRef<Value*>({ getQueuePtrFromEdge(edge), valuePop->popCast, byteLength });
          valuePop->popQueueCall = entryBuilder.CreateCall(queuePopTemporary, queueCallArgs);
          valuePop->loadStorage = entryBuilder.CreateLoad(valuePop->popStorage);
          stageInfo->valuePopQueuesMap[outgoingI] = std::move(valuePop);
        }
      }

      void createAndPopulateStageBB (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        auto M = LDI->func->getParent();
        auto &iCloneMap = *stageInfo->iCloneMap;
        auto &bbCloneMap = *stageInfo->bbCloneMap;

        /*
         * Assign stage entry block as "clone" of loop header
         */
        bbCloneMap[LDI->loop->getLoopPreheader()] = stageInfo->entryBlock;

        for (auto bbi = LDI->loop->block_begin(); bbi != LDI->loop->block_end(); ++bbi)
        {
          auto bb = *bbi;

          /*
           * FIX: Create basic blocks for induction variables and the SCC in question instead of all the loop's basic blocks
           */
          auto cloneBB = BasicBlock::Create(M->getContext(), "", stageInfo->sccStage);
          IRBuilder<> builder(cloneBB);

          for (auto &I : *bb)
          {
            auto cloneIter = iCloneMap.find(&I);
            if (cloneIter == iCloneMap.end()) continue;
            iCloneMap[&I] = builder.Insert(cloneIter->second);
          }

          bbCloneMap[bb] = cloneBB;
        }
      }

      // Decouple mapping instruction operands and linking basic blocks into two functions (Link first, then remap operands)
      void remapOperandsOfClones (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
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

      void replaceDependentUsesWithQueuePops (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        /*
         * Replace use of dependents in previous SCCs with the appropriate queue pop and load 
         */
        for (auto edge : stageInfo->incomingSCCEdges)
        {
          auto nodePair = edge->getNodePair();
          auto outgoingI = cast<Instruction>(nodePair.first->getT());
          auto incomingI = cast<Instruction>(nodePair.second->getT());
          auto userInstruction = (*stageInfo->iCloneMap)[incomingI];
          auto &valuePop = stageInfo->valuePopQueuesMap[outgoingI];
          valuePop->userInstructions.push_back(userInstruction);

          /*
           * If mem, do nothing because queue pop took care of synchronization with previous stage. There is no need to pop a value; it's loaded from mem 
           */
          if (edge->isMemoryDependence()) continue;

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

      void scheduleQueuePushAndPops (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        /*
         * Insert queue pops + loads at start of loop body
         */
        
        // FIX: Use control on userInstructions of queue to properly place the pop
        auto headerCloneBB = (*stageInfo->bbCloneMap)[LDI->loop->getHeader()];
        for (auto succBB : make_range(succ_begin(headerCloneBB), succ_end(headerCloneBB)))
        {
          if (succBB == stageInfo->exitBlock) continue;
          for (auto &popBeforeI : *succBB)
          {
            if (auto phi = dyn_cast<PHINode>(&popBeforeI)) continue;
            for (auto &valuePopIter : stageInfo->valuePopQueuesMap)
            {
              auto &valuePop = valuePopIter.second;
              cast<Instruction>(valuePop->popCast)->moveBefore(&popBeforeI);
              valuePop->popQueueCall->moveBefore(&popBeforeI);
              valuePop->loadStorage->moveBefore(&popBeforeI);
            }
            break;
          }
          break;
        }

        /*
         * Insert queue pushes right after the instruction which computes the variable to be pushed
         */
        for (auto &valuePush : stageInfo->valuePushQueues)
        {
          auto valInst = valuePush->valueInstruction;
          auto valueBBIter = valInst->getParent()->end();
          while (&*valueBBIter != valInst) --valueBBIter;
          auto afterValInst = &*(++valueBBIter);
          cast<Instruction>(valuePush->valueIntoPtrStore)->moveBefore(afterValInst);
          cast<Instruction>(valuePush->ptrToValueCast)->moveBefore(afterValInst);
          valuePush->pushQueueCall->moveBefore(afterValInst);
        }
      }

      void createPipelineStageFromSCC (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
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

        cloneLoopInstWithinStage(LDI, stageInfo);
        linkStageWithEnvironment(LDI, stageInfo);
        createQueuePushesAndPops(LDI, stageInfo);

        /*
         * Clone loop basic blocks that are used by given SCC / non-loop-body basic blocks
         */
        unordered_map<BasicBlock*, BasicBlock*> bbCloneMap;
        stageInfo->bbCloneMap = &bbCloneMap;

        createAndPopulateStageBB(LDI, stageInfo);
        remapOperandsOfClones(LDI, stageInfo);
        replaceDependentUsesWithQueuePops(LDI, stageInfo);
        scheduleQueuePushAndPops(LDI, stageInfo);

        IRBuilder<> exitBuilder(stageInfo->exitBlock);
        exitBuilder.CreateRetVoid();
        stageInfo->sccStage->print(errs() << "Function printout:\n"); errs() << "\n";
      }

      Value * createEnvArrayFromStages (LoopDependenceInfo *LDI, std::vector<std::unique_ptr<StageInfo>> &stages, IRBuilder<> builder, Value *envAlloca)
      {
        /*
         * Create empty environment array
         */
        std::vector<Value*> envVarPtrs;
        for (int i = 0; i < LDI->externalDependentTypes.size(); ++i)
        {
          Type *envType = LDI->externalDependentTypes[i];
          auto envVarPtr = builder.CreateAlloca(envType);
          envVarPtrs.push_back(envVarPtr);
          auto envIndex = cast<Value>(ConstantInt::get(int64, i));
          auto depInEnvPtr = builder.CreateInBoundsGEP(envAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndex }));
          auto depCast = builder.CreateBitCast(depInEnvPtr, PointerType::getUnqual(PointerType::getUnqual(envType)));
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
        
        return cast<Value>(builder.CreateBitCast(envAlloca, PointerType::getUnqual(int8)));
      }

      Value * createStagesArrayFromStages (LoopDependenceInfo *LDI, std::vector<std::unique_ptr<StageInfo>> &stages, IRBuilder<> builder)
      {
        auto stagesAlloca = cast<Value>(builder.CreateAlloca(LDI->stageArrayType));
        for (int i = 0; i < stages.size(); ++i)
        {
          auto &stage = stages[i];
          auto stageIndex = cast<Value>(ConstantInt::get(int64, i));
          auto stagePtr = builder.CreateInBoundsGEP(stagesAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, stageIndex }));
          auto stageCast = builder.CreateBitCast(stagePtr, PointerType::getUnqual(stages[0]->sccStage->getType()));
          builder.CreateStore(stage->sccStage, stageCast);
        }
        return cast<Value>(builder.CreateBitCast(stagesAlloca, PointerType::getUnqual(int8)));
      }

      void storeOutgoingDependentsIntoExternalValues (LoopDependenceInfo *LDI, std::vector<std::unique_ptr<StageInfo>> &stages, IRBuilder<> builder, Value *envAlloca)
      {
        /*
         * Extract the outgoing dependents for each stage
         */
        for (auto &stage : stages)
        {
          for (auto dependencyPair : stage->outgoingDependentMap)
          {
            auto externalDependency = dependencyPair.second;
            auto depIndex = stage->externalDependencyToEnvMap[externalDependency];
            auto envIndex = cast<Value>(ConstantInt::get(int64, depIndex));
            auto envType = LDI->externalDependentTypes[depIndex];
            auto depInEnvPtr = builder.CreateInBoundsGEP(envAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndex }));
            auto envVarCast = builder.CreateBitCast(builder.CreateLoad(depInEnvPtr), PointerType::getUnqual(envType));
            auto envVar = builder.CreateLoad(envVarCast);

            if (auto depPHI = dyn_cast<PHINode>(externalDependency))
            {
              depPHI->addIncoming(envVar, LDI->pipelineBB);
              continue;
            }
            LDI->pipelineBB->eraseFromParent();
            errs() << "Loop not in LCSSA!\n";
            abort();
          }
        }
      }

      void createPipelineFromStages (LoopDependenceInfo *LDI, std::vector<std::unique_ptr<StageInfo>> &stages)
      {
        auto M = LDI->func->getParent();
        LDI->pipelineBB = BasicBlock::Create(M->getContext(), "", LDI->func);
        IRBuilder<> builder(LDI->pipelineBB);

        /*
         * Create and populate the environment and stages arrays
         */
        auto envAlloca = cast<Value>(builder.CreateAlloca(LDI->envArrayType));
        auto envPtr = createEnvArrayFromStages(LDI, stages, builder, envAlloca);
        auto stagesPtr = createStagesArrayFromStages(LDI, stages, builder);

        /*
         * Create empty queues array to be used by the stage dispatcher
         */
        auto queuesAlloca = cast<Value>(builder.CreateAlloca(LDI->queueArrayType));
        auto queuesPtr = cast<Value>(builder.CreateBitCast(queuesAlloca, PointerType::getUnqual(int8)));

        /*
         * Call the stage dispatcher with the environment, queues array, and stages array
         */
        auto queuesCount = cast<Value>(ConstantInt::get(int64, LDI->internalDependentTypes.size()));
        auto stagesCount = cast<Value>(ConstantInt::get(int64, stages.size()));
        builder.CreateCall(stageDispatcher, ArrayRef<Value*>({ envPtr, queuesPtr, stagesPtr, stagesCount, queuesCount }));

        storeOutgoingDependentsIntoExternalValues(LDI, stages, builder, envAlloca);

        /*
         * ASSUMPTION: Only one unique exiting basic block from the loop
         */
        builder.CreateBr(LDI->loop->getExitBlock());
        LDI->func->print(errs() << "Final printout:\n"); errs() << "\n";
      }

      void linkParallelizedLoopToOriginalFunction (LoopDependenceInfo *LDI)
      {
        auto M = LDI->func->getParent();
        auto preheader = LDI->loop->getLoopPreheader();
        auto loopSwitch = BasicBlock::Create(M->getContext(), "", LDI->func, preheader);
        IRBuilder<> loopSwitchBuilder(loopSwitch);

        auto globalBool = new GlobalVariable(*M, int32, /*isConstant=*/ false, GlobalValue::ExternalLinkage, Constant::getNullValue(int32));
        auto const0 = ConstantInt::get(int32, APInt(32, 0, false));
        auto compareInstruction = loopSwitchBuilder.CreateICmpEQ(loopSwitchBuilder.CreateLoad(globalBool), const0);
        loopSwitchBuilder.CreateCondBr(compareInstruction, LDI->pipelineBB, preheader);
      }

      void printLoop(Loop *loop)
      {
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

      void printSCCs(SCCDAG *sccSubgraph)
      {
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
