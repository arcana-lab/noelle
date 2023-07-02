/*
 * Copyright 2023 Xiao Chen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "PrivatizerManager.hpp"
#include "llvm/IR/Constants.h"
#include <numeric>

namespace llvm::noelle {

/*
 * Compute @malloc() or @calloc() insts that could be transformed to allocaInst.
 * Compute @free() insts that could be removed becase if @malloc() is
 * transformed to allocaInst, the corresponding @free() must be removed.
 */
LiveMemorySummary PrivatizerManager::getLiveMemorySummary(
    Noelle &noelle,
    PointToSummary *ptSum,
    FunctionSummary *funcSum) {

  auto loopStructures = noelle.getLoopStructures();
  auto loopForest = noelle.organizeLoopsInTheirNestingForest(*loopStructures);
  auto funcPtGraph = funcSum->functionPointToGraph;

  /*
   * Only fixed size @malloc(), such as %1 = tail call i8* @malloc(i64 8), can
   * be transformed to allocaInst. Otherwise, it may cause stack overflow.
   */
  MemoryObjects allocable = [&]() {
    MemoryObjects fixedSized;
    auto heapAllocInsts = funcSum->mallocInsts;
    heapAllocInsts.insert(funcSum->callocInsts.begin(),
                          funcSum->callocInsts.end());

    for (auto heapAllocInst : heapAllocInsts) {
      if (isFixedSizedHeapAllocation(heapAllocInst)) {
        fixedSized.insert(ptSum->getMemoryObject(heapAllocInst));
      }
    }
    return fixedSized;
  }();

  MemoryObjects allocatedInLoop = [&]() {
    MemoryObjects result;
    for (auto heapMemObj : allocable) {
      auto heapAllocInst = dyn_cast<Instruction>(heapMemObj->getSource());
      auto loop = loopForest->getInnermostLoopThatContains(heapAllocInst);
      if (loop) {
        result.insert(heapMemObj);
      }
    }
    return result;
  }();

  allocable = minus(allocable, allocatedInLoop);
  allocable = minus(allocable, funcSum->mustHeap);
  allocable = minus(allocable, funcSum->reachableFromReturnValue);
  // allocable = minus(allocable, ptSum->ambiguous);

  auto [removable, notAllocable] =
      [&]() -> pair<unordered_set<CallBase *>, MemoryObjects> {
    /*
     * We have:
     * %1 = call i8* @malloc(i64 8), %1 -> M1 (M1 is the memory object allocated
     * by @malloc) and %2 = call i8* @malloc(i64 8), %2 -> M2 (M2 is the memory
     * object allocated by @malloc)
     *
     * Assume M1 is can be transformed to allocaInst, M2 escapes and therefore
     * cannot be transformed to allocaInst, and we have a free instruction
     * %7 = call i8* @free(i8* %6), where %6 may point to M1 or M2.
     *
     * In this case:
     * M2 is not allocable because it escapes, it remains a @malloc().
     * To maintain the original semantics, we cannot remove %7, because it
     * may free M2. Since %7 may also free M1, we cannot transform %1 to
     * allocaInst since that may cause segfault.
     *
     * Therefore, for any free inst: %7 = call i8* @free(i8* %6)
     * if any memory object pointed by %6 is not allocable,
     * then all memory objects pointed by %6 are not allocable.
     */
    MemoryObjects notAllocable;
    for (auto freeInst : funcSum->freeInsts) {
      auto ptr = ptSum->getVariable(freeInst->getArgOperand(0));
      auto mayBeFreed = funcPtGraph->getPointees(ptr);
      for (auto memObj : mayBeFreed) {
        if (allocable.count(memObj) == 0) {
          notAllocable = unite(notAllocable, mayBeFreed);
        }
      }
    }

    unordered_set<CallBase *> removable = funcSum->freeInsts;
    for (auto freeInst : funcSum->freeInsts) {
      auto ptr = ptSum->getVariable(freeInst->getArgOperand(0));
      auto mayBeFreed = funcPtGraph->getPointees(ptr);

      for (auto memObj : mayBeFreed) {
        if (notAllocable.count(memObj) > 0) {
          removable.erase(freeInst);
        }
      }
    }

    return make_pair(removable, notAllocable);
  }();

  allocable = minus(allocable, notAllocable);

  LiveMemorySummary memSum = LiveMemorySummary();
  memSum.removable = removable;
  memSum.allocable = [&](void) {
    unordered_set<CallBase *> result;
    for (auto memObj : allocable) {
      auto heapAllocInst = dyn_cast<CallBase>(memObj->getSource());
      assert(heapAllocInst);
      result.insert(heapAllocInst);
    }
    return result;
  }();

  return memSum;
}

