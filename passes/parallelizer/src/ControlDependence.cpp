#include "DSWP.hpp"

using namespace llvm;

void Parallelizer::collectTransitiveCondBrs (DSWPLoopDependenceInfo *LDI,
  std::set<TerminatorInst *> &bottomLevelBrs,
  std::set<TerminatorInst *> &descendantCondBrs
  ) {
  std::queue<DGNode<Value> *> queuedBrs;
  std::set<TerminatorInst *> visitedBrs;
  for (auto br : bottomLevelBrs) {
    queuedBrs.push(LDI->loopInternalDG->fetchNode(cast<Value>(br)));
    visitedBrs.insert(br);
  }

  while (!queuedBrs.empty()) {
    auto brNode = queuedBrs.front();
    auto term = cast<TerminatorInst>(brNode->getT());
    queuedBrs.pop();
    if (term->getNumSuccessors() > 1) {
      descendantCondBrs.insert(term);
    }

    for (auto edge : brNode->getIncomingEdges()) {
      if (auto termI = dyn_cast<TerminatorInst>(edge->getOutgoingT())) {
        if (visitedBrs.find(termI) == visitedBrs.end()) {
          queuedBrs.push(edge->getOutgoingNode());
          visitedBrs.insert(termI);
        }
      }
    }
  }
}
