#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DerivedUser.h"
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
#include "llvm/Analysis/CallGraph.h"

#include "llvm/IR/Mangler.h"
#include "llvm/IR/IRBuilder.h"

#include "LoopDependenceInfo.hpp"
#include "PipelineInfo.hpp"
#include "PDG.hpp"
#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "PDGAnalysis.hpp"

#include <unordered_map>
#include <set>
#include <queue>
#include <deque>

using namespace llvm;

namespace llvm {

  struct DSWP : public ModulePass {
    public:
      static char ID;

      Function *stageDispatcher;
      Function *printReachedI;

      std::vector<Function *> queuePushes;
      std::vector<Function *> queuePops;
      std::vector<Type *> queueTypes;
      std::vector<Type *> queueElementTypes;
      unordered_map<int, int> queueSizeToIndex;

      FunctionType *stageType;
      IntegerType *int1, *int8, *int16, *int32, *int64;

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

        /*
         * Collect functions through call graph starting at function "main"
         */
        std::set<Function *> funcToModify;
        collectAllFunctionsInCallGraph(M, funcToModify);

        auto modified = false;
        for (auto F : funcToModify)
        {
          auto loopDI = fetchLoopToParallelize(*F, graph);
          if (loopDI == nullptr) {
            continue ;
          }

          /*
           * Parallelize the current loop with DSWP.
           */
          modified |= applyDSWP(loopDI);
          delete loopDI;
        }
        return modified;
      }

      void getAnalysisUsage (AnalysisUsage &AU) const override
      {
        AU.addRequired<PDGAnalysis>();
        AU.addRequired<AssumptionCacheTracker>();
        AU.addRequired<DominatorTreeWrapperPass>();
        AU.addRequired<PostDominatorTreeWrapperPass>();
        AU.addRequired<LoopInfoWrapperPass>();
        AU.addRequired<ScalarEvolutionWrapperPass>();
        AU.addRequired<CallGraphWrapperPass>();
        return ;
      }

    private:
      void collectAllFunctionsInCallGraph (Module &M, std::set<Function *> &funcSet)
      {
        auto &callGraph = getAnalysis<CallGraphWrapperPass>().getCallGraph();
        std::queue<Function *> funcToTraverse;
        funcToTraverse.push(M.getFunction("main"));
        while (!funcToTraverse.empty())
        {
          auto func = funcToTraverse.front();
          funcToTraverse.pop();
          if (funcSet.find(func) != funcSet.end()) continue;
          funcSet.insert(func);

          auto funcCGNode = callGraph[func];
          for (auto &callRecord : make_range(funcCGNode->begin(), funcCGNode->end()))
          {
            auto F = callRecord.second->getFunction();
            if (F->empty()) continue;
            funcToTraverse.push(F);
          }
        }
      }

      bool collectThreadPoolHelperFunctionsAndTypes (Module &M)
      {
        int1 = IntegerType::get(M.getContext(), 1);
        int8 = IntegerType::get(M.getContext(), 8);
        int16 = IntegerType::get(M.getContext(), 16);
        int32 = IntegerType::get(M.getContext(), 32);
        int64 = IntegerType::get(M.getContext(), 64);

        printReachedI = M.getFunction("printReachedI");
        std::string pushers[4] = { "queuePush8", "queuePush16", "queuePush32", "queuePush64" };
        std::string poppers[4] = { "queuePop8", "queuePop16", "queuePop32", "queuePop64" };
        for (auto pusher : pushers) queuePushes.push_back(M.getFunction(pusher));
        for (auto popper : poppers) queuePops.push_back(M.getFunction(popper));
        for (auto queueF : queuePushes) queueTypes.push_back(queueF->arg_begin()->getType());
        queueSizeToIndex = unordered_map<int, int>({ { 1, 0 }, { 8, 0 }, { 16, 1 }, { 32, 2 }, { 64, 3 }});
        queueElementTypes = std::vector<Type *>({ int8, int16, int32, int64 });

        stageDispatcher = M.getFunction("stageDispatcher");
        auto stageExecuter = M.getFunction("stageExecuter");

        auto stageArgType = stageExecuter->arg_begin()->getType();
        stageType = cast<FunctionType>(cast<PointerType>(stageArgType)->getElementType());
        return true;
      }

      LoopDependenceInfo *fetchLoopToParallelize (Function &function, PDG *graph)
      {
        /*
         * Fetch the loops.
         */
        auto &LI = getAnalysis<LoopInfoWrapperPass>(function).getLoopInfo();
        auto &DT = getAnalysis<DominatorTreeWrapperPass>(function).getDomTree();
        auto &PDT = getAnalysis<PostDominatorTreeWrapperPass>(function).getPostDomTree();
        auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(function).getSE();

        /*
         * Fetch the PDG.
         */
        auto funcPDG = graph->createFunctionSubgraph(function);

        /*
         * ASSUMPTION: One outermost loop for the function.
         *
         * We have to have one single outermost loop.
         */
        // if (std::distance(LI.begin(), LI.end()) != 1) return nullptr;

        /*
         * Choose the loop to parallelize.
         */
        for (auto loopIter : LI)
        {
          auto loop = &*loopIter;
          return new LoopDependenceInfo(&function, funcPDG, loop, LI, DT, PDT, SE);
        }

        return nullptr;
      }