unordered_map<Function *, LiveMemorySummary> PrivatizerManager::
    collectHeapToStack(Noelle &noelle, PointToSummary *ptSum) {

  unordered_map<Function *, LiveMemorySummary> result;

  for (auto &[f, funcSum] : ptSum->functionSummaries) {
    auto fname = funcSum->currentF->getName();
    auto suffix = " in function " + fname + "\n";

    if (funcSum->mallocInsts.empty() && funcSum->callocInsts.empty()) {
      errs() << prefix << "@malloc or @calloc not invoked" << suffix;
      continue;
    }

    auto memSum = getLiveMemorySummary(noelle, ptSum, funcSum);
    if (memSum.allocable.empty()) {
      errs() << prefix << "@malloc or @calloc not allocable" << suffix;
      continue;
    }

    result[f] = memSum;
  }

  return result;
}

bool PrivatizerManager::applyHeapToStack(Noelle &noelle,
                                         LiveMemorySummary liveMemSum) {
  bool modified = false;

  for (auto heapAllocInst : liveMemSum.allocable) {
    auto allocationSize = getAllocationSize(heapAllocInst);
    auto currentF = heapAllocInst->getParent()->getParent();
    auto suffix = " in function " + currentF->getName() + "\n";

    if (!stackHasEnoughSpaceForNewAllocaInst(allocationSize, currentF)) {
      errs()
          << prefix
          << "Stack memory usage exceeds the limit, can't transfrom to allocaInst: "
          << *heapAllocInst << suffix;
      continue;
    }

    errs()
        << prefix << "Transform @malloc() or @calloc() to allocaInst" << suffix;

    modified = true;
    auto entryBlock = &currentF->getEntryBlock();
    auto firstInst = entryBlock->getFirstNonPHI();
    IRBuilder<> entryBuilder(firstInst);
    IRBuilder<> allocBuilder(heapAllocInst);

    LLVMContext &context = noelle.getProgramContext();
    Type *oneByteType = Type::getInt8Ty(context);
    ConstantInt *arraySize =
        ConstantInt::get(Type::getInt64Ty(context), allocationSize);

    if (getCalledFuncName(heapAllocInst) == "malloc") {
      AllocaInst *allocaInst =
          entryBuilder.CreateAlloca(oneByteType, arraySize, "");

      errs() << prefix << "Replace @malloc: " << *heapAllocInst << "\n";
      errs() << emptyPrefix << "with allocaInst: " << *allocaInst << "\n";

      heapAllocInst->replaceAllUsesWith(allocaInst);
      heapAllocInst->eraseFromParent();

    } else if (getCalledFuncName(heapAllocInst) == "calloc") {
      ConstantInt *zeroVal = ConstantInt::get(Type::getInt8Ty(context), 0);

      AllocaInst *allocaInst =
          entryBuilder.CreateAlloca(oneByteType, arraySize, "");

      CallInst *memSetInst =
          allocBuilder.CreateMemSet(allocaInst, zeroVal, arraySize, 1);

      errs() << prefix << "Replace @malloc: " << *heapAllocInst << "\n";
      errs() << emptyPrefix << "with allocaInst: " << *allocaInst << "\n";
      errs() << emptyPrefix << "and memsetInst: " << *memSetInst << "\n";

      heapAllocInst->replaceAllUsesWith(allocaInst);
      heapAllocInst->eraseFromParent();
    }
  }
  for (auto freeInst : liveMemSum.removable) {
    freeInst->eraseFromParent();
  }
  return modified;
}

} // namespace llvm::noelle
