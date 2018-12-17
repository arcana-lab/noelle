/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <thread>
#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "llvm/Analysis/LoopInfo.h"

#include "LoopDependenceInfo.hpp"

using namespace std;
using namespace llvm;

LoopDependenceInfo::LoopDependenceInfo(Function *f, PDG *fG, Loop *l, LoopInfo &li, PostDominatorTree &pdt)
		: function{f}, functionDG{fG}, DOALLChunkSize{8}, maximumNumberOfCoresForTheParallelization{std::thread::hardware_concurrency()}
  {

  /*
   * Create a LoopInfo summary
   */
  this->liSummary.populate(li, l);

  /*
   * Set the headers.
   */
  this->header = l->getHeader();
  this->preHeader = l->getLoopPreheader();

  /*
   * Set the loop body.
   */
  for (auto bb : l->blocks()){
    this->loopBBs.push_back(&*bb);
    loopBBtoPD[&*bb] = pdt.getNode(&*bb)->getIDom()->getBlock();
  }

  /*
   * Set the loop dependence graph.
   */
  this->loopDG = functionDG->createLoopsSubgraph(l);

  /*
   * Build a SCCDAG of loop-internal instructions
   */
  std::vector<Value *> loopInternals;
  for (auto internalNode : loopDG->internalNodePairs()) {
      loopInternals.push_back(internalNode.first);
  }
  loopInternalDG = loopDG->createSubgraphFromValues(loopInternals, false);
  loopSCCDAG = SCCDAG::createSCCDAGFrom(loopInternalDG);

  l->getExitBlocks(loopExitBlocks);

  environment = new LoopEnvironment(loopDG, loopExitBlocks);

  return ;
};
    
uint32_t LoopDependenceInfo::numberOfExits (void) const{

  /*
   * Fetch the number of exit blocks.
   */
  auto exits = this->loopExitBlocks.size();

  return exits;
}

void LoopDependenceInfo::createPDGs (void){
  return ;
}

llvm::LoopDependenceInfo::~LoopDependenceInfo() {
  delete loopDG;
  delete loopInternalDG;
  delete loopSCCDAG;
  delete environment;

  return ;
}
