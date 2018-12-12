#include "DSWP.hpp"

using namespace llvm;

void DSWP::collectLiveInEnvInfo (DSWPLoopDependenceInfo *LDI) {
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

void DSWP::collectLiveOutEnvInfo (DSWPLoopDependenceInfo *LDI) {
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
