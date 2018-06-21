#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

#include "SCCDAG.hpp"
#include "DSWP.hpp"

void llvm::DSWP::printSCCs (SCCDAG *sccSubgraph){
  if (!this->verbose){
    return ;
  }
  errs() << "DSWP:  Print SCCDAG\nInternal SCCs\n";
  for (auto sccI = sccSubgraph->begin_internal_node_map(); sccI != sccSubgraph->end_internal_node_map(); ++sccI) {

    /*
     * Fetch and print the current SCC.
     */
    sccI->first->print(errs());
  }

  return ;
}

void llvm::DSWP::printLoop (Loop *loop)
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

void llvm::DSWP::printStageSCCs (DSWPLoopDependenceInfo *LDI)
{
  if (!this->verbose){
    return ;
  }
  errs() << "------------ STAGE'S SCCs PRINTOUT ------------\n";
  for (auto &stage : LDI->stages)
  {
    errs() << "Stage: " << stage->order << "\n";
    for (auto scc : stage->stageSCCs) scc->print(errs() << "SCC:\n") << "\n";
  }
  errs() << "------------ END STAGE'S SCCs PRINTOUT ------------\n\n";
}

void llvm::DSWP::printStageQueues (DSWPLoopDependenceInfo *LDI)
{
  if (!this->verbose){
    return ;
  }
  errs() << "------------ STAGE'S QUEUES PRINTOUT ------------\n";

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
  errs() << "------------ END STAGE'S QUEUES PRINTOUT ------------\n\n";
}

void llvm::DSWP::printEnv (DSWPLoopDependenceInfo *LDI)
{
  if (!this->verbose){
    return ;
  }
  errs() << "------------ ENVIRONMENT PRINTOUT ------------\n";
  int count = 1;
  for (auto prod : LDI->environment->envProducers)
  {
    prod->print(errs() << "Env producer" << count++ << ":\t"); errs() << "\n";
  }
  errs() << "------------ END ENVIRONMENT PRINTOUT ------------\n\n";
}
