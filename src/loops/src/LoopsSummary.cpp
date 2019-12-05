/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopsSummary.hpp"

using namespace llvm;
      
LoopSummary * LoopsSummary::getLoop (Instruction *instIncludedInLoop){
  auto instBB = instIncludedInLoop->getParent();
  auto l = this->getLoop(instBB);
  return l;
}

LoopSummary * LoopsSummary::getLoop (BasicBlock *bbIncludedInLoop){
  auto lIter = this->bbToLoop.find(bbIncludedInLoop);
  if (lIter == this->bbToLoop.end()){
    return nullptr;
  }
  auto l = this->bbToLoop[bbIncludedInLoop];
  return l;
}

LoopSummary * LoopsSummary::createSummary (Loop *l) {
  auto lSummary = std::make_shared<LoopSummary>(l);
  auto lPtr = lSummary.get();
  for (auto bb : l->blocks()) {
    bbToLoop[bb] = lPtr;
  }
  auto ls = this->loops.insert(std::move(lSummary)).first->get();

  return ls;
}
      
void LoopsSummary::populate (LoopInfo &li, Loop *loop) {
  std::unordered_map<Loop *, LoopSummary *> loopToSummary;
  loopToSummary[loop->getParentLoop()] = nullptr;

  /*
   * NOTE(angelo): subloops only include 1-level deep loops
   *  entirely contained within the top level loop
   */
  std::queue<Loop *> toSummarize;
  toSummarize.push(loop);

  while (!toSummarize.empty()) {

    /*
     * Fetch the current loop.
     */
    auto l = toSummarize.front();
    toSummarize.pop();

    /*
     * Create the summary of the current loop.
     */
    auto summary = this->createSummary(l);
    loopToSummary[l] = summary;

    /*
     * Set the parent loop.
     */
    auto parent = l->getParentLoop();
    assert(loopToSummary.find(parent) != loopToSummary.end());
    summary->parent = loopToSummary[parent];

    /*
     * Iterate over the subloops of the current loop.
     */
    for (auto subLoop : l->getSubLoops()) {
      toSummarize.push(subLoop);
    }
  }

  /*
   * Set the root of the tree
   */
  this->topLoop = loopToSummary[loop];

  return ;
}

void LoopsSummary::print (raw_ostream &stream) {
  stream << "Loop summaries:\n";
  for (auto &loop : loops) {
    loop->print(stream);
  }

  return ;
}
      
LoopSummary * LoopsSummary::getLoopNestingTreeRoot (void) const {
  return this->topLoop;
}
