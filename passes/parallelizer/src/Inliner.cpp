#include "DSWP.hpp"

using namespace llvm;

void Parallelizer::inlineQueueCalls (DSWPLoopDependenceInfo *LDI, std::unique_ptr<StageInfo> &stageInfo) {
  std::queue<CallInst *> callsToInline;
  for (auto &queueInstrPair : stageInfo->queueInstrMap) {
    auto &queueInstr = queueInstrPair.second;
    callsToInline.push(cast<CallInst>(queueInstr->queueCall));
  }

  while (!callsToInline.empty()) {

    /*
     * Empty the queue, inlining each site
     */
    std::set<Function *> funcToInline;
    while (!callsToInline.empty()) {
      auto callToInline = callsToInline.front();
      callsToInline.pop();

      auto F = callToInline->getCalledFunction();
      for (auto &B : *F) {
        for (auto &I : B) {
          if (auto call = dyn_cast<CallInst>(&I)) {
            auto func = call->getCalledFunction();
            if (func == nullptr || func->empty()) continue;
            funcToInline.insert(func);
          }
        }
      }

      InlineFunctionInfo IFI;
      InlineFunction(callToInline, IFI);
    }

    /*
     * Collect next level of queue push/pop calls to inline
     */
    for (auto &B : *stageInfo->sccStage) {
      for (auto &I : B) {
        if (auto call = dyn_cast<CallInst>(&I)) {
          if (funcToInline.find(call->getCalledFunction()) != funcToInline.end()) {
            callsToInline.push(call);
          }
        }
      }
    }
  }
}

