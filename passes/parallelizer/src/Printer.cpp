#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

#include "SCCDAG.hpp"
#include "Parallelizer.hpp"

void llvm::Parallelizer::printSCCs (SCCDAG *sccSubgraph){

  /*
   * Check if we should print.
   */
  if (this->verbose <= Verbosity::Minimal){
    return ;
  }

  /*
   * Print the SCCs.
   */
  for (auto sccI = sccSubgraph->begin_internal_node_map(); sccI != sccSubgraph->end_internal_node_map(); ++sccI) {

    /*
     * Fetch and print the current SCC.
     */
    errs() << "Parallelizer:   SCC"
    auto scc = sccI->first;
    scc->print(errs(), "Parallelizer:     ");
  }

  return ;
}

void llvm::Parallelizer::printLoop (Loop *loop)
{
  errs() << "Parallelizing the following loop\n" ;
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
