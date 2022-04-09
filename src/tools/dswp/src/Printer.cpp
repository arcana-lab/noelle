/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/SCCDAG.hpp"
#include "DSWP.hpp"

using namespace llvm;
using namespace llvm::noelle;

void DSWP::printStageSCCs (LoopDependenceInfo *LDI) const {
  if (this->verbose == Verbosity::Disabled) {
    return ;
  }

  errs() << "DSWP:  Pipeline stages\n";
  for (auto techniqueTask : this->tasks) {
    auto task = (DSWPTask *)techniqueTask;
    errs() << "DSWP:    Stage: " << task->getID() << "\n";
    for (auto scc : task->stageSCCs) {
      errs() << "DSWP:    SCC\n";
      for (auto nodePair : scc->internalNodePairs()) {
        nodePair.second->print(errs() << "DSWP:    ") << "\n";
      }
      errs() << "DSWP:    \n" ;
    }
  }

  return ;
}

void DSWP::printStageQueues (LoopDependenceInfo *LDI) const {

  /*
   * Check if we should print.
   */
  if (this->verbose == Verbosity::Disabled) {
    return ;
  }

  /*
   * Print the IDs of the queues.
   */
  errs() << "DSWP:  Queues that connect the pipeline stages\n";
  for (auto techniqueTask : this->tasks) {
    auto task = (DSWPTask *)techniqueTask;
    errs() << "DSWP:    Stage: " << task->getID() << "\n";

    errs() << "DSWP:      Push value queues: ";
    for (auto qInd : task->pushValueQueues) {
      errs() << qInd << " ";
    }
    errs() << "\n" ;

    errs() << "DSWP:      Pop value queues: ";
    for (auto qInd : task->popValueQueues) {
      errs() << qInd << " ";
    }
    errs() << "\n";
  }

  /*
   * Print the queues.
   */
  int count = 0;
  for (auto &queue : this->queues) {
    errs() << "DSWP:    Queue: " << count++ << "\n";
    queue->producer->print(errs() << "DSWP:     Producer:\t"); errs() << "\n";
    for (auto consumer : queue->consumers) {
      consumer->print(errs() << "DSWP:     Consumer:\t"); errs() << "\n";
    }
  }

  return ;
}

void DSWP::printEnv (LoopDependenceInfo *LDI) const {

  /*
   * Check if we should print.
   */
  if (this->verbose == Verbosity::Disabled){
    return ;
  }

  /*
   * Fetch the environment of the loop
   */
  auto environment = LDI->getEnvironment();
  assert(environment != nullptr);

  /*
   * Print the environment.
   */
  errs() << "DSWP:  Environment\n";
  int count = 1;
  for (auto envIndex : environment->getEnvIndicesOfLiveInVars()) {
    environment->producerAt(envIndex)->print(errs() << "DSWP:    Pre loop env " << count++ << ", producer:\t");
    errs() << "\n";
  }
  for (auto envIndex : environment->getEnvIndicesOfLiveOutVars()) {
    environment->producerAt(envIndex)->print(errs() << "DSWP:    Post loop env " << count++ << ", producer:\t");
    errs() << "\n";
  }

  return ;
}

