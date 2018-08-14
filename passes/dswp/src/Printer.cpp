#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

#include "SCCDAG.hpp"
#include "DSWP.hpp"

void llvm::DSWP::printPartition (DSWPLoopDependenceInfo *LDI){
  LDI->partition.print(errs(), "DSWP:   ");
}

void llvm::DSWP::printStageSCCs (DSWPLoopDependenceInfo *LDI) {
  if (this->verbose <= Verbosity::Minimal){
    return ;
  }

  errs() << "DSWP:  Pipeline stages\n";
  for (auto &stage : LDI->stages) {
    errs() << "DSWP:    Stage: " << stage->order << "\n";
    for (auto scc : stage->stageSCCs) {
      scc->print(errs(), "DSWP:     ");
      errs() << "DSWP:    \n" ;
    }
  }

  return ;
}

void llvm::DSWP::printStageQueues (DSWPLoopDependenceInfo *LDI) {

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
  for (auto &stage : LDI->stages) {
    errs() << "DSWP:    Stage: " << stage->order << "\n";

    errs() << "DSWP:      Push value queues: ";
    for (auto qInd : stage->pushValueQueues) {
      errs() << qInd << " ";
    }
    errs() << "\n" ;

    errs() << "DSWP:      Pop value queues: ";
    for (auto qInd : stage->popValueQueues) {
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

void llvm::DSWP::printEnv (DSWPLoopDependenceInfo *LDI) {

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
  for (auto prod : LDI->environment.envProducers) {
    prod->print(errs() << "DSWP:    Outside the loop producer" << count++ << ":\t"); errs() << "\n";
  }

  return ;
}
