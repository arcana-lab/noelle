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
        printSCCs(LDI->loopSCCDAG);

        /*
         * Create the pipeline stages.
         */
        if (!isWorthParallelizing(LDI)) return false;

        printStageQueues(LDI);
        return false;
        
        for (auto &stage : LDI->stages) createPipelineStageFromSCC(LDI, stage);

        /*
         * Create the pipeline (connecting the stages)
         */
        //createPipelineFromStages(LDI);
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

      void mergeSCCs (LoopDependenceInfo *LDI)
      {

        /*
         * Merge the SCC related to a single PHI node and its use if there is only one.
         */
        //TODO

        /*
         * Remove SCCs consisting solely of trailing branches or LCCSA PHINode
         */
        auto sccSubgraph = LDI->loopSCCDAG;
        std::vector<DGNode<SCC> *> nodesToRemove; 
        for (auto sccNode : make_range(sccSubgraph->begin_nodes(), sccSubgraph->end_nodes()))
        {
          auto scc = sccNode->getT();
          if (scc->numInternalNodes() > 1) continue ;

          auto singleInstrNode = *scc->begin_nodes();
          if (auto branch = dyn_cast<TerminatorInst>(singleInstrNode->getT()))
          {
            // scc->print(errs() << "Removing SCC because of trailing branch:\n") << "\n";
            nodesToRemove.push_back(sccNode);
          }
        }
        sccSubgraph->removeNodesFromSelf(nodesToRemove);

        return ;
      }

      int fetchOrCreateValueOrControlQueue (LoopDependenceInfo *LDI, StageInfo *fromStage, Instruction *producer, Instruction *consumer)
      {
        int queueIndex = LDI->queues.size();
        auto prodQueueIter = fromStage->producerToValueOrControlQueueMap.find(producer);
        if (prodQueueIter != fromStage->producerToValueOrControlQueueMap.end()) return prodQueueIter->second;

        LDI->queues.push_back(std::move(std::make_unique<QueueInfo>(producer, consumer)));
        return queueIndex;
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
              auto queueIndex = fetchOrCreateValueOrControlQueue(LDI, fromStage, producer, consumer);

              instructionEdge->print(errs()) << "\n";
              if (instructionEdge->isControlDependence())
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

              fromStage->producerToValueOrControlQueueMap[producer] = queueIndex;
              toStage->consumerToQueuesMap[consumer].insert(queueIndex);
            }
          }
        }
        return true;
      }

      int fetchOrCreateSwitchQueue (LoopDependenceInfo *LDI, StageInfo *fromStage, Instruction *producer, Instruction *consumer)
      {
        int queueIndex = LDI->queues.size();
        auto prodQueueIter = fromStage->producerToSwitchQueueMap.find(producer);
        if (prodQueueIter != fromStage->producerToSwitchQueueMap.end()) return prodQueueIter->second;

        LDI->queues.push_back(std::move(std::make_unique<QueueInfo>(producer, consumer, int32, false)));
        return queueIndex;
      }

      bool createSwitchQueues (LoopDependenceInfo *LDI)
      {
        for (auto &stage : LDI->stages)
        {
          errs() << "Sizes: " << stage->popValueQueues.size() << " " << stage->popControlQueues.size() << "\n";
          for (auto consumerQueues : stage->consumerToQueuesMap)
          {
            auto &queues = consumerQueues.second;
            consumerQueues.first->print(errs() << "Consumer\t"); errs() << "\n";
            if (auto consumerPHI = dyn_cast<PHINode>(consumerQueues.first))
            {
              for (auto queueIndex : queues)
              {
                if (stage->popValueQueues.find(queueIndex) == stage->popValueQueues.end()) continue;

                errs() << "\tQueue: " << queueIndex << "\n";
                auto queueInfo = LDI->queues[queueIndex].get();
                auto fromStage = LDI->stages[queueInfo->fromStage].get();
                auto toStage = LDI->stages[queueInfo->toStage].get();
                auto producerNode = fromStage->scc->fetchNodeOf(queueInfo->producer);

                auto createQueueFromProducer = [&](Instruction *producer) -> void {
                  auto queueIndex = fetchOrCreateSwitchQueue(LDI, fromStage, producer, consumerPHI);
                  fromStage->pushSwitchQueues.insert(queueIndex);
                  toStage->popSwitchQueues.insert(queueIndex);
                };

                /*
                 * Find all scc S that the producer control depends on, including self if applicable
                 * Create a queue from the stage of S to the stage of the consumer PHI
                 */
                bool isControlled = false;
                for (auto incomingEdge : make_range(producerNode->begin_incoming_edges(), producerNode->end_incoming_edges()))
                {
                  if (!incomingEdge->isControlDependence()) continue;
                  isControlled = true;
                  auto control = incomingEdge->getNodePair().first->getT();
                  createQueueFromProducer(cast<Instruction>(control));
                }
                
                if (!isControlled) createQueueFromProducer(queueInfo->producer);
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
          for (auto incomingNode : make_range(externalNode->begin_incoming_nodes(), externalNode->end_incoming_nodes()))
          {
            addExternalDependentToStagesWithInst(cast<Instruction>(incomingNode->getT()), true);
          }
          for (auto outgoingNode : make_range(externalNode->begin_outgoing_nodes(), externalNode->end_outgoing_nodes()))
          {
            addExternalDependentToStagesWithInst(cast<Instruction>(outgoingNode->getT()), false);
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

      void collectSCCIntoStages (LoopDependenceInfo *LDI)
      {
        auto topLevelSCCNodes = LDI->loopSCCDAG->getTopLevelNodes();

        /*
         * TODO: Check if all entries to the loop are into top level nodes
         */
        std::set<DGNode<SCC> *> nodesFound;
        std::deque<DGNode<SCC> *> nodesToTraverse(topLevelSCCNodes.begin(), topLevelSCCNodes.end());

        int order = 0;
        while (!nodesToTraverse.empty())
        {
          auto sccNode = nodesToTraverse.front();
          nodesToTraverse.pop_front();

          auto scc = sccNode->getT();
          auto stage = std::make_unique<StageInfo>();
          stage->order = order++;
          stage->scc = scc;
          LDI->stages.push_back(std::move(stage));
          LDI->sccToStage[scc] = LDI->stages[order - 1].get();

          /*
           * Add all unvisited, next depth nodes to the traversal queue 
           */
          auto outgoingNodesVec = sccNode->getOutgoingNodes();
          std::set<DGNode<SCC> *> outgoingNodes(outgoingNodesVec.begin(), outgoingNodesVec.end());
          for (auto outgoingNode : outgoingNodes)
          {
            if (nodesFound.find(outgoingNode) != nodesFound.end()) continue;

            bool isNextDepth = true;
            for (auto incoming : outgoingNode->getIncomingNodes())
            {
              isNextDepth &= (outgoingNodes.find(incoming) == outgoingNodes.end());
            }
            if (!isNextDepth) continue;

            nodesFound.insert(outgoingNode);
            nodesToTraverse.push_back(outgoingNode);
          }
        }
      }

      bool isWorthParallelizing (LoopDependenceInfo *LDI)
      {
        if (LDI->loopSCCDAG->numNodes() <= 1) return false;
        collectSCCIntoStages(LDI);
        return collectQueueInfo(LDI) && collectEnvInfo(LDI) && configureDependencyStorage(LDI);
      }

      void createPipelineStageFromSCC (LoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo)
      {
        auto M = LDI->function->getParent();
        auto stageF = cast<Function>(M->getOrInsertFunction("", stageType));
        auto &context = M->getContext();
        stageInfo->sccStage = stageF;
        stageInfo->entryBlock = BasicBlock::Create(context, "", stageF);
        stageInfo->exitBlock = BasicBlock::Create(context, "", stageF);
        stageInfo->prologueBlock = BasicBlock::Create(context, "", stageF);
        stageInfo->epilogueBlock = BasicBlock::Create(context, "", stageF);

        return;

        /*
         * SCC iteration
         */
        //createInstAndBBForSCC(LDI, stageInfo);
        //remapLocalAndEnvOperandsOfInstClones(LDI, stageInfo);

        /*
         * Preparation for current iteration
         */
        //loadAllQueuesInEntry(LDI, stageInfo);
        //popFromQueuesInPrologue(LDI, stageInfo);
        //enterSCCUsingSwitchQueues(LDI, stageInfo);

        /*
         * Preparation for next iteration
         */
        //pushToSwitchAndControlQueuesAfterSCC(LDI, stageInfo);
        //pushToSwitchAndControlQueuesAfterContinue(LDI, stageInfo);
        //pushValueQueuesInEpilogue(LDI, stageInfo);

        /*
         * Cleanup
         */
        //sendKillControl(LDI, stageInfo);
        IRBuilder<> exitBuilder(stageInfo->exitBlock);
        exitBuilder.CreateRetVoid();
        stageF->print(errs() << "Function printout:\n"); errs() << "\n";
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

      void printStageQueues(LoopDependenceInfo *LDI)
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
