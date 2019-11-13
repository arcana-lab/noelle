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

using namespace llvm ;

template <typename T>
bool reachesTargetFirst (
  T bbIterator,
  BasicBlock *startB,
  BasicBlock *targetB,
  BasicBlock *terminalB
){
  bool reachedTarget = false;

  std::queue<BasicBlock *> nexts;
  std::set<BasicBlock *> seen;
  nexts.push(startB);
  while (!nexts.empty()) {
    auto B = nexts.front();
    nexts.pop();

    if (seen.find(B) != seen.end()) continue;
    seen.insert(B);

    // B->printAsOperand(errs() << "Reach B:\n", false); errs() << "\n";

    if (B == targetB) {
      reachedTarget = true;
      continue;
    }
    if (B == terminalB) return false;

    for (auto next : bbIterator(B)) {
      nexts.push(next);
    }
  }

  assert(reachedTarget && "Basic Block traversal reached neither expected value\n");
  return true;
}

template <typename Forward, typename Backward, typename DomMap>
void traverseDomination (
  Forward forwardBBIter,
  Backward backwardBBIter,
  DomMap &domMap,
  std::set<BasicBlock *> startBs,
  BasicBlock *terminalB
){
  std::queue<BasicBlock *> workList;
  std::set<BasicBlock *> seenList;
  for (auto B : startBs) workList.push(B);

  while (!workList.empty()) {
    auto B = workList.front();
    workList.pop();

    if (seenList.find(B) != seenList.end()) continue;
    seenList.insert(B);

    // B->printAsOperand(errs() << "Traverse B: ", false); errs() << "\n";

    for (auto next : forwardBBIter(B)) {
      // next->printAsOperand(errs() << "Next B: ", false); errs() << "\n";
      if (reachesTargetFirst(backwardBBIter, next, B, terminalB)) {
        // errs() << "In Dom map\n";
        workList.push(next);
        domMap[next] = B;
      }
    }
  }
}

void collectIDomsAndIPostDoms (
  LoopDependenceInfo *LDI,
  std::unordered_map<BasicBlock *, BasicBlock *> &iDoms,
  std::unordered_map<BasicBlock *, BasicBlock *> &iPostDoms
){
  pred_range (*preds)(BasicBlock *) = predecessors;
  succ_range (*succs)(BasicBlock *) = successors;

  std::set<BasicBlock *> exitBBs;
  for (auto exitBB : LDI->loopExitBlocks) exitBBs.insert(exitBB);
  traverseDomination(preds, succs, iPostDoms, exitBBs, &*LDI->function->begin());

  std::set<BasicBlock *> startBBs = { LDI->header };
  traverseDomination(succs, preds, iDoms, startBBs, &*LDI->function->end());

  for (auto B : LDI->loopBBs) {
    // B->printAsOperand(errs() << "Basic block in loop: ", false); errs() << "\n";
    assert(iPostDoms.find(B) != iPostDoms.end()
      && "ERROR: Ad-hoc PDT analysis came up short on helix task's loop basic blocks");
    assert(iDoms.find(B) != iDoms.end()
      && "ERROR: Ad-hoc DT analysis came up short on helix task's loop basic blocks");
  }
}

/*
 * Not working yet but would improve performance greatly
 *

template <typename iDomMap, typename tDomMap, typename Iter>
void propagateTransitiveDomination (
  iDomMap &iDoms,
  tDomMap &tDoms,
  Iter bbIter,
  std::set<BasicBlock *> leafBs
){

  for (auto iDomPair : iDoms) {
    tDoms[iDomPair.first].insert(iDomPair.second);
  }

  std::queue<BasicBlock *> workList;
  for (auto B : leafBs) workList.push(B);

  while (!workList.empty()) {
    auto B = workList.front();
    workList.pop();

    std::set<BasicBlock *> transitives;
    for (auto domB : tDoms[B]) {
      auto domDoms = tDoms[domB];
      transitives.insert(domDoms.begin(), domDoms.end());
    }
    tDoms[B].insert(transitives.begin(), transitives.end());

    for (auto next : bbIter(B)) {
      workList.push(next);
    }
  }
}

*/

void HELIX::squeezeSequentialSegment (
  LoopDependenceInfo *LDI,
  SequentialSegment *ss
  ){

  //TODO
  return ;

  /*
   * HACK: The LLVM post dominator pass cannot be run on newly created bitcode, so we
   * will re-create a map of immediate post dominators for the basic blocks in question
   */
  // LDI->function->print(errs() << "Function up until now: \n");
  std::unordered_map<BasicBlock *, BasicBlock *> iDoms, iPostDoms;
  collectIDomsAndIPostDoms(LDI, iDoms, iPostDoms);

  /*
   * TODO: Propagate domination information through loop basic blocks
   */
  // std::unordered_map<BasicBlock *, std::set<BasicBlock *>> doms, postDoms;

  auto propagateThroughPDs = [&](SequentialSegment *ss, Instruction *I) -> void {
    // Collect instructions dependent on I within the sequential segment 
    std::set<Instruction *> dependents;
    for (auto scc : ss->getSCCs()) {
      if (!scc->isInternal(I)) continue;
      auto node = scc->fetchNode(I);
      for (auto edge : node->getOutgoingEdges()) {
        dependents.insert(cast<Instruction>(edge->getIncomingT()));
      }
    }

    std::set<BasicBlock *> dependentBBs;
    for (auto depI : dependents) dependentBBs.insert(depI->getParent());

    /*
     * Identify common dominator of dependents
     */
  };

  /*
   * TODO: For each SCC of each SS:
   * Ensure all externals are outside the sequential segment
   * whether before (incoming) or after (outgoing)
   * Why do that when you can just try to squeeze firsts and lasts
   * as close to each other as possible given their dependencies!
   */
    for (auto scc : ss->getSCCs()) {
      // Determine first and last instructions
      std::set<Value *> firsts, lasts;
      auto &backEdges = LDI->sccdagAttrs.interIterDeps[scc];
      for (auto edge : backEdges) {
        lasts.insert(edge->getOutgoingT());
        firsts.insert(edge->getIncomingT());
      }

      std::set<Value *> extractBefore, extractAfter;

      for (auto externalPair : scc->externalNodePairs()) {
        auto externalV = externalPair.first;
        if (externalPair.second->numOutgoingEdges() > 0) {
          // Confirm the external is before the first instructions in the SCC
          auto executesBefore = false;
          for (auto first : firsts) {
            // TODO: Determine execution order between first and externalV
          }
          if (!executesBefore) extractBefore.insert(externalV);
        } else {
          // Confirm the external is after the last instructions in the SCC
          auto executesAfter = false;
          for (auto last : lasts) {
            // TODO: Determine execution order between last and externalV
          }
          if (!executesAfter) extractAfter.insert(externalV);
        }
      }
    }
}

void HELIX::squeezeSequentialSegments (
  LoopDependenceInfo *LDI,
  std::vector<SequentialSegment *> *sss
  ){

  /*
   * Squeeze all sequential segments.
   */
  for (auto ss : *sss){
    this->squeezeSequentialSegment(LDI, ss);
  }

  return ;
}

void HELIX::scheduleSequentialSegments (
  LoopDependenceInfo *LDI,
  std::vector<SequentialSegment *> *sss
  ){
  //TODO

  return ;
}
