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
using namespace llvm::noelle;
      
LoopsSummary::LoopsSummary (
  Loop *loop
  ) {
  std::unordered_map<Loop *, LoopStructure *> loopToSummary;
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
     * Fetch the parent loop.
     */
    auto parent = l->getParentLoop();
    assert(loopToSummary.find(parent) != loopToSummary.end());

    /*
     * Create the summary of the current loop.
     */
    auto parentStructure = loopToSummary[parent];
    auto childStructure = this->createSummary(l, parentStructure);
    loopToSummary[l] = childStructure;

    if (parentStructure) {
      parentStructure->addChild(childStructure);
    }

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
  assert(loopToSummary.find(loop) != loopToSummary.end());
  this->topLoop = loopToSummary[loop];

  return ;
}

LoopStructure * LoopsSummary::getLoop (Instruction &instIncludedInLoop) const {
  auto instBB = instIncludedInLoop.getParent();
  auto l = this->getLoop(*instBB);
  return l;
}

LoopStructure * LoopsSummary::getLoop (BasicBlock &bbIncludedInLoop) const {
  auto lIter = this->bbToLoop.find(&bbIncludedInLoop);
  if (lIter == this->bbToLoop.end()){
    return nullptr;
  }
  auto l = this->bbToLoop.at(&bbIncludedInLoop);
  return l;
}

LoopStructure * LoopsSummary::createSummary (
  Loop *l, 
  LoopStructure *parentLoop
  ) {

  /*
   * Fetch the loop header.
   */
  auto header = l->getHeader();

  /*
   * Allocate the LoopStructure
   */
  std::shared_ptr<LoopStructure> lSummary;
  lSummary = std::make_shared<LoopStructure>(l, parentLoop);

  /*
   * Create the map from basic blocks to loop.
   */
  auto lPtr = lSummary.get();
  for (auto bb : l->blocks()) {
    this->bbToLoop[bb] = lPtr;
  }
  auto ls = this->loops.insert(std::move(lSummary)).first->get();

  return ls;
}
      
void LoopsSummary::print (raw_ostream &stream) const {
  stream << "Loop summaries:\n";
  for (auto &loop : loops) {
    loop->print(stream);
  }

  return ;
}
      
LoopStructure * LoopsSummary::getLoopNestingTreeRoot (void) const {
  assert(this->topLoop != nullptr);

  return this->topLoop;
}
