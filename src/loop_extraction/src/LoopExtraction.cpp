/*
 * Copyright 2019 - 2020  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopExtraction.hpp"

#include "PDGPrinter.hpp"
#include "SubCFGs.hpp"

using namespace llvm;

int counter = 0;

LoopExtraction::LoopExtraction(Noelle &noelle)
  : noelle{noelle}
  {
  return ;
}

bool LoopExtraction::extractValuesFromLoop (
  LoopDependenceInfo const &LDI
  ){
  
  auto modified = false;

  modified |= hoistStoreOfLastValueLiveOut(LDI);

  counter++;

  return modified;
}

bool LoopExtraction::hoistStoreOfLastValueLiveOut (
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

  PostDominatorTree PDT(*header->getParent());
  auto sccdag = LDI.sccdagAttrs.getSCCDAG();
  std::unordered_set<StoreInst *> independentStoresExecutedEveryIteration;

  /*
   * TODO: Prevent stores that alias with these from being hoisted
   */
  std::unordered_set<StoreInst *> independentStoresExecutedSomeIterations;

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

    // if (stores.size() > 0) {
    //   std::set<BasicBlock *> bbs(loopSummary->orderedBBs.begin(), loopSummary->orderedBBs.end());
    //   DGPrinter::writeGraph<SubCFGs>("technique-original-loop-" + std::to_string(counter) + ".dot", new SubCFGs(bbs));
    //   DGPrinter::writeGraph<SCCDAG>("technique-sccdag-loop-" + std::to_string(counter) + ".dot", LDI.sccdagAttrs.getSCCDAG());
    // }

    /*
     * Determine if the store is executed every iteration
     * This is true if the store basic block post dominates
     * all loop-internal successor blocks of the loop entry block
     */
    for (auto store : stores) {
      auto storeBlock = store->getParent();
      bool postDominatesAll = true;
      // store->print(errs() << "ENABLERS:  Might hoist"); errs() << "\n";

      for (auto B : loopEntrySuccessors) {
        postDominatesAll &= PDT.dominates(storeBlock, B);
      }
      if (!postDominatesAll) continue;

      independentStoresExecutedEveryIteration.insert(store);
    }

  }

  modified = independentStoresExecutedEveryIteration.size() > 0;

  IRBuilder<> preHeaderBuilder(preHeader->getTerminator());
  IRBuilder<> headerBuilder(header->getFirstNonPHIOrDbgOrLifetime());
  for (auto store : independentStoresExecutedEveryIteration) {
    auto storedValue = store->getValueOperand();
    auto pointerOperand = store->getPointerOperand();
    // store->print(errs() << "ENABLERS:  Hoisting"); errs() << "\n";
    store->eraseFromParent();

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
