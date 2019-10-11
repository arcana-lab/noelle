/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "DSWP.hpp"

using namespace llvm;

void DSWP::collectLiveInEnvInfo (LoopDependenceInfo *LDI) {
  for (auto envIndex : LDI->environment->getEnvIndicesOfLiveInVars()) {
    auto producer = LDI->environment->producerAt(envIndex);

    for (auto consumer : LDI->environment->consumersOf(producer)) {
      bool isSharedInst = false;
      for (auto scc : LDI->sccdagAttrs.clonableSCCs) {
        if (!scc->isInternal(consumer)) continue;
        isSharedInst = true;
        for (auto i = 0; i < tasks.size(); ++i) {
          envBuilder->getUser(i)->addLiveInIndex(envIndex);
        }
        break;
      }

      if (!isSharedInst) {
        for (auto i = 0; i < this->tasks.size(); ++i) {
          auto task = (DSWPTask *)this->tasks[i];
          bool isInternal = false;
          for (auto scc : task->stageSCCs) isInternal |= scc->isInternal(consumer);
          if (isInternal) envBuilder->getUser(i)->addLiveInIndex(envIndex);
        }
      }
    }
  }
}

void DSWP::collectLiveOutEnvInfo (LoopDependenceInfo *LDI) {
  for (auto envIndex : LDI->environment->getEnvIndicesOfLiveOutVars()) {
    auto producer = LDI->environment->producerAt(envIndex);

    bool isSharedInst = false;
    for (auto scc : LDI->sccdagAttrs.clonableSCCs) {
      if (!scc->isInternal(producer)) continue;
      isSharedInst = true;
      envBuilder->getUser(0)->addLiveOutIndex(envIndex);
      break;
    }

    if (!isSharedInst) {
      for (auto i = 0; i < this->tasks.size(); ++i) {
        auto task = (DSWPTask *)this->tasks[i];
        bool isInternal = false;
        for (auto scc : task->stageSCCs) isInternal |= scc->isInternal(producer);
        if (isInternal) {
          envBuilder->getUser(i)->addLiveOutIndex(envIndex);
          break;
        }
      }
    }
  }
}