      bool applyDSWP (LoopDependenceInfo *LDI)
      {
        errs() << "Applying DSWP\n";

        /*
         * Merge SCCs of the SCCDAG.
         */
        // printSCCs(LDI->loopSCCDAG);
        mergeSCCs(LDI);
        // printSCCs(LDI->loopSCCDAG);

        /*
         * Create the pipeline stages.
         */
        if (!isWorthParallelizing(LDI)) return false;
        if (!collectStageAndQueueInfo(LDI)) return false;
        // printStageSCCs(LDI);
        // printStageQueues(LDI);

        for (auto &stage : LDI->stages) createPipelineStageFromSCC(LDI, stage);

        /*
         * Create the pipeline (connecting the stages)
         */
        createPipelineFromStages(LDI);
        if (LDI->pipelineBB == nullptr)
        {
          for (auto &stage : LDI->stages) stage->sccStage->eraseFromParent();
          return false;
        }

        /*
         * Link the parallelized loop within the original function that includes the sequential loop.
         */
        linkParallelizedLoopToOriginalFunction(LDI);
        // LDI->function->print(errs() << "Final printout:\n"); errs() << "\n";

        return true;
      }

      void mergeBranchesWithoutOutgoingEdges (LoopDependenceInfo *LDI)
      {
        auto &sccSubgraph = LDI->loopSCCDAG;
        std::vector<DGNode<SCC> *> tailCmpBrs;
        for (auto sccNode : make_range(sccSubgraph->begin_nodes(), sccSubgraph->end_nodes()))
        {
          auto scc = sccNode->getT();
          if (sccNode->numIncomingEdges() == 0 || sccNode->numOutgoingEdges() > 0) continue ;

          bool allCmpOrBr = true;
          for (auto node : scc->getNodes())
          {
            allCmpOrBr &= (isa<TerminatorInst>(node->getT()) || isa<CmpInst>(node->getT()));
          }
          if (allCmpOrBr) tailCmpBrs.push_back(sccNode);
        }

        /*
         * Merge trailing compare/branch scc into previous depth scc
         */
        for (auto tailSCC : tailCmpBrs)
        {
          std::set<DGNode<SCC> *> nodesToMerge = { tailSCC };
          nodesToMerge.insert(*sccSubgraph->previousDepthNodes(tailSCC).begin());
          sccSubgraph->mergeSCCs(nodesToMerge);
        }
      }

      void mergeSCCs (LoopDependenceInfo *LDI)
      {
        // errs() << "Number of unmerged nodes: " << LDI->loopSCCDAG->numNodes() << "\n";

        /*
         * Merge the SCC related to a single PHI node and its use if there is only one.
         */
        //TODO

        mergeBranchesWithoutOutgoingEdges(LDI);

        // errs() << "Number of merged nodes: " << LDI->loopSCCDAG->numNodes() << "\n";
        return ;
      }

      bool isWorthParallelizing (LoopDependenceInfo *LDI)
      {
        return LDI->loopSCCDAG->numNodes() > 1;
      }

      void collectSCCIntoStages (LoopDependenceInfo *LDI)
      {
        auto topLevelSCCNodes = LDI->loopSCCDAG->getTopLevelNodes();

        /*
         * TODO: Check if all entries to the loop are into top level nodes
         */
        std::set<DGNode<SCC> *> nodesFound(topLevelSCCNodes.begin(), topLevelSCCNodes.end());
        std::deque<DGNode<SCC> *> nodesToTraverse(topLevelSCCNodes.begin(), topLevelSCCNodes.end());

        int order = 0;
        while (!nodesToTraverse.empty())
        {
          auto sccNode = nodesToTraverse.front();
          nodesToTraverse.pop_front();

          /*
           * Add all unvisited, next depth nodes to the traversal queue 
           */
          auto nextNodes = LDI->loopSCCDAG->nextDepthNodes(sccNode);
          for (auto next : nextNodes)
          {
            if (nodesFound.find(next) != nodesFound.end()) continue;
            nodesFound.insert(next);
            nodesToTraverse.push_back(next);
          }

          auto scc = sccNode->getT();
          auto stage = std::make_unique<StageInfo>();
          stage->order = order++;
          stage->scc = scc;
          LDI->stages.push_back(std::move(stage));
          LDI->sccToStage[scc] = LDI->stages[order - 1].get();
        }
      }

      bool collectValueQueueInfo (LoopDependenceInfo *LDI)
      {
        for (auto scc : LDI->loopSCCDAG->getNodes())
        {
          for (auto sccEdge : scc->getOutgoingEdges())
          {
            auto sccPair = sccEdge->getNodePair();
            auto fromStage = LDI->sccToStage[sccPair.first->getT()];
            auto toStage = LDI->sccToStage[sccPair.second->getT()];
            if (fromStage == toStage) continue;

            /*
             * Create value and control queues for each dependency of the form: producer -> consumers
             */
            for (auto instructionEdge : sccEdge->getSubEdges())
            {
              assert(!instructionEdge->isMemoryDependence());
              if (instructionEdge->isControlDependence()) continue;

              auto pcPair = instructionEdge->getNodePair();
              auto producer = cast<Instruction>(pcPair.first->getT());
              auto consumer = cast<Instruction>(pcPair.second->getT());

              int queueIndex = LDI->queues.size();
              for (auto queueI : fromStage->producerToQueues[producer])
              {
                if (LDI->queues[queueI]->toStage != toStage->order) continue;
                queueIndex = queueI;
                break;
              }

              if (queueIndex == LDI->queues.size())
              {
                LDI->queues.push_back(std::move(std::make_unique<QueueInfo>(producer, consumer, producer->getType())));
                fromStage->producerToQueues[producer].insert(queueIndex);
              }
              // errs() << "Stage pair: " << fromStage->order << ", " << toStage->order << "\n";
              // producer->print(errs() << "P-C Pair:\t"); consumer->print(errs() << "\t"); errs() << "\n";
              fromStage->pushValueQueues.insert(queueIndex);
              toStage->popValueQueues.insert(queueIndex);
              toStage->producedPopQueue[producer] = queueIndex;

              auto queueInfo = LDI->queues[queueIndex].get();
              queueInfo->consumers.insert(consumer);
              queueInfo->fromStage = fromStage->order;
              queueInfo->toStage = toStage->order;

              if (queueSizeToIndex.find(queueInfo->bitLength) == queueSizeToIndex.end()) return false;
            }
          }
        }

        for (auto bb : LDI->loop->blocks())
        {
          auto consumer = cast<Instruction>(bb->getTerminator());
          auto cV = cast<Value>(consumer);
          // consumer->print(errs() << "CONSUMER BR:\t"); errs() << "\n";
          StageInfo *brStage;
          for (auto &stage : LDI->stages)
          {
            if (!stage->scc->isInternal(cV)) continue;
            brStage = stage.get();
            break;
          }

          auto brNode = brStage->scc->fetchNode(cV);
          for (auto edge : brNode->getIncomingEdges())
          {
            if (edge->isControlDependence()) continue;
            auto producer = cast<Instruction>(edge->getOutgoingT());
            auto pV = cast<Value>(producer);
            StageInfo *prodStage;            
            for (auto &stage : LDI->stages)
            {
              if (!stage->scc->isInternal(pV)) continue;
              prodStage = stage.get();
              break;
            }

            for (auto &otherStage : LDI->stages)
            {
              if (otherStage.get() == prodStage) continue;
              int queueIndex = LDI->queues.size();
              LDI->queues.push_back(std::move(std::make_unique<QueueInfo>(producer, consumer, producer->getType())));
              prodStage->producerToQueues[producer].insert(queueIndex);
              prodStage->pushValueQueues.insert(queueIndex);
              otherStage->popValueQueues.insert(queueIndex);
              otherStage->producedPopQueue[producer] = queueIndex;

              auto queueInfo = LDI->queues[queueIndex].get();
              queueInfo->consumers.insert(consumer);
              queueInfo->fromStage = prodStage->order;
              queueInfo->toStage = otherStage->order;
            }
          }
        }
        return true;
      }

      void collectEnvInfo (LoopDependenceInfo *LDI)
      {
        LDI->environment = std::make_unique<EnvInfo>();
        auto &externalDeps = LDI->environment->externalDependents;
        for (auto nodeI : LDI->loopDG->externalNodePairs())
        {
          auto externalNode = nodeI.second;
          auto externalValue = externalNode->getT();
          auto envIndex = externalDeps.size();
          externalDeps.push_back(externalValue);
          bool envUsed = false;

          auto addExternalDependentToStagesWithInst = [&](Instruction *internalInst, bool outgoing) -> void {
            envUsed = true;
            for (auto &stage : LDI->stages)
            {
              if (!stage->scc->isInternal(cast<Value>(internalInst)) && !isa<TerminatorInst>(internalInst)) continue;
              auto &envMap = outgoing ? stage->outgoingToEnvMap : stage->incomingToEnvMap;
              envMap[internalInst] = envIndex;
              stage->externalToEnvMap[externalValue] = envIndex;
            }
            auto &envSet = outgoing ? LDI->environment->postLoopExternals : LDI->environment->preLoopExternals;
            envSet.insert(envIndex);
          };

          /*
           * Check if loop-external instruction has incoming/outgoing nodes within one of the stages
           */
          for (auto incomingEdge : externalNode->getIncomingEdges())
          {
            if (incomingEdge->isMemoryDependence() || incomingEdge->isControlDependence()) continue;
            addExternalDependentToStagesWithInst(cast<Instruction>(incomingEdge->getOutgoingT()), true);
          }
          for (auto outgoingEdge : externalNode->getOutgoingEdges())
          {
            if (outgoingEdge->isMemoryDependence() || outgoingEdge->isControlDependence()) continue;
            addExternalDependentToStagesWithInst(cast<Instruction>(outgoingEdge->getIncomingT()), false);
          }
          if (!envUsed) externalDeps.pop_back();
        }
      }

      void configureDependencyStorage (LoopDependenceInfo *LDI)
      {
        LDI->zeroIndexForBaseArray = cast<Value>(ConstantInt::get(int64, 0));
        LDI->envArrayType = ArrayType::get(PointerType::getUnqual(int8), LDI->environment->envSize());
        LDI->queueArrayType = ArrayType::get(PointerType::getUnqual(int8), LDI->queues.size());
        LDI->stageArrayType = ArrayType::get(PointerType::getUnqual(int8), LDI->stages.size());
      }

      bool collectStageAndQueueInfo (LoopDependenceInfo *LDI)
      {
        collectSCCIntoStages(LDI);
        if (!collectValueQueueInfo(LDI)) return false;
        collectEnvInfo(LDI);
        configureDependencyStorage(LDI);
        return true;
      }

      void createInstAndBBForSCC (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        auto &context = LDI->function->getParent()->getContext();

        /*
         * Clone instructions within the stage's scc, and their basic blocks
         */
        for (auto nodePair : stageInfo->scc->internalNodePairs())
        {
          auto I = cast<Instruction>(nodePair.first);
          stageInfo->iCloneMap[I] = I->clone();
          // I->print(errs() << "Orig I:\t"); stageInfo->iCloneMap[I]->print(errs() << "\tInternal I:\t"); errs() << "\n";
        }

        for (auto B : LDI->loop->blocks())
        {
          stageInfo->sccBBCloneMap[B] = BasicBlock::Create(context, "", stageInfo->sccStage);
          auto terminator = cast<Instruction>(B->getTerminator());
          if (stageInfo->iCloneMap.find(terminator) == stageInfo->iCloneMap.end())
          {
            stageInfo->iCloneMap[terminator] = terminator->clone();
          }
        }
        for (int i = 0; i < LDI->loopExitBlocks.size(); ++i)
        {
          stageInfo->sccBBCloneMap[LDI->loopExitBlocks[i]] = stageInfo->loopExitBlocks[i];
        }

        /*
         * Attach SCC instructions to their basic blocks in correct relative order
         */
        for (auto B : LDI->loop->blocks())
        {
          IRBuilder<> builder(stageInfo->sccBBCloneMap[B]);
          for (auto &I : *B)
          {
            if (stageInfo->iCloneMap.find(&I) == stageInfo->iCloneMap.end()) continue;
            auto iClone = stageInfo->iCloneMap[&I];
            builder.Insert(iClone);
          }
        }
      }

