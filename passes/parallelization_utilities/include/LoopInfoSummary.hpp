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

using namespace std;
using namespace llvm;

namespace llvm {

    struct LoopSummary {
    	int id;
    	LoopSummary *parent;
      std::set<LoopSummary *> children;
    	int depth;
      BasicBlock *header;
    	std::set<BasicBlock *> bbs;
      std::set<BasicBlock *> latchBBs;

    	LoopSummary(int id, Loop *l) {
    		this->id = id;
    		this->depth = l->getLoopDepth();
        this->header = l->getHeader();
    		for (auto bb : l->blocks()) {
          this->bbs.insert(bb);
          if (l->isLoopLatch(bb)) latchBBs.insert(bb);
        }
    	}

      void print (raw_ostream &stream) {
        stream << "Loop summary: " << id << ", depth: " << depth << "\n";
        header->begin()->print(stream); stream << "\n";
      }
    };

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

  		void populate(LoopInfo &li, Loop *l) {
        std::unordered_map<Loop *, LoopSummary *> loopToSummary;

        /*
         * Summarize information about each loop
         */
  			loopToSummary[l] = this->createSummary(l);
  			loopToSummary[l]->parent = nullptr;
  			topLoop = loopToSummary[l];

        /*
         * NOTE(angelo): subloops only include 1-level deep loops
         *  entirely contained within the top level loop
         */
  			for (auto subLoop : l->getSubLoops()) {
  				loopToSummary[subLoop] = this->createSummary(subLoop);
  			}

  			/*
         * TODO(angelo): Separate out populating parents and children
         *  from this pseudo constructor, promote above to constructor
  			 * Associate loops with their parents
  			 */
  			for (auto subLoop : l->getSubLoops()) {
  				auto lSum = loopToSummary[subLoop];
  				auto parLoop = subLoop->getParentLoop();
  				auto inSummary = loopToSummary.find(parLoop) != loopToSummary.end();
  				lSum->parent = (inSummary ? loopToSummary[parLoop] : nullptr);
  			}
  		}

      void print (raw_ostream &stream) {
        stream << "Loop summaries:\n";
        for (auto &loop : loops) {
          loop->print(stream);
        }
      }
    };
}
