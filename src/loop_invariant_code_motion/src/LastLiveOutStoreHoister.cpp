/*
 * Copyright 2019 - 2020  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopInvariantCodeMotion.hpp"

using namespace llvm;

bool LoopInvariantCodeMotion::hoistStoreOfLastValueLiveOut (
  LoopDependenceInfo const &LDI
){

  bool modified = false;

  auto loopSummary = LDI.getLoopStructure();
  auto preHeader = loopSummary->getPreHeader();
  auto header = loopSummary->getHeader();
  std::unordered_set<BasicBlock *> loopEntrySuccessors{};
  for (auto bIter = succ_begin(header); bIter != succ_end(header); ++bIter) {
    auto B = *bIter;
    if (!loopSummary->isIncluded(B)) continue;
    loopEntrySuccessors.insert(B);
  }

  DominatorTree DT(*header->getParent());
  PostDominatorTree PDT(*header->getParent());
  DominatorSummary DS(DT, PDT);

  auto sccdag = LDI.sccdagAttrs.getSCCDAG();
  std::unordered_set<StoreInst *> independentStoresExecutedEveryIteration;

  for (auto sccNode : sccdag->getNodes()) {
    auto scc = sccNode->getT();
    auto sccInfo = LDI.sccdagAttrs.getSCCAttrs(scc);
    if (!sccInfo->canExecuteIndependently()) continue;

    /*
     * Determine if an independent SCC contains 1+ store instructions
     * without loads
     */
    bool onlyStores = true;
    std::unordered_set<StoreInst *> stores;
    for (auto nodePair : scc->internalNodePairs()) {
      auto value = nodePair.first;
      if (!isa<StoreInst>(value)) {
        onlyStores = false;
        break;
      }
      stores.insert(cast<StoreInst>(value));
    }

    /*
     * Alias-ing stores require further analysis to hoist
     * 
     * For now, as long as the pointer operand is the same
     * and one store post dominates the rest, hoist that one
     */
    StoreInst *singleLastStore = nullptr;
    Value *singlePointerOperand = nullptr;
    for (auto store : stores) {
      auto pointerOp = store->getPointerOperand();
      if (!singlePointerOperand) {
        singlePointerOperand = pointerOp;
        singleLastStore = store;
        continue;
      }

      if (pointerOp == singlePointerOperand) {
        if (DS.PDT.dominates(singleLastStore, store)) continue;
        if (DS.PDT.dominates(store, singleLastStore)) {
          singleLastStore = store;
          continue;
        }
      }

      singleLastStore = nullptr;
      break;
    }
    if (!singleLastStore) continue;

    /*
     * Determine if the store is executed every iteration
     * This is true if the store basic block post dominates
     * all loop-internal successor blocks of the loop entry block
     */
    singleLastStore->print(errs() << "ENABLERS:  Might hoist"); errs() << "\n";
    auto storeBlock = singleLastStore->getParent();
    bool postDominatesAll = true;

    for (auto B : loopEntrySuccessors) {
      postDominatesAll &= DS.PDT.dominates(storeBlock, B);
    }
    if (!postDominatesAll) continue;

    independentStoresExecutedEveryIteration.insert(singleLastStore);
  }

  modified = independentStoresExecutedEveryIteration.size() > 0;

  IRBuilder<> preHeaderBuilder(preHeader->getTerminator());
  IRBuilder<> headerBuilder(header->getFirstNonPHIOrDbgOrLifetime());
  for (auto store : independentStoresExecutedEveryIteration) {
    auto storedValue = store->getValueOperand();
    auto pointerOperand = store->getPointerOperand();

    /*
     * Erase this store and any other in its SCC
     */
    auto scc = sccdag->sccOfValue(store);
    for (auto nodePair : scc->internalNodePairs()) {
      auto value = nodePair.first;
      assert(isa<StoreInst>(value));
      cast<StoreInst>(value)->eraseFromParent();
    }

    auto initialValue = preHeaderBuilder.CreateLoad(pointerOperand);

    /*
     * Create PHI to track last value in loop entry
     */
    auto latches = loopSummary->getLatches();
    auto numPredecessors = latches.size() + 1;
    PHINode *phi = headerBuilder.CreatePHI(initialValue->getType(), numPredecessors, "lastValueToStore");
    phi->addIncoming(initialValue, preHeader);
    for (auto latch : latches) {
      phi->addIncoming(storedValue, latch);
    }

    /*
     * Insert store of the last value PHI at every exit
     */
    for (auto exitBlock : loopSummary->getLoopExitBasicBlocks()) {
      IRBuilder<> exitBuilder(exitBlock->getTerminator());
      exitBuilder.CreateStore(phi, pointerOperand);
    }
  }

  return modified;
}