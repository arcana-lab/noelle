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

      Function *stageDispatcher, *queuePushTemporary, *queuePopTemporary;
      FunctionType *stageType;
      Type *queueType;
      IntegerType *int1, *int8, *int32, *int64;

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
        if (std::distance(LI.begin(), LI.end()) != 1) return nullptr;

        /*
         * Choose the loop to parallelize.
         */
        for (auto loopIter : LI)
        {
          auto loop = &*loopIter;

          /* 
           * ASSUMPTION: No sub-loops.
           */
          auto subLoops = loop->getSubLoops();
          if (subLoops.size() > 0) continue ;
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
        mergeSCCs(LDI);
        // printSCCs(LDI->loopSCCDAG);

        /*
         * Create the pipeline stages.
         */
        if (!isWorthParallelizing(LDI)) return false;
        // printStageSCCs(LDI);
        // printStageQueues(LDI);
        // printLocalSwitches(LDI);
        
        for (auto &stage : LDI->stages) createPipelineStageFromSCC(LDI, stage);
        return false;
        /*
         * Create the pipeline (connecting the stages)
         */
        // createPipelineFromStages(LDI);
        if (LDI->pipelineBB == nullptr)
        {
          for (auto &stage : LDI->stages) stage->sccStage->eraseFromParent();
          return false;
        }

        /*
         * Link the parallelized loop within the original function that includes the sequential loop.
         */
        //linkParallelizedLoopToOriginalFunction(LDI);

        return true;
      }

      void mergeTailBranches (LoopDependenceInfo *LDI)
      {
        auto &sccSubgraph = LDI->loopSCCDAG;
        std::vector<DGNode<SCC> *> tailBranches;
        for (auto sccNode : make_range(sccSubgraph->begin_nodes(), sccSubgraph->end_nodes()))
        {
          auto scc = sccNode->getT();
          if (scc->numInternalNodes() > 1) continue ;
          if (sccNode->numOutgoingEdges() > 0) continue ;
          
          auto singleInstrNode = *scc->begin_nodes();
          if (auto branch = dyn_cast<TerminatorInst>(singleInstrNode->getT())) tailBranches.push_back(sccNode);
        }

        /*
         * Merge trailing branch nodes into previous depth scc
         */
        for (auto tailBranch : tailBranches)
        {
          std::set<DGNode<SCC> *> nodesToMerge = { tailBranch };
          nodesToMerge.insert(*sccSubgraph->previousDepthNodes(tailBranch).begin());
          sccSubgraph->mergeSCCs(nodesToMerge);
        }
      }

      void mergeSCCs (LoopDependenceInfo *LDI)
      {
        errs() << "Number of unmerged nodes: " << LDI->loopSCCDAG->numNodes() << "\n";

        /*
         * Merge the SCC related to a single PHI node and its use if there is only one.
         */
        //TODO

        mergeTailBranches(LDI);

        errs() << "Number of merged nodes: " << LDI->loopSCCDAG->numNodes() << "\n";
        return ;
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
          nodesFound.insert(sccNode);

          /*
           * Add all unvisited, next depth nodes to the traversal queue 
           */
          auto nextNodes = LDI->loopSCCDAG->nextDepthNodes(sccNode);
          for (auto next : nextNodes)
          {
            if (nodesFound.find(next) != nodesFound.end()) continue;
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

      void computeStageEntriesAndExits (LoopDependenceInfo *LDI)
      {
        /*
         * TODO: Build entirely from CDG instead of CFG!!!
         */

        for (auto &stage : LDI->stages)
        {
          /*
           * Log all basic blocks partially or fully executed by this stage
           */
          for (auto nodePair : stage->scc->internalNodePairs())
          {
            auto I = cast<Instruction>(nodePair.first);
            stage->sccBBs.insert(I->getParent());
          }

          for (auto BB : stage->sccBBs)
          {
            /*
             * Tag stages' basic blocks for reference
             */
            LDI->bbToStage[BB].insert(stage.get());

            /*
             * Log all entry blocks to this stage's basic block
             */
            for (auto predBBIter = pred_begin(BB); predBBIter != pred_end(BB); ++predBBIter)
            {
              if (stage->sccBBs.find(*predBBIter) != stage->sccBBs.end()) continue;
              stage->sccEntries.insert(*predBBIter);
            }

            /*
             * If this stage holds the terminator of the basic block, log all exit blocks
             */
            if (!stage->scc->isInternal((Value*)BB->getTerminator())) continue;
            for (auto succBBIter = succ_begin(BB); succBBIter != succ_end(BB); ++succBBIter)
            {
              auto succBB = *succBBIter;
              if (stage->sccBBs.find(succBB) != stage->sccBBs.end()) continue;
              stage->sccExits.insert(succBB);
              
              /*
               * Index the entry point taken by the exiting block of this stage
               */
              int index = 0;
              for (auto predOfSuccIter = pred_begin(succBB); predOfSuccIter != pred_end(succBB); ++predOfSuccIter, ++index)
              {
                if (*predOfSuccIter != BB) continue;
                stage->exitBBToIndex[succBB] = index;
                break;
              }
            }
          }
        }
      }

      bool createControlAndValueQueues (LoopDependenceInfo *LDI)
      {
        for (auto scc : make_range(LDI->loopSCCDAG->begin_nodes(), LDI->loopSCCDAG->end_nodes()))
        {
          for (auto sccEdge : make_range(scc->begin_outgoing_edges(), scc->end_outgoing_edges()))
          {
            auto sccPair = sccEdge->getNodePair();
            auto fromStage = LDI->sccToStage[sccPair.first->getT()];
            auto toStage = LDI->sccToStage[sccPair.second->getT()];
            if (fromStage == toStage) continue;

            /*
             * Create value and control queues for each dependency of the form: producer -> consumers
             */
            for (auto instructionEdge : make_range(sccEdge->begin_sub_edges(), sccEdge->end_sub_edges()))
            {
              /*
               * ASSERTION: No memory data dependences across strongly connected components
               */
              if (instructionEdge->isMemoryDependence()) return false;

              auto pcPair = instructionEdge->getNodePair();
              auto producer = cast<Instruction>(pcPair.first->getT());
              auto consumer = cast<Instruction>(pcPair.second->getT());
              
              auto isControl = instructionEdge->isControlDependence();
              auto dependentType = isControl ? int1 : producer->getType();

              int queueIndex = LDI->queues.size();
              for (auto queueI : fromStage->producerToQueues[producer])
              {
                if (LDI->queues[queueI]->toStage != toStage->order) continue;
                queueIndex = queueI;
                break;
              }

              if (queueIndex == LDI->queues.size())
              {
                LDI->queues.push_back(std::move(std::make_unique<QueueInfo>(producer, consumer, dependentType)));
                fromStage->producerToQueues[producer].insert(queueIndex);
              }
              toStage->consumerToQueues[consumer].insert(queueIndex);

              if (isControl)
              {
                fromStage->pushControlQueues.insert(queueIndex);
                toStage->popControlQueues.insert(queueIndex);
              }
              else
              {
                fromStage->pushValueQueues.insert(queueIndex);
                toStage->popValueQueues.insert(queueIndex);
              }

              auto queueInfo = LDI->queues[queueIndex].get();
              queueInfo->consumers.insert(consumer);
              queueInfo->fromStage = fromStage->order;
              queueInfo->toStage = toStage->order;
            }
          }
        }
        return true;
      }

      void addLocalSwitches (LoopDependenceInfo *LDI)
      {
        for (auto sccNode : LDI->loopSCCDAG->getNodes())
        {
          auto scc = sccNode->getT();
          auto stage = LDI->sccToStage[scc];

          for (auto instructionEdge : scc->getEdges())
          {
            auto pcPair = instructionEdge->getNodePair();
            auto producer = cast<Instruction>(pcPair.first->getT());
            auto consumer = cast<Instruction>(pcPair.second->getT());

            if (!scc->isInternal(producer) || !scc->isInternal(consumer)) continue;
            if (auto consumerPHI = dyn_cast<PHINode>(consumer))
            {
              int opInd = 0;
              auto prodV = cast<Value>(producer);
              for (; opInd < consumerPHI->getNumOperands(); ++opInd)
              {
                if (prodV == consumerPHI->getOperand(opInd)) break;
              }
              if (opInd == consumerPHI->getNumOperands()) continue;

              auto localSwitchI = stage->consumerToLocalSwitches.find(consumer);
              if (localSwitchI != stage->consumerToLocalSwitches.end())
              {
                (*localSwitchI).second->producerToPushIndex[producer] = opInd;
                continue;
              }

              auto localSwitch = std::make_unique<LocalSwitch>();
              localSwitch->producerToPushIndex[producer] = opInd;
              stage->consumerToLocalSwitches[consumer] = std::move(localSwitch);
              continue;
            }
          }
        }

        /*
         * Locate default entry into consumerPHIs
         */
        for (auto &stage : LDI->stages)
        {
          for (auto &consumerSwitch : stage->consumerToLocalSwitches)
          {
            auto consumerPHI = cast<PHINode>(consumerSwitch.first);
            auto &localSwitch = consumerSwitch.second;
            localSwitch->defaultEntry = -1;
            for (auto BB : consumerPHI->blocks())
            {
              if (LDI->bbToStage.find(BB) != LDI->bbToStage.end()) continue;

              if (localSwitch->defaultEntry != -1)
              {
                errs() << "More than one unique entry point into the loop!\n";
                abort();
              }
              localSwitch->defaultEntry = consumerPHI->getBasicBlockIndex(BB);
            }
          }
        }
      }

      bool createSwitchQueues (LoopDependenceInfo *LDI)
      {
        for (auto &toStage : LDI->stages)
        {
          for (auto queueIndex : toStage->popValueQueues)
          {
            auto queueInfo = LDI->queues[queueIndex].get();
            auto producer = queueInfo->producer;
            auto producerBB = producer->getParent();
            auto &fromStage = LDI->stages[queueInfo->fromStage];

            for (auto consumer : queueInfo->consumers)
            {
              /*
               * For each PHINode consumer, create switch queues from all producers to consumerPHI (via predecessor producer dominates)
               */
              if (auto consumerPHI = dyn_cast<PHINode>(consumer))
              {
                auto prodV = cast<Value>(producer);
                int opInd = 0;
                for (; opInd < consumerPHI->getNumOperands(); ++opInd)
                {
                  if (prodV == consumerPHI->getOperand(opInd)) break;
                }

                auto queueIndex = LDI->queues.size();
                LDI->queues.push_back(std::move(std::make_unique<QueueInfo>(producer, consumerPHI, int32)));
                LDI->queues[queueIndex]->consumerToPushIndex[consumerPHI] = opInd;
                fromStage->producerToQueues[producer].insert(queueIndex);
                toStage->consumerToQueues[consumer].insert(queueIndex);
                fromStage->pushSwitchQueues.insert(queueIndex);
                toStage->popSwitchQueues.insert(queueIndex);
              }
            }
          }
        }
        return true;
      }

      bool collectQueueInfo (LoopDependenceInfo *LDI)
      {
        return createControlAndValueQueues(LDI) && createSwitchQueues(LDI);
      }

      bool collectEnvInfo (LoopDependenceInfo *LDI)
      {
        LDI->environment = std::make_unique<EnvInfo>();
        auto &externalDeps = LDI->environment->externalDependents;
        for (auto nodeI : LDI->loopDG->externalNodePairs())
        {
          auto externalNode = nodeI.second;
          auto externalValue = externalNode->getT();
          auto envIndex = externalDeps.size();
          externalDeps.push_back(externalValue);

          auto addExternalDependentToStagesWithInst = [&](Instruction *internalInst, bool outgoing) -> void {
            for (auto &stage : LDI->stages)
            {
              if (!stage->scc->isInternal(cast<Value>(internalInst))) continue;
              auto &envMap = outgoing ? stage->outgoingToEnvMap : stage->incomingToEnvMap;
              envMap[internalInst] = envIndex;
            }
          };

          /*
           * Check if loop-external instruction has incoming/outgoing nodes within one of the stages
           */
          for (auto incomingEdge : externalNode->getIncomingEdges())
          {
            addExternalDependentToStagesWithInst(cast<Instruction>(incomingEdge->getOutgoingNode()->getT()), true);
          }
          for (auto outgoingEdge : externalNode->getOutgoingEdges())
          {
            addExternalDependentToStagesWithInst(cast<Instruction>(outgoingEdge->getIncomingNode()->getT()), false);
          }
        }
        return true;
      }

      bool configureDependencyStorage (LoopDependenceInfo *LDI)
      {
        LDI->zeroIndexForBaseArray = cast<Value>(ConstantInt::get(int64, 0));
        LDI->envArrayType = ArrayType::get(PointerType::getUnqual(int8), LDI->environment->externalDependents.size());
        LDI->queueArrayType = ArrayType::get(PointerType::getUnqual(int8), LDI->queues.size());
        LDI->stageArrayType = ArrayType::get(PointerType::getUnqual(int8), LDI->stages.size());
        return true;
      }

      bool isWorthParallelizing (LoopDependenceInfo *LDI)
      {
        if (LDI->loopSCCDAG->numNodes() <= 1) return false;
        collectSCCIntoStages(LDI);
        computeStageEntriesAndExits(LDI);
        addLocalSwitches(LDI);
        return collectQueueInfo(LDI) && collectEnvInfo(LDI) && configureDependencyStorage(LDI);
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
        }
        for (auto B : stageInfo->sccBBs)
        {
          stageInfo->sccBBCloneMap[B] = BasicBlock::Create(context, "", stageInfo->sccStage);
        }

        /*
         * Attach SCC instructions to their basic blocks in correct relative order
         */
        for (auto bbPair : stageInfo->sccBBCloneMap)
        {
          auto originalBB = bbPair.first;
          IRBuilder<> builder(bbPair.second);
          auto terminator = originalBB->getTerminator();
          for (auto &I : *originalBB)
          {
            auto cloneIter = stageInfo->iCloneMap.find(&I);
            if (cloneIter == stageInfo->iCloneMap.end()) continue;
            if (terminator == cloneIter->second) continue;
            stageInfo->iCloneMap[&I] = builder.Insert(cloneIter->second);
          }
        }
      }

      void linkEnvironmentDependencies (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        IRBuilder<> entryBuilder(stageInfo->entryBlock);
        IRBuilder<> exitBuilder(stageInfo->exitBlock);
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
          auto envVar = accessEnvVarFromIndex(outgoingEnvPair.second, exitBuilder);
          auto outgoingDepClone = stageInfo->iCloneMap[outgoingEnvPair.first];
          exitBuilder.CreateStore(outgoingDepClone, envVar);
        }

        /*
         * Load (outside of loop -> SCC) dependencies from the environment array 
         */
        for (auto incomingEnvPair : stageInfo->incomingToEnvMap)
        {
          auto envVar = accessEnvVarFromIndex(incomingEnvPair.second, entryBuilder);
          auto envLoad = entryBuilder.CreateLoad(envVar);

          Value *incomingDepValue = cast<Value>(incomingEnvPair.first);
          auto incomingDepClone = stageInfo->iCloneMap[incomingEnvPair.first];
          for (auto &depOp : incomingDepClone->operands())
          {
            if (depOp != incomingDepValue) continue;
            depOp.set(envLoad);
          }
        }
      }

      void remapLocalAndEnvOperandsOfInstClones (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        linkEnvironmentDependencies(LDI, stageInfo);

        /*
         * IMPROVEMENT: Ignore special cases upfront. If a clone of a general case is not found, abort with a corresponding error 
         */
        auto &iCloneMap = stageInfo->iCloneMap;
        for (auto ii = iCloneMap.begin(); ii != iCloneMap.end(); ++ii) {
          auto cloneInstruction = ii->second;

          for (auto &op : cloneInstruction->operands()) {
            auto opV = op.get();
            if (auto opI = dyn_cast<Instruction>(opV)) {
              auto iCloneIter = iCloneMap.find(opI);
              if (iCloneIter != iCloneMap.end()) {
                op.set(iCloneMap[opI]);
              }
              continue;
            }
            // Add cases such as constants where no clone needs to exist. Abort with an error if no such type is found
          }
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
          auto queueCast = entryBuilder.CreateBitCast(queuePtr, PointerType::getUnqual(queueType));

          auto queueInstrs = std::make_unique<QueueInstrs>();
          queueInstrs->queuePtr = entryBuilder.CreateLoad(queueCast);
          queueInstrs->alloca = entryBuilder.CreateAlloca(queueInfo->dependentType);
          queueInstrs->allocaCast = entryBuilder.CreateBitCast(queueInstrs->alloca, PointerType::getUnqual(int8));
          stageInfo->queueInstrMap[queueIndex] = std::move(queueInstrs);
        };

        for (auto queueIndex : stageInfo->pushValueQueues) loadQueuePtrFromIndex(queueIndex);
        for (auto queueIndex : stageInfo->popValueQueues) loadQueuePtrFromIndex(queueIndex);
        for (auto queueIndex : stageInfo->pushControlQueues) loadQueuePtrFromIndex(queueIndex);
        for (auto queueIndex : stageInfo->popControlQueues) loadQueuePtrFromIndex(queueIndex);
        for (auto queueIndex : stageInfo->pushSwitchQueues) loadQueuePtrFromIndex(queueIndex);
        for (auto queueIndex : stageInfo->popSwitchQueues) loadQueuePtrFromIndex(queueIndex);
      }

      void loadLocalSwitchIndices (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        IRBuilder<> entryBuilder(stageInfo->entryBlock);

        for (auto &consumerSwitch : stageInfo->consumerToLocalSwitches)
        {
          auto &localSwitch = consumerSwitch.second;
          auto alloca = entryBuilder.CreateAlloca(int32);
          entryBuilder.CreateStore(ConstantInt::get(int32, localSwitch->defaultEntry), alloca);
          localSwitch->indexTracker = entryBuilder.CreateLoad(alloca);
        }
      }

      void popControlAndSwitchQueuesInPrologue (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        IRBuilder<> prologueBuilder(stageInfo->prologueBlock);

        auto createQueueLoad = [&](int queueIndex) -> void {
          auto queueInfo = LDI->queues[queueIndex].get();
          auto queueInstrs = stageInfo->queueInstrMap[queueIndex].get();
          auto queueCallArgs = ArrayRef<Value*>({ queueInstrs->queuePtr, queueInstrs->allocaCast, ConstantInt::get(int32, queueInfo->byteLength) });
          queueInstrs->queueCall = prologueBuilder.CreateCall(queuePopTemporary, queueCallArgs);
          queueInstrs->load = prologueBuilder.CreateLoad(queueInstrs->alloca);
        };

        for (auto queueIndex : stageInfo->popControlQueues) createQueueLoad(queueIndex);
        for (auto queueIndex : stageInfo->popSwitchQueues) createQueueLoad(queueIndex);
      }

      void branchOnControls (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        if (stageInfo->popControlQueues.size() == 0) return;

        IRBuilder<> prologueBuilder(stageInfo->prologueBlock);
        auto &context = LDI->function->getContext();
        auto stageF = stageInfo->sccStage;
        for (auto queueIndex : stageInfo->popControlQueues)
        {
          auto switchBlock = BasicBlock::Create(context, "", stageF);
          stageInfo->controlToSwitchBlock[queueIndex] = switchBlock;
          
          /*
           * Find top-most dominating consumer block to enter from switch block
           */
          auto queueInfo = LDI->queues[queueIndex].get();
          BasicBlock *dominatingBB = (*queueInfo->consumers.begin())->getParent();
          for (auto consumerI = ++(queueInfo->consumers.begin()); consumerI != queueInfo->consumers.end(); ++consumerI)
          {
            auto consumerBB = (*consumerI)->getParent();
            if (LDI->DT.dominates(consumerBB, dominatingBB)) dominatingBB = consumerBB;
          }
          stageInfo->switchToSCCEntry[switchBlock] = dominatingBB;
        }

        auto firstIndex = *stageInfo->popControlQueues.begin();
        auto &firstQueueInstrs = stageInfo->queueInstrMap[firstIndex];
        auto caseValue = prologueBuilder.CreateZExt(firstQueueInstrs->load, int32);

        if (stageInfo->popControlQueues.size() == 1)
        {
          auto weightedLoad = prologueBuilder.CreateMul(caseValue, ConstantInt::get(int32, firstIndex + 1));
          auto ctrlSwitch = prologueBuilder.CreateSwitch(caseValue, stageInfo->epilogueBlock, 2);
          ctrlSwitch->addCase(ConstantInt::get(int32, firstIndex + 1), stageInfo->controlToSwitchBlock[firstIndex]);
          return;
        }

        for (auto queueIter = ++(stageInfo->popControlQueues.begin()); queueIter != stageInfo->popControlQueues.end(); ++queueIter)
        {
          auto queueIndex = *queueIter;
          auto &queueInstrs = stageInfo->queueInstrMap[queueIndex];
          auto zextLoad = prologueBuilder.CreateZExt(queueInstrs->load, int32);
          auto weightedLoad = prologueBuilder.CreateMul(zextLoad, ConstantInt::get(int32, queueIndex + 1));
          caseValue = prologueBuilder.CreateAdd(caseValue, weightedLoad);
        }

        auto ctrlSwitch = prologueBuilder.CreateSwitch(caseValue, stageInfo->epilogueBlock, stageInfo->popControlQueues.size() + 1);
        for (auto controlToSwitchBlock : stageInfo->controlToSwitchBlock)
        {
          ctrlSwitch->addCase(ConstantInt::get(int32, controlToSwitchBlock.first + 1), controlToSwitchBlock.second);
        }
      }

      void branchOnSwitches (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        IRBuilder<> prologueBuilder(stageInfo->prologueBlock);
        auto &context = LDI->function->getContext();
        auto stageF = stageInfo->sccStage;

        for (auto switchMap : stageInfo->switchToSCCEntry)
        {
          IRBuilder<> switchBuilder(switchMap.first);

          auto originalConsumerBB = switchMap.second;
          auto consumerBB = stageInfo->sccBBCloneMap[originalConsumerBB];
          Instruction *consumer = nullptr;
          PHINode *consumerPHI;
          for (auto &consumerI : *originalConsumerBB)
          {
            /*
             * TODO: ASSERT ONLY ONE CONSUMER GETS SWITCHED ON (TO AVOID DUPLICATES IN SUM OVER SWITCHES) 
             */
            if (stageInfo->consumerToQueues.find(&consumerI) == stageInfo->consumerToQueues.end()) continue;
            if (auto phi = dyn_cast<PHINode>(&consumerI))
            {
              consumer = &consumerI;
              consumerPHI = phi;
            }
            break;
          }

          std::set<Value *> switchVals;
          auto cSwitchIter = stageInfo->consumerToLocalSwitches.find(consumer);
          if (cSwitchIter != stageInfo->consumerToLocalSwitches.end())
          {
            switchVals.insert(cSwitchIter->second->indexTracker);
          }
          for (auto queueIndex : stageInfo->consumerToQueues[consumer])
          {
            if (stageInfo->popSwitchQueues.find(queueIndex) == stageInfo->popSwitchQueues.end()) continue;
            switchVals.insert(stageInfo->queueInstrMap[queueIndex]->load);
          }

          if (switchVals.size() == 0)
          {
            switchBuilder.CreateBr(consumerBB);
            continue;
          }

          Value *caseValue = *switchVals.begin();
          for (auto switchValI = ++(switchVals.begin()); switchValI != switchVals.end(); ++switchValI)
          {
            caseValue = prologueBuilder.CreateAdd(caseValue, *switchValI);
          }
          auto entrySwitch = switchBuilder.CreateSwitch(caseValue, stageInfo->abortBlock, switchVals.size() + 1);

          int switchIndex = 1;
          auto newConsumer = cast<PHINode>(stageInfo->iCloneMap[consumer]);
          while (switchIndex <= consumerPHI->getNumIncomingValues())
          {
            auto intermBB = BasicBlock::Create(context, "", stageF);
            stageInfo->switchToIntermediates[switchMap.first].insert(intermBB);

            newConsumer->setIncomingBlock(switchIndex - 1, intermBB);
            entrySwitch->addCase(ConstantInt::get(int32, switchIndex++), intermBB);
            IRBuilder<> intermBuilder(intermBB);
            intermBuilder.CreateBr(consumerBB);
          }
        }
      }

      void createQueueCall (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo, int queueIndex, IRBuilder<> builder)
      {
        auto queueInfo = LDI->queues[queueIndex].get();
        auto queueInstrs = stageInfo->queueInstrMap[queueIndex].get();
        auto queueCallArgs = ArrayRef<Value*>({ queueInstrs->queuePtr, queueInstrs->allocaCast, ConstantInt::get(int32, queueInfo->byteLength) });
        auto call = builder.CreateCall(queuePushTemporary, queueCallArgs);
      }

      void popValueQueuesInSCCOrEpilogue (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        auto createQueueLoad = [&](int queueIndex, IRBuilder<> builder) -> void {
          auto queueInfo = LDI->queues[queueIndex].get();
          auto queueInstrs = stageInfo->queueInstrMap[queueIndex].get();
          auto queueCallArgs = ArrayRef<Value*>({ queueInstrs->queuePtr, queueInstrs->allocaCast, ConstantInt::get(int32, queueInfo->byteLength) });
          queueInstrs->queueCall = builder.CreateCall(queuePopTemporary, queueCallArgs);
          queueInstrs->load = builder.CreateLoad(queueInstrs->alloca);
        };

        for (auto queueIndex : stageInfo->popValueQueues)
        {
          IRBuilder<> epilogueBuilder(stageInfo->epilogueBlock);
          createQueueLoad(queueIndex, epilogueBuilder);

          auto queueInfo = LDI->queues[queueIndex].get();
          auto dominatingC = (*queueInfo->consumers.begin());
          auto dominatingBB = dominatingC->getParent();
          for (auto consumerIter = ++(queueInfo->consumers.begin()); consumerIter != queueInfo->consumers.end(); ++consumerIter)
          {
            auto consumer = (*consumerIter);
            auto consumerBB = consumer->getParent();
            if (LDI->DT.dominates(consumerBB, dominatingBB))
            {
              dominatingBB = consumerBB;
              dominatingC = consumer;
              continue;
            }
            if (!LDI->DT.dominates(dominatingBB, consumerBB))
            {
              dominatingBB = nullptr;
              break;
            }
          }

          if (dominatingBB == nullptr)
          {
            IRBuilder<> prologueBuilder(stageInfo->prologueBlock);
            createQueueLoad(queueIndex, prologueBuilder);
            continue;
          }

          /*
           * Place pop in predecessor of PHINode; no instruction can precede PHINodes in a basic block
           */
          if (auto consumerPHI = dyn_cast<PHINode>(dominatingC))
          {
            auto producer = cast<Value>(queueInfo->producer);
            for (auto bb : consumerPHI->blocks())
            {
              if (consumerPHI->getIncomingValueForBlock(bb) != producer) continue;
              auto phiClone = cast<PHINode>(stageInfo->iCloneMap[dominatingC]);
              dominatingBB = phiClone->getIncomingBlock(consumerPHI->getBasicBlockIndex(bb));
              break;
            }
          }
          else
          {
            dominatingBB = stageInfo->sccBBCloneMap[dominatingBB];
          }

          IRBuilder<> builder(dominatingBB);
          createQueueLoad(queueIndex, builder);
        }
      }

      void remapValueConsumerOperands (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        for (auto queueIndex : stageInfo->popValueQueues)
        {
          auto queueInfo = LDI->queues[queueIndex].get();
          auto producer = cast<Value>(queueInfo->producer);
          auto load = stageInfo->queueInstrMap[queueIndex]->load;
          for (auto consumer : queueInfo->consumers)
          {
            for (auto &op : stageInfo->iCloneMap[consumer]->operands())
            {
              auto opV = op.get();
              if (opV != producer) continue;
              op.set(load);
            }
          }
        }
      }

      void pushControlQueues (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        IRBuilder<> prologueBuilder(stageInfo->prologueBlock);
        IRBuilder<> epilogueBuilder(stageInfo->epilogueBlock);

        for (auto queueIndex : stageInfo->pushControlQueues)
        {
          auto queueInstrs = stageInfo->queueInstrMap[queueIndex].get();
          prologueBuilder.CreateStore(ConstantInt::get(int1, 0), queueInstrs->alloca);

          auto queueInfo = LDI->queues[queueIndex].get();
          auto pBB = queueInfo->producer->getParent();
          auto exampleConsumerBB = (*queueInfo->consumers.begin())->getParent();

          BasicBlock *succBB = nullptr;
          for (auto succIter = succ_begin(pBB); succIter != succ_end(pBB); ++succIter)
          {
            succBB = *succIter;
            if (LDI->PDT.dominates(exampleConsumerBB, succBB)) break;
          }

          // FIX: Might need to create empty basic block clone of succBB, might not need to
          auto succBBClone = stageInfo->sccBBCloneMap[succBB];
          IRBuilder<> builder(succBBClone);
          auto store = builder.CreateStore(ConstantInt::get(int1, 1), queueInstrs->alloca);
        }

        for (auto queueIndex : stageInfo->pushControlQueues) createQueueCall(LDI, stageInfo, queueIndex, epilogueBuilder);
      }

      void pushValueQueues (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        IRBuilder<> epilogueBuilder(stageInfo->epilogueBlock);

        for (auto queueIndex : stageInfo->pushValueQueues)
        {
          auto queueInstrs = stageInfo->queueInstrMap[queueIndex].get();
          auto queueInfo = LDI->queues[queueIndex].get();
          auto pClone = stageInfo->iCloneMap[queueInfo->producer];
          auto pCloneBB = pClone->getParent();

          IRBuilder<> builder(pCloneBB);
          auto store = builder.CreateStore(pClone, queueInstrs->alloca);
        }

        for (auto queueIndex : stageInfo->pushValueQueues) createQueueCall(LDI, stageInfo, queueIndex, epilogueBuilder);
      }

      void pushSwitchQueues (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        IRBuilder<> epilogueBuilder(stageInfo->epilogueBlock);

        for (auto queueIndex : stageInfo->pushSwitchQueues)
        {
          auto queueInstrs = stageInfo->queueInstrMap[queueIndex].get();
          auto queueInfo = LDI->queues[queueIndex].get();
          auto consumer = *queueInfo->consumers.begin();
          auto pClone = stageInfo->iCloneMap[queueInfo->producer];
          auto pCloneBB = pClone->getParent();
          IRBuilder<> builder(pCloneBB);
          builder.CreateStore(ConstantInt::get(int32, queueInfo->consumerToPushIndex[consumer]), queueInstrs->alloca);
        }

        for (auto queueIndex : stageInfo->pushSwitchQueues) createQueueCall(LDI, stageInfo, queueIndex, epilogueBuilder);

        for (auto &consumerSwitch : stageInfo->consumerToLocalSwitches)
        {
          for (auto prodInd : consumerSwitch.second->producerToPushIndex)
          {
            auto pClone = stageInfo->iCloneMap[prodInd.first];
            auto pCloneBB = pClone->getParent();
            IRBuilder<> builder(pCloneBB);
            builder.CreateStore(ConstantInt::get(int32, prodInd.second), consumerSwitch.second->indexTracker);
          }
        }
      }

      void movePrologueTerminatorsToEnd (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        std::set<BasicBlock *> scanBBs;
        scanBBs.insert(stageInfo->prologueBlock);
        for (auto indSwitch : stageInfo->controlToSwitchBlock)
        {
          scanBBs.insert(indSwitch.second);
          for (auto intermBB : stageInfo->switchToIntermediates[indSwitch.second]) scanBBs.insert(intermBB);
        }

        for (auto BB : scanBBs)
        {
          // BB->print(errs() << "Scanning\n"); errs() << "\n";
          std::vector<Instruction *> instToMove;
          TerminatorInst *terminator; 
          for (auto instIter = BB->begin(); instIter != BB->end(); ++instIter)
          {
            if (terminator = dyn_cast<TerminatorInst>(&*instIter))
            {
              // terminator->print(errs() << "Inserting\t"); errs() << "\n";
              for (auto moveIter = ++instIter; moveIter != BB->end(); ++moveIter)
              {
                instToMove.push_back(&*moveIter);
              }
              break;
            }
          }

          for (auto I : instToMove)
          {
            // I->print(errs() << "Moving\t"); errs() << "\n";
            I->removeFromParent();
            I->insertBefore(terminator);
          }
        }

        errs() << "Done\n";
      }

      void remapControlFlowForSCC (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        auto &context = LDI->function->getContext();
        auto stageF = stageInfo->sccStage;

        for (auto sccBB : stageInfo->sccBBs)
        {
          std::vector<BasicBlock *> successorClones;
          for (auto sccSuccIter = succ_begin(sccBB); sccSuccIter != succ_end(sccBB); ++sccSuccIter)
          {
            BasicBlock *nextBB = nullptr;
            std::queue<BasicBlock *> descendants;
            descendants.push(*sccSuccIter);
            // (*sccSuccIter)->print(errs() << "Checking"); errs() << "\n";
            while (!descendants.empty())
            {
              auto bb = descendants.front();
              // bb->print(errs() << "Discovering descendant"); errs() << "\n";
              descendants.pop();
              if (!LDI->PDT.dominates(bb, *sccSuccIter)) continue;
              if (stageInfo->sccBBs.find(bb) != stageInfo->sccBBs.end())
              {
                // errs() << "Is the nearest post dominator";
                nextBB = bb;
                break;
              }

              for (auto succIter = succ_begin(bb); succIter != succ_end(bb); ++succIter) descendants.push(*succIter);
            }

            auto succClone = nextBB == nullptr ? stageInfo->epilogueBlock : stageInfo->sccBBCloneMap[nextBB];
            if (succClone == stageInfo->sccBBCloneMap[LDI->loop->getHeader()]) succClone = stageInfo->epilogueBlock;
            successorClones.push_back(succClone);
          }

          /*
           * If the terminator is not internal, exactly one successor will be recognized
           * If none are, then exit to the epilogue 
           */
          if (!stageInfo->scc->isInternal(sccBB->getTerminator()))
          {
            IRBuilder<> builder(stageInfo->sccBBCloneMap[sccBB]);
            bool found = false;
            for (auto succ : successorClones)
            {
              if (succ == stageInfo->epilogueBlock) continue;
              builder.CreateBr(succ);
              found = true;
              break;
            }
            if (!found) builder.CreateBr(stageInfo->epilogueBlock);
            continue;
          }

          auto termClone = cast<TerminatorInst>(stageInfo->iCloneMap[sccBB->getTerminator()]);
          for (auto succInd = 0; succInd < successorClones.size(); ++succInd)
          {
            termClone->setSuccessor(succInd, successorClones[succInd]);
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
        stageInfo->abortBlock = BasicBlock::Create(context, "", stageF);
        stageInfo->prologueBlock = BasicBlock::Create(context, "", stageF);
        stageInfo->epilogueBlock = BasicBlock::Create(context, "", stageF);

        /*
         * SCC iteration
         */
        createInstAndBBForSCC(LDI, stageInfo);
        remapLocalAndEnvOperandsOfInstClones(LDI, stageInfo);

        /*
         * Preparation for current iteration
         */
        loadAllQueuePointersInEntry(LDI, stageInfo);
        loadLocalSwitchIndices(LDI, stageInfo);

        popControlAndSwitchQueuesInPrologue(LDI, stageInfo);
        branchOnControls(LDI, stageInfo);
        branchOnSwitches(LDI, stageInfo);

        popValueQueuesInSCCOrEpilogue(LDI, stageInfo);
        remapValueConsumerOperands(LDI, stageInfo);

        /*
         * Preparation for next iteration
         */
        pushControlQueues(LDI, stageInfo);
        pushValueQueues(LDI, stageInfo);
        pushSwitchQueues(LDI, stageInfo);

        movePrologueTerminatorsToEnd(LDI, stageInfo);
        remapControlFlowForSCC(LDI, stageInfo);

        IRBuilder<> entryB(stageInfo->entryBlock);
        entryB.CreateBr(stageInfo->prologueBlock);
        IRBuilder<> epilogueB(stageInfo->epilogueBlock);
        epilogueB.CreateBr(stageInfo->exitBlock);

        /*
         * Cleanup
         */
        //sendKillControl(LDI, stageInfo);
        IRBuilder<> exitBuilder(stageInfo->exitBlock);
        exitBuilder.CreateRetVoid();
        stageF->print(errs() << "Function printout:\n"); errs() << "\n";
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
          (*edgeI)->print(errs());
        }
        errs() << "Number of edges: " << std::distance(sccSubgraph->begin_edges(), sccSubgraph->end_edges()) << "\n";
      }

      void printStageEntriesAndExits (LoopDependenceInfo *LDI)
      {
        for (auto &stage : LDI->stages)
        {
          errs() << "Stage:\t" << stage->order << "\n";
          for (auto entry : stage->sccEntries)
          {
            entry->print(errs() << "Entry:\n"); errs() << "\n";
          }
          for (auto exit : stage->sccExits)
          {
            exit->print(errs() << "Exit:\n"); errs() << "\n";
          }
        }
      }

      void printStageSCCs (LoopDependenceInfo *LDI)
      {
        for (auto &stage : LDI->stages)
        {
          errs() << "Stage: " << stage->order << "\n";
          stage->scc->print(errs() << "SCC:\n") << "\n";
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
          errs() << "\nPush control queues: ";
          for (auto qInd : stage->pushControlQueues) errs() << qInd << " ";
          errs() << "\nPop control queues: ";
          for (auto qInd : stage->popControlQueues) errs() << qInd << " ";
          errs() << "\nPush value switch queues: ";
          for (auto qInd : stage->pushSwitchQueues) errs() << qInd << " ";
          errs() << "\nPop value switch queues: ";
          for (auto qInd : stage->popSwitchQueues) errs() << qInd << " ";
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

      void printLocalSwitches (LoopDependenceInfo *LDI)
      {
        for (auto &stage : LDI->stages)
        {
          for (auto &cSwitch : stage->consumerToLocalSwitches)
          {
            errs() << "Local Switch:\tDefault index: " << cSwitch.second->defaultEntry << "\n";
            cSwitch.first->print(errs() << "Consumer PHI:\t"); errs() << "\n";
            for (auto &prodInd : cSwitch.second->producerToPushIndex)
            {
              prodInd.first->print(errs() << "Producer:\t"); errs() << "\tIndex: " << prodInd.second << "\n";
            }
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
