#include "DSWP.hpp"

using namespace llvm;

void DSWP::collectLiveInEnvInfo (DSWPLoopDependenceInfo *LDI) {
  for (auto envIndex : LDI->environment->getPreEnvIndices()) {
    auto producer = LDI->environment->producerAt(envIndex);

    for (auto consumer : LDI->environment->consumersOf(producer)) {
      bool isSharedInst = false;
      for (auto scc : LDI->sccdagAttrs.clonableSCCs) {
        if (!scc->isInternal(consumer)) continue;
        isSharedInst = true;
        for (auto i = 0; i < workers.size(); ++i) {
          envBuilder->getUser(i)->addPreEnvIndex(envIndex);
        }
        break;
      }

      if (!isSharedInst) {
        for (auto i = 0; i < this->workers.size(); ++i) {
          auto worker = (DSWPTechniqueWorker *)this->workers[i];
          bool isInternal = false;
          for (auto scc : worker->stageSCCs) isInternal |= scc->isInternal(consumer);
          if (isInternal) envBuilder->getUser(i)->addPreEnvIndex(envIndex);
        }
      }
    }
  }
}

void DSWP::collectLiveOutEnvInfo (DSWPLoopDependenceInfo *LDI) {
  for (auto envIndex : LDI->environment->getPostEnvIndices()) {
    auto producer = LDI->environment->producerAt(envIndex);

    bool isSharedInst = false;
    for (auto scc : LDI->sccdagAttrs.clonableSCCs) {
      if (!scc->isInternal(producer)) continue;
      isSharedInst = true;
      envBuilder->getUser(0)->addPostEnvIndex(envIndex);
      break;
    }

    if (!isSharedInst) {
      for (auto i = 0; i < this->workers.size(); ++i) {
        auto worker = (DSWPTechniqueWorker *)this->workers[i];
        bool isInternal = false;
        for (auto scc : worker->stageSCCs) isInternal |= scc->isInternal(producer);
        if (isInternal) {
          envBuilder->getUser(i)->addPostEnvIndex(envIndex);
          break;
        }
      }
    }
  }
}
