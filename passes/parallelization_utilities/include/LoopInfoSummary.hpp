#pragma once

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace llvm;

namespace llvm {

    struct LoopSummary {
    	int id;
    	LoopSummary *parent;
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
  			loops.insert(std::move(lSummary));

  			for (auto &loop : loops) {
  				if (loop->id == id) return loop.get();
  			}
  			return nullptr;
  		}

  		void populate(LoopInfo &li, Loop *l) {
			std::unordered_map<Loop *, LoopSummary *> loopToSummary;

			/*
			 * Summarize information about each loop
			 */
  			loopToSummary[l] = this->createSummary(l);
  			loopToSummary[l]->parent = nullptr;
  			topLoop = loopToSummary[l];
  			for (auto subLoop : l->getSubLoops()) {
  				loopToSummary[subLoop] = this->createSummary(subLoop);
  			}

  			/*
  			 * Associate loops with their parents
  			 */
  			for (auto subLoop : l->getSubLoops()) {
  				auto lSum = loopToSummary[subLoop];
  				auto parLoop = subLoop->getParentLoop();
  				auto inSummary = loopToSummary.find(parLoop) != loopToSummary.end();
  				lSum->parent = (inSummary ? loopToSummary[parLoop] : nullptr);
  			}
  		}
    };
}
