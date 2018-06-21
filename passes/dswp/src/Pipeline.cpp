#include "DSWP.hpp"

using namespace llvm;

void DSWP::createStagesfromPartitionedSCCs (DSWPLoopDependenceInfo *LDI)
{
  auto topLevelSCCNodes = LDI->loopSCCDAG->getTopLevelNodes();
  unordered_map<int, StageInfo *> partitionToStage;

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
    if (LDI->removableSCCs.find(scc) == LDI->removableSCCs.end())
    {
      StageInfo *stage;
      int sccPartition = LDI->sccToPartition[scc];
      if (partitionToStage.find(sccPartition) != partitionToStage.end())
      {
        stage = partitionToStage[sccPartition];
        stage->stageSCCs.insert(scc);
      } else
      {
        LDI->stages.push_back(std::move(std::make_unique<StageInfo>(order++, scc)));
        stage = LDI->stages[order - 1].get();
        partitionToStage[sccPartition] = stage;
      }
      LDI->sccToStage[scc] = stage;
    }
  }
}

void DSWP::createPipelineStageFromSCC (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo, Parallelization &par) {
  auto M = LDI->function->getParent();
  auto stageF = cast<Function>(M->getOrInsertFunction("", stageType));
  auto &context = M->getContext();
  stageInfo->sccStage = stageF;
  stageInfo->entryBlock = BasicBlock::Create(context, "", stageF);
  stageInfo->exitBlock = BasicBlock::Create(context, "", stageF);
  stageInfo->sccBBCloneMap[LDI->preHeader] = stageInfo->entryBlock;

  for (auto exitBB : LDI->loopExitBlocks) {
    auto newExitBB = BasicBlock::Create(context, "", stageF);
    stageInfo->loopExitBlocks.push_back(newExitBB);
    IRBuilder<> builder(newExitBB);
    builder.CreateBr(stageInfo->exitBlock);
  }

  createInstAndBBForSCC(LDI, stageInfo);
  loadAllQueuePointersInEntry(LDI, stageInfo, par);
  popValueQueues(LDI, stageInfo, par);
  pushValueQueues(LDI, stageInfo, par);
  loadAndStoreEnv(LDI, stageInfo, par);

  remapControlFlow(LDI, stageInfo);
  remapOperandsOfInstClones(LDI, stageInfo);

  IRBuilder<> entryBuilder(stageInfo->entryBlock);
  entryBuilder.CreateBr(stageInfo->sccBBCloneMap[LDI->header]);
  IRBuilder<> exitBuilder(stageInfo->exitBlock);
  exitBuilder.CreateRetVoid();

  inlineQueueCalls(LDI, stageInfo);

  if (this->verbose){
    stageF->print(errs() << "Function printout:\n"); errs() << "\n";
  }
}