void DSWP::writeStageGraphsAsDot (LoopDependenceInfo &LDI) const {

  DG<DGString> stageGraph;
  std::set<DGString *> elements;
  std::unordered_map<DGNode<SCC> *, DGNode<DGString> *> sccToDescriptionMap;

  auto addNode = [&](std::string val, bool isInternal) -> DGNode<DGString> * {
    auto element = new DGString(val);
    elements.insert(element);
    return stageGraph.addNode(element, isInternal);
  };

  auto findTasks = [&](SCC *scc) -> std::pair<std::set<DSWPTask *>, std::set<DSWPTask *>> {
    std::set<DSWPTask *> tasksOwningSCC;
    std::set<DSWPTask *> tasksWithClonableSCC;
    for (auto techniqueTask : this->tasks) {
      auto task = (DSWPTask *)techniqueTask;
      if (task->stageSCCs.find(scc) != task->stageSCCs.end()) {
        tasksOwningSCC.insert(task);
      }
      if (task->clonableSCCs.find(scc) != task->clonableSCCs.end()) {
        tasksWithClonableSCC.insert(task);
      }
    }
    return std::make_pair(tasksOwningSCC, tasksWithClonableSCC);
  };

  auto sccManager = LDI.getSCCManager();
  auto sccdag = sccManager->getSCCDAG();
  for (auto sccNode : sccdag->getNodes()) {
    std::string sccDescription;
    raw_string_ostream ros(sccDescription);
    auto taskAssignments = findTasks(sccNode->getT());

    ros << "Task ids owning scc: ";
    for (auto owningTask : taskAssignments.first) {
      ros << owningTask->getID() << " ";
    }
    ros << "\n";

    ros << "Task ids using clonable scc: ";
    for (auto owningTask : taskAssignments.second) {
      ros << owningTask->getID() << " ";
    }
    ros << "\n";

    for (auto iNodePair : sccNode->getT()->internalNodePairs()) {
      iNodePair.first->print(ros);
      ros << "\n";
    }

    ros.flush();
    sccToDescriptionMap.insert(std::make_pair(sccNode, addNode(sccDescription, true)));
  }

  for (auto sccEdge : sccdag->getEdges()) {
    auto outgoingDesc = sccToDescriptionMap.at(sccEdge->getOutgoingNode())->getT();
    auto incomingDesc = sccToDescriptionMap.at(sccEdge->getIncomingNode())->getT();
    stageGraph.addEdge(outgoingDesc, incomingDesc);
  }

  DGPrinter::writeGraph<DG<DGString>, DGString>("dswpStagesForLoop_" + std::to_string(LDI.getID()) + ".dot", &stageGraph);
  for (auto elem : elements) delete elem;
}

void DSWP::writeStageQueuesAsDot (const LoopDependenceInfo &LDI) const {

  DG<DGString> queueGraph;
  std::set<DGString *> elements;

  /*
   * Add a stage's queue producer or consumer as a node to the graph
   */
  auto addNode = [&](int stageIndex, Instruction *I) -> DGNode<DGString> * {
    std::string queueDesc;
    raw_string_ostream ros(queueDesc);
    I->print(ros << "Stage: " + std::to_string(stageIndex) << "\n");
    ros.flush();
    auto element = new DGString(queueDesc);
    elements.insert(element);
    return queueGraph.addNode(element, true);
  };

  for (auto &queue : this->queues) {
    auto producerNode = addNode(queue->fromStage, queue->producer);
    for (auto consumerI : queue->consumers) {
      auto consumerNode = addNode(queue->toStage, consumerI);
      queueGraph.addEdge(producerNode->getT(), consumerNode->getT());
    }
  }

  DGPrinter::writeGraph<DG<DGString>, DGString>("dswpQueuesForLoop_" + std::to_string(LDI.getID()) + ".dot", &queueGraph);
  for (auto elem : elements) delete elem;
}

void DSWP::printStageClonedValues (const LoopDependenceInfo &LDI, int taskIndex) const {
  raw_ostream &stream = errs();
  auto task = (DSWPTask *)this->tasks[taskIndex];

  stream << "Basic Block mapping\n";
  for (auto origBB : task->getOriginalBasicBlocks()){
    origBB->printAsOperand(stream << "Original: "); stream << "\n";
    auto cloneBB = task->getCloneOfOriginalBasicBlock(origBB);
    cloneBB->printAsOperand(stream << "\tCloned: "); stream << "\n";
  }
  stream << "\n";

  stream << "Instruction mapping\n";
  for (auto origI : task->getOriginalInstructions()){
    origI->print(stream << "Original: "); stream << "\n";
    auto cloneI = task->getCloneOfOriginalInstruction(origI);
    cloneI->print(stream << "\tCloned: "); stream << "\n";
  }
  stream << "\n";

}