      void loadAndStoreEnv (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        IRBuilder<> entryBuilder(stageInfo->entryBlock);

        auto envArg = &*(stageInfo->sccStage->arg_begin());
        auto envAlloca = entryBuilder.CreateBitCast(envArg, PointerType::getUnqual(LDI->envArrayType));

        auto accessEnvVarFromIndex = [&](int envIndex, IRBuilder<> builder) -> Value * {
          auto envIndexValue = cast<Value>(ConstantInt::get(int64, envIndex));
          auto envPtr = builder.CreateInBoundsGEP(envAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndexValue }));
          auto envType = LDI->environment->externalDependents[envIndex]->getType();
          return builder.CreateBitCast(builder.CreateLoad(envPtr), PointerType::getUnqual(envType));
        };

        /*
         * Store (SCC -> outside of loop) dependencies within the environment array
         */
        for (auto outgoingEnvPair : stageInfo->outgoingToEnvMap)
        {
          auto outgoingDepClone = stageInfo->iCloneMap[outgoingEnvPair.first];
          auto outgoingDepBB = outgoingDepClone->getParent();
          IRBuilder<> outgoingBuilder(outgoingDepBB->getTerminator());
          auto envVar = accessEnvVarFromIndex(outgoingEnvPair.second, outgoingBuilder);
          outgoingBuilder.CreateStore(outgoingDepClone, envVar);
        }

        /*
         * Store exit index in the exit environment variable
         */
        for (int i = 0; i < stageInfo->loopExitBlocks.size(); ++i)
        {
          IRBuilder<> builder(stageInfo->loopExitBlocks[i]);
          auto envIndexValue = cast<Value>(ConstantInt::get(int64, LDI->environment->externalDependents.size()));
          auto envPtr = builder.CreateInBoundsGEP(envAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndexValue }));
          auto envVar = builder.CreateBitCast(builder.CreateLoad(envPtr), PointerType::getUnqual(int32));
          builder.CreateStore(ConstantInt::get(int32, i), envVar);
        }

        /*
         * Load (outside of loop -> SCC) dependencies from the environment array 
         */
        for (auto incomingEnvPair : stageInfo->incomingToEnvMap)
        {
          auto envVar = accessEnvVarFromIndex(incomingEnvPair.second, entryBuilder);
          auto envLoad = entryBuilder.CreateLoad(envVar);
          stageInfo->envLoadMap[incomingEnvPair.second] = cast<Instruction>(envLoad);
        }
      }

      void loadAllQueuePointersInEntry (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        IRBuilder<> entryBuilder(stageInfo->entryBlock);
        auto argIter = stageInfo->sccStage->arg_begin();
        auto queuesArray = entryBuilder.CreateBitCast(&*(++argIter), PointerType::getUnqual(LDI->queueArrayType));

        /*
         * Load this stage's relevant queues
         */
        auto loadQueuePtrFromIndex = [&](int queueIndex) -> void {
          auto queueInfo = LDI->queues[queueIndex].get();
          auto queueIndexValue = cast<Value>(ConstantInt::get(int64, queueIndex));
          auto queuePtr = entryBuilder.CreateInBoundsGEP(queuesArray, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, queueIndexValue }));
          auto queueCast = entryBuilder.CreateBitCast(queuePtr, PointerType::getUnqual(queueTypes[queueSizeToIndex[queueInfo->bitLength]]));

          auto queueInstrs = std::make_unique<QueueInstrs>();
          queueInstrs->queuePtr = entryBuilder.CreateLoad(queueCast);
          queueInstrs->alloca = entryBuilder.CreateAlloca(queueInfo->dependentType);
          queueInstrs->allocaCast = entryBuilder.CreateBitCast(queueInstrs->alloca, PointerType::getUnqual(queueElementTypes[queueSizeToIndex[queueInfo->bitLength]]));
          stageInfo->queueInstrMap[queueIndex] = std::move(queueInstrs);
        };

        for (auto queueIndex : stageInfo->pushValueQueues) loadQueuePtrFromIndex(queueIndex);
        for (auto queueIndex : stageInfo->popValueQueues) loadQueuePtrFromIndex(queueIndex);
      }

      void popValueQueues (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        for (auto queueIndex : stageInfo->popValueQueues)
        {
          auto &queueInfo = LDI->queues[queueIndex];
          auto queueInstrs = stageInfo->queueInstrMap[queueIndex].get();
          auto queueCallArgs = ArrayRef<Value*>({ queueInstrs->queuePtr, queueInstrs->allocaCast });

          auto bb = queueInfo->producer->getParent();
          IRBuilder<> builder(stageInfo->sccBBCloneMap[bb]);
          queueInstrs->queueCall = builder.CreateCall(queuePops[queueSizeToIndex[queueInfo->bitLength]], queueCallArgs);
          queueInstrs->load = builder.CreateLoad(queueInstrs->alloca);

          /*
           * Position queue call and load relatively identically to where the producer is in the basic block
           */
          bool pastProducer = false;
          for (auto &I : *bb)
          {
            if (&I == queueInfo->producer) pastProducer = true;
            else if (auto phi = dyn_cast<PHINode>(&I)) continue;
            else if (pastProducer && stageInfo->iCloneMap.find(&I) != stageInfo->iCloneMap.end())
            {
              cast<Instruction>(queueInstrs->queueCall)->moveBefore(stageInfo->iCloneMap[&I]);
              cast<Instruction>(queueInstrs->load)->moveBefore(stageInfo->iCloneMap[&I]);
              break;
            }
          }
        }
      }

      void pushValueQueues (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        for (auto queueIndex : stageInfo->pushValueQueues)
        {
          auto queueInstrs = stageInfo->queueInstrMap[queueIndex].get();
          auto queueInfo = LDI->queues[queueIndex].get();
          auto queueCallArgs = ArrayRef<Value*>({ queueInstrs->queuePtr, queueInstrs->allocaCast });
          
          auto pClone = stageInfo->iCloneMap[queueInfo->producer];
          auto pCloneBB = pClone->getParent();
          IRBuilder<> builder(pCloneBB);
          auto store = builder.CreateStore(pClone, queueInstrs->alloca);
          queueInstrs->queueCall = builder.CreateCall(queuePushes[queueSizeToIndex[queueInfo->bitLength]], queueCallArgs);

          bool pastProducer = false;
          for (auto &I : *pCloneBB)
          {
            if (&I == pClone) pastProducer = true;
            else if (auto phi = dyn_cast<PHINode>(&I)) continue;
            else if (pastProducer)
            {
              store->moveBefore(&I);
              cast<Instruction>(queueInstrs->queueCall)->moveBefore(&I);
              
              if (pClone->getType() == int32)
              {
                //auto printCall = builder.CreateCall(printReachedI, ArrayRef<Value*>({ cast<Value>(pClone) }));
                //printCall->moveBefore(&I);
              }
              break;
            }
          }
        }
      }

      void remapOperandsOfInstClones (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        auto &iCloneMap = stageInfo->iCloneMap;
        for (auto ii = iCloneMap.begin(); ii != iCloneMap.end(); ++ii) {
          auto cloneInstruction = ii->second;

          for (auto &op : cloneInstruction->operands()) {
            auto opV = op.get();
            if (auto opI = dyn_cast<Instruction>(opV)) {
              if (iCloneMap.find(opI) != iCloneMap.end()) {
                op.set(iCloneMap[opI]);
                // opV->print(errs() << "Set in op\t"); cloneInstruction->print(errs() << "\t"); errs() << "\n";
              } else if (stageInfo->externalToEnvMap.find(opV) != stageInfo->externalToEnvMap.end()) {
                op.set(stageInfo->envLoadMap[stageInfo->externalToEnvMap[opV]]);
                // opV->print(errs() << "Set env op\t"); cloneInstruction->print(errs() << "\t"); errs() << "\n";
              } else if (stageInfo->producedPopQueue.find(opI) != stageInfo->producedPopQueue.end()) {
                op.set(stageInfo->queueInstrMap[stageInfo->producedPopQueue[opI]]->load);
                // opV->print(errs() << "Set pop op\t"); cloneInstruction->print(errs() << "\t"); errs() << "\n";
              } else {
                opV->print(errs() << "Ignore operand\t"); cloneInstruction->print(errs() << "\t"); errs() << "\n";
                abort();
              }
              continue;
            } else if (auto opA = dyn_cast<Argument>(opV)) {
              if (stageInfo->externalToEnvMap.find(opV) != stageInfo->externalToEnvMap.end()) {
                op.set(stageInfo->envLoadMap[stageInfo->externalToEnvMap[opV]]);
                // opV->print(errs() << "Set env op\t"); cloneInstruction->print(errs() << "\t"); errs() << "\n";
              } else {
                opV->print(errs() << "Ignore operand\t"); cloneInstruction->print(errs() << "\t"); errs() << "\n";
                abort();
              }
            } else if (auto opC = dyn_cast<Constant>(opV)) {
              continue;
            } else if (auto opB = dyn_cast<BasicBlock>(opV)) {
              continue;
            } else if (auto opF = dyn_cast<Function>(opV)) {
              continue;
            } else if (auto opDU = dyn_cast<DerivedUser>(opV)) {
              continue;
            } else {
              opV->print(errs() << "Unknown what to do with operand\n"); opV->getType()->print(errs() << "\tType:\t");
              cloneInstruction->print(errs() << "\nfor instruction:\n"); errs() << "\n";
              abort();
            }
          }
        }
      }

      void remapControlFlow (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        auto &context = LDI->function->getContext();
        auto stageF = stageInfo->sccStage;

        for (auto bbPair : stageInfo->sccBBCloneMap)
        {
          auto originalT = bbPair.first->getTerminator();
          if (stageInfo->iCloneMap.find(originalT) == stageInfo->iCloneMap.end()) continue;
          auto terminator = cast<TerminatorInst>(stageInfo->iCloneMap[originalT]);
          for (int i = 0; i < terminator->getNumSuccessors(); ++i)
          {
            terminator->setSuccessor(i, stageInfo->sccBBCloneMap[terminator->getSuccessor(i)]);
          }
        }

        for (auto bbPair : stageInfo->sccBBCloneMap)
        {
          auto iIter = bbPair.second->begin();
          while (auto phi = dyn_cast<PHINode>(&*iIter))
          {
            for (auto bb : phi->blocks())
            {
              phi->setIncomingBlock(phi->getBasicBlockIndex(bb), stageInfo->sccBBCloneMap[bb]);
            }
            ++iIter;
          }
        }
      }

      void createPipelineStageFromSCC (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        auto M = LDI->function->getParent();
        auto stageF = cast<Function>(M->getOrInsertFunction("", stageType));
        auto &context = M->getContext();
        stageInfo->sccStage = stageF;
        stageInfo->entryBlock = BasicBlock::Create(context, "", stageF);
        stageInfo->exitBlock = BasicBlock::Create(context, "", stageF);
        stageInfo->sccBBCloneMap[LDI->loop->getLoopPreheader()] = stageInfo->entryBlock;
        for (auto exitBB : LDI->loopExitBlocks) stageInfo->loopExitBlocks.push_back(BasicBlock::Create(context, "", stageF));

        // errs() << "Stage:\t" << stageInfo->order << "\n";

        /*
         * SCC iteration
         */
        createInstAndBBForSCC(LDI, stageInfo);
        loadAllQueuePointersInEntry(LDI, stageInfo);
        popValueQueues(LDI, stageInfo);
        pushValueQueues(LDI, stageInfo);
        loadAndStoreEnv(LDI, stageInfo);

        remapControlFlow(LDI, stageInfo);
        remapOperandsOfInstClones(LDI, stageInfo);

        IRBuilder<> entryBuilder(stageInfo->entryBlock);
        entryBuilder.CreateBr(stageInfo->sccBBCloneMap[LDI->loop->getHeader()]);

        /*
         * Cleanup
         */
        for (auto exitBB : stageInfo->loopExitBlocks)
        {
          IRBuilder<> builder(exitBB);
          builder.CreateBr(stageInfo->exitBlock);
        }
        IRBuilder<> exitBuilder(stageInfo->exitBlock);
        exitBuilder.CreateRetVoid();
        // stageF->print(errs() << "Function printout:\n"); errs() << "\n";
      }

      Value * createEnvArrayFromStages (LoopDependenceInfo *LDI, IRBuilder<> funcBuilder, IRBuilder<> builder, Value *envAlloca)
      {
        /*
         * Create empty environment array with slots for external values dependent on loop values
         */
        std::vector<Value*> envPtrsForDep;
        auto extDepSize = LDI->environment->externalDependents.size();
        for (int i = 0; i < extDepSize; ++i)
        {
          Type *envType = LDI->environment->externalDependents[i]->getType();
          auto envVarPtr = funcBuilder.CreateAlloca(envType);
          envPtrsForDep.push_back(envVarPtr);
          auto envIndex = cast<Value>(ConstantInt::get(int64, i));
          auto depInEnvPtr = funcBuilder.CreateInBoundsGEP(envAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndex }));

          auto depCast = funcBuilder.CreateBitCast(depInEnvPtr, PointerType::getUnqual(PointerType::getUnqual(envType)));
          funcBuilder.CreateStore(envVarPtr, depCast);
        }

        /*
         * Add exit block tracking variable to env
         */
        auto exitVarPtr = funcBuilder.CreateAlloca(int32);
        auto envIndex = cast<Value>(ConstantInt::get(int64, extDepSize));
        auto varInEnvPtr = funcBuilder.CreateInBoundsGEP(envAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndex }));
        auto depCast = funcBuilder.CreateBitCast(varInEnvPtr, PointerType::getUnqual(PointerType::getUnqual(int32)));
        funcBuilder.CreateStore(exitVarPtr, depCast);

        /*
         * Insert incoming dependents for stages into the environment array
         */
        for (int envIndex : LDI->environment->preLoopExternals)
        {
          builder.CreateStore(LDI->environment->externalDependents[envIndex], envPtrsForDep[envIndex]);
        }
        
        return cast<Value>(builder.CreateBitCast(envAlloca, PointerType::getUnqual(int8)));
      }

      Value * createStagesArrayFromStages (LoopDependenceInfo *LDI, IRBuilder<> funcBuilder)
      {
        auto stagesAlloca = cast<Value>(funcBuilder.CreateAlloca(LDI->stageArrayType));
        auto stageCastType = PointerType::getUnqual(LDI->stages[0]->sccStage->getType());
        for (int i = 0; i < LDI->stages.size(); ++i)
        {
          auto &stage = LDI->stages[i];
          auto stageIndex = cast<Value>(ConstantInt::get(int64, i));
          auto stagePtr = funcBuilder.CreateInBoundsGEP(stagesAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, stageIndex }));
          auto stageCast = funcBuilder.CreateBitCast(stagePtr, stageCastType);
          funcBuilder.CreateStore(stage->sccStage, stageCast);
        }
        return cast<Value>(funcBuilder.CreateBitCast(stagesAlloca, PointerType::getUnqual(int8)));
      }

      Value * createQueueSizesArrayFromStages (LoopDependenceInfo *LDI, IRBuilder<> funcBuilder)
      {
        auto queuesAlloca = cast<Value>(funcBuilder.CreateAlloca(ArrayType::get(int64, LDI->queues.size())));
        for (int i = 0; i < LDI->queues.size(); ++i)
        {
          auto &queue = LDI->queues[i];
          auto queueIndex = cast<Value>(ConstantInt::get(int64, i));
          auto queuePtr = funcBuilder.CreateInBoundsGEP(queuesAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, queueIndex }));
          auto queueCast = funcBuilder.CreateBitCast(queuePtr, PointerType::getUnqual(int64));
          funcBuilder.CreateStore(ConstantInt::get(int64, queue->bitLength), queueCast);
        }
        return cast<Value>(funcBuilder.CreateBitCast(queuesAlloca, PointerType::getUnqual(int64)));
      }

      void storeOutgoingDependentsIntoExternalValues (LoopDependenceInfo *LDI, IRBuilder<> builder, Value *envAlloca)
      {
        /*
         * Extract the outgoing dependents for each stage
         */
        for (int envInd : LDI->environment->postLoopExternals)
        {
          auto depI = LDI->environment->externalDependents[envInd];
          auto envIndex = cast<Value>(ConstantInt::get(int64, envInd));
          auto depInEnvPtr = builder.CreateInBoundsGEP(envAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndex }));
          auto envVarCast = builder.CreateBitCast(builder.CreateLoad(depInEnvPtr), PointerType::getUnqual(depI->getType()));
          auto envVar = builder.CreateLoad(envVarCast);

          if (auto depPHI = dyn_cast<PHINode>(depI))
          {
            depPHI->addIncoming(envVar, LDI->pipelineBB);
            continue;
          }
          LDI->pipelineBB->eraseFromParent();
          depI->print(errs() << "Dep I:\t"); errs() << "\n";
          errs() << "Loop not in LCSSA!\n";
          abort();
        }
      }

      void createPipelineFromStages (LoopDependenceInfo *LDI)
      {
        auto M = LDI->function->getParent();
        LDI->pipelineBB = BasicBlock::Create(M->getContext(), "", LDI->function);
        IRBuilder<> builder(LDI->pipelineBB);
        
        auto firstBB = &*LDI->function->begin();
        IRBuilder<> funcBuilder(firstBB->getTerminator());

        /*
         * Create and populate the environment and stages arrays
         */
        auto envAlloca = cast<Value>(funcBuilder.CreateAlloca(LDI->envArrayType));
        auto envPtr = createEnvArrayFromStages(LDI, funcBuilder, builder, envAlloca);
        auto stagesPtr = createStagesArrayFromStages(LDI, funcBuilder);

        /*
         * Create empty queues array to be used by the stage dispatcher
         */
        auto queuesAlloca = cast<Value>(funcBuilder.CreateAlloca(LDI->queueArrayType));
        auto queuesPtr = cast<Value>(builder.CreateBitCast(queuesAlloca, PointerType::getUnqual(int8)));
        auto queueSizesPtr = createQueueSizesArrayFromStages(LDI, funcBuilder);

        /*
         * Call the stage dispatcher with the environment, queues array, and stages array
         */
        auto queuesCount = cast<Value>(ConstantInt::get(int64, LDI->queues.size()));
        auto stagesCount = cast<Value>(ConstantInt::get(int64, LDI->stages.size()));
        builder.CreateCall(stageDispatcher, ArrayRef<Value*>({ envPtr, queuesPtr, queueSizesPtr, stagesPtr, stagesCount, queuesCount }));

        storeOutgoingDependentsIntoExternalValues(LDI, builder, envAlloca);

        /*
         * Branch from pipeline to the correct loop exit block
         */
        auto envIndex = cast<Value>(ConstantInt::get(int64, LDI->environment->envSize() - 1));
        auto depInEnvPtr = builder.CreateInBoundsGEP(envAlloca, ArrayRef<Value*>({ LDI->zeroIndexForBaseArray, envIndex }));
        auto envVarCast = builder.CreateBitCast(builder.CreateLoad(depInEnvPtr), PointerType::getUnqual(int32));
        auto envVar = builder.CreateLoad(envVarCast);

        auto exitSwitch = builder.CreateSwitch(envVar, LDI->loopExitBlocks[0]);
        for (int i = 1; i < LDI->loopExitBlocks.size(); ++i)
        {
          exitSwitch->addCase(ConstantInt::get(int32, i), LDI->loopExitBlocks[i]);
        }
      }

      void linkParallelizedLoopToOriginalFunction (LoopDependenceInfo *LDI)
      {
        auto M = LDI->function->getParent();
        auto preheader = LDI->loop->getLoopPreheader();
        auto originalTerminator = preheader->getTerminator();
        IRBuilder<> loopSwitchBuilder(originalTerminator);

        auto globalBool = new GlobalVariable(*M, int32, /*isConstant=*/ false, GlobalValue::ExternalLinkage, Constant::getNullValue(int32));
        auto const0 = ConstantInt::get(int32, 0);
        auto const1 = ConstantInt::get(int32, 1);
        auto globalLoad = loopSwitchBuilder.CreateLoad(globalBool);
        auto compareInstruction = loopSwitchBuilder.CreateICmpEQ(globalLoad, const0);
        loopSwitchBuilder.CreateCondBr(compareInstruction, LDI->pipelineBB, LDI->loop->getHeader());
        originalTerminator->eraseFromParent();

        /*
         * Set/Reset global variable so only one loop is run in parallel at a time
         */
        IRBuilder<> pipelineBuilder(&*LDI->pipelineBB->begin());
        pipelineBuilder.CreateStore(const1, globalBool);
        pipelineBuilder.SetInsertPoint(LDI->pipelineBB->getTerminator());
        pipelineBuilder.CreateStore(const0, globalBool);
      }

      /*
       * Debug printers:
       */

      void printLoop (Loop *loop)
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

      void printSCCs (SCCDAG *sccSubgraph)
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
          // (*edgeI)->print(errs());
          for (auto subEdge : (*edgeI)->getSubEdges()) subEdge->print(errs());
        }
        errs() << "Number of edges: " << std::distance(sccSubgraph->begin_edges(), sccSubgraph->end_edges()) << "\n";
      }

      void printStageSCCs (LoopDependenceInfo *LDI)
      {
        for (auto &stage : LDI->stages)
        {
          errs() << "Stage: " << stage->order << "\n";
          stage->scc->print(errs() << "SCC:\n") << "\n";
          for (auto edge : stage->scc->getEdges()) edge->print(errs()) << "\n";
        }
      }

      void printStageQueues (LoopDependenceInfo *LDI)
      {
        for (auto &stage : LDI->stages)
        {
          errs() << "Stage: " << stage->order << "\n";
          errs() << "Push value queues: ";
          for (auto qInd : stage->pushValueQueues) errs() << qInd << " ";
          errs() << "\nPop value queues: ";
          for (auto qInd : stage->popValueQueues) errs() << qInd << " ";
          errs() << "\n";
        }

        int count = 0;
        for (auto &queue : LDI->queues)
        {
          errs() << "Queue: " << count++ << "\n";
          queue->producer->print(errs() << "Producer:\t"); errs() << "\n";
          for (auto consumer : queue->consumers)
          {
            consumer->print(errs() << "Consumer:\t"); errs() << "\n";
          }
        }
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
