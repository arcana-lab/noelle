#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

#include "SCCDAG.hpp"
#include "DSWP.hpp"

void llvm::DSWP::printPartition (DSWPLoopDependenceInfo *LDI) const {
  LDI->partition.print(errs(), "DSWP:   ");
}

void llvm::DSWP::printStageSCCs (DSWPLoopDependenceInfo *LDI) const {
  if (this->verbose <= Verbosity::Minimal){
    return ;
  }

  errs() << "DSWP:  Pipeline stages\n";
  for (auto techniqueWorker : this->workers) {
    auto worker = (DSWPTechniqueWorker *)techniqueWorker;
    errs() << "DSWP:    Stage: " << worker->order << "\n";
    for (auto scc : worker->stageSCCs) {
      scc->print(errs(), "DSWP:     ", /*maxEdges=*/15);
      errs() << "DSWP:    \n" ;
    }
  }

  return ;
}

void llvm::DSWP::printStageQueues (DSWPLoopDependenceInfo *LDI) const {

  /*
   * Check if we should print.
   */
  if (this->verbose <= Verbosity::Minimal){
    return ;
  }

  /*
   * Print the IDs of the queues.
   */
  errs() << "DSWP:  Queues that connect the pipeline stages\n";
  for (auto techniqueWorker : this->workers) {
    auto worker = (DSWPTechniqueWorker *)techniqueWorker;
    errs() << "DSWP:    Stage: " << worker->order << "\n";

    errs() << "DSWP:      Push value queues: ";
    for (auto qInd : worker->pushValueQueues) {
      errs() << qInd << " ";
    }
    errs() << "\n" ;

    errs() << "DSWP:      Pop value queues: ";
    for (auto qInd : worker->popValueQueues) {
      errs() << qInd << " ";
    }
    errs() << "\n";
  }

  /*
   * Print the queues.
   */
  int count = 0;
  for (auto &queue : LDI->queues) {
    errs() << "DSWP:    Queue: " << count++ << "\n";
    queue->producer->print(errs() << "DSWP:     Producer:\t"); errs() << "\n";
    for (auto consumer : queue->consumers) {
      consumer->print(errs() << "DSWP:     Consumer:\t"); errs() << "\n";
    }
  }

  return ;
}

void llvm::DSWP::printEnv (DSWPLoopDependenceInfo *LDI) const {

  /*
   * Check if we should print.
   */
  if (this->verbose <= Verbosity::Minimal){
    return ;
  }

  /*
   * Print the environment.
   */
  errs() << "DSWP:  Environment\n";
  int count = 1;
  for (auto envIndex : LDI->environment->getPreEnvIndices()) {
    LDI->environment->producerAt(envIndex)->print(errs()
      << "DSWP:    Pre loop env " << count++ << ", producer:\t");
    errs() << "\n";
  }
  for (auto envIndex : LDI->environment->getPostEnvIndices()) {
    LDI->environment->producerAt(envIndex)->print(errs()
      << "DSWP:    Post loop env " << count++ << ", producer:\t");
    errs() << "\n";
  }

  return ;
}
