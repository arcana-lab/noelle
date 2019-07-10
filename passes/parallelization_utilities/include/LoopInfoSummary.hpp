/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "LoopSummary.hpp"

using namespace std;
using namespace llvm;

namespace llvm {

  struct LoopInfoSummary {
    LoopSummary *topLoop;
    std::set<unique_ptr<LoopSummary>> loops;
    std::unordered_map<BasicBlock *, LoopSummary *> bbToLoop;

    LoopSummary *createSummary(Loop *l) {
      auto id = loops.size();
      auto lSummary = std::make_unique<LoopSummary>(id, l);
      auto lPtr = lSummary.get();
      for (auto bb : l->blocks()) bbToLoop[bb] = lPtr;
      return loops.insert(std::move(lSummary)).first->get();
    }

    void populate(LoopInfo &li, Loop *loop) {
      std::unordered_map<Loop *, LoopSummary *> loopToSummary;
      loopToSummary[loop->getParentLoop()] = nullptr;

      /*
       * NOTE(angelo): subloops only include 1-level deep loops
       *  entirely contained within the top level loop
       */
      std::queue<Loop *> toSummarize;
      toSummarize.push(loop);

      while (!toSummarize.empty()) {
        auto l = toSummarize.front();
        toSummarize.pop();

        auto summary = this->createSummary(l);
        loopToSummary[l] = summary;
        auto parent = l->getParentLoop();
        assert(loopToSummary.find(parent) != loopToSummary.end());
        summary->parent = loopToSummary[parent];

        for (auto subLoop : l->getSubLoops()) {
          toSummarize.push(subLoop);
        }
      }

      topLoop = loopToSummary[loop];
    }

    void print (raw_ostream &stream) {
      stream << "Loop summaries:\n";
      for (auto &loop : loops) {
        loop->print(stream);
      }
    }
  };
}
