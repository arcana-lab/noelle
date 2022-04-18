/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "HELIX.hpp"
#include "llvm/IR/CFG.h"
#include "noelle/core/SCCPartitionScheduler.hpp"

using namespace llvm;
using namespace llvm::noelle;

/*
 * Heuristic used: push furthest outlier instructions closer to the rest of the sequential segment
 * by moving between control flow equivalent sets of basic blocks
 */
void HELIX::squeezeSequentialSegment (
  LoopDependenceInfo *LDI,
  DataFlowResult *reachabilityDFR,
  SequentialSegment *ss
  ){

  /*
   * Fetch ControlFlowEquivalence and dependence graph
   * TODO: Move this to LDI
   */
  auto loops = LDI->getLoopHierarchyStructures();
  auto rootLoop = loops->getLoop();
  auto taskFunction = rootLoop->getHeader()->getParent();
  auto taskDG = LDI->getLoopDG();
  DominatorTree taskDT(*taskFunction);
  PostDominatorTree taskPDT(*taskFunction);
  DominatorSummary taskDS(taskDT, taskPDT);
  ControlFlowEquivalence cfe(&taskDS, loops, rootLoop);

  return ;
}

void HELIX::squeezeSequentialSegments (
  LoopDependenceInfo *LDI,
  std::vector<SequentialSegment *> *sss,
  DataFlowResult *reachabilityDFR
  ){

  auto sccdagAttribution = LDI->getSCCManager();
  auto sccdag = sccdagAttribution->getSCCDAG();
  std::unordered_set<SCCSet *> sccPartitions;
  for (auto ss : *sss) {
    auto ssPartition = new SCCSet();
    for (auto scc : ss->getSCCs()) {
      ssPartition->sccs.insert(scc);
    }
    sccPartitions.insert(ssPartition);
  }

  SCCPartitionScheduler scheduler(sccdag, sccPartitions, reachabilityDFR);
  scheduler.squeezePartitions();

  for (auto ssPartition : sccPartitions) {
    delete ssPartition;
  }

  return ;
}

void HELIX::scheduleSequentialSegments (
  LoopDependenceInfo *LDI,
  std::vector<SequentialSegment *> *sss,
  DataFlowResult *reachabilityDFR
  ){
  //TODO

  return ;
}
