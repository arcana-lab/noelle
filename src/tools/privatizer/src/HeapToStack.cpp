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
#include "Privatizer.hpp"
#include "Utils.hpp"

namespace arcana::noelle {

bool Privatizer::applyH2S(Noelle &noelle) {
  bool modified = false;
  for (auto &[f, liveMemSum] : collectH2S(noelle)) {
    modified |= transformH2S(noelle, liveMemSum);
  }
  clearFunctionSummaries();
  return modified;
}

/*
 * Compute @malloc() or @calloc() insts that could be transformed to allocaInst.
 * Compute @free() insts that could be removed becase if @malloc() is
 * transformed to allocaInst, the corresponding @free() must be removed.
 */
LiveMemorySummary Privatizer::getLiveMemorySummary(Noelle &noelle,
                                                   Function *f) {

  auto cfgAnalysis = noelle.getCFGAnalysis();
  auto funcSum = getFunctionSummary(f);

  auto heapAllocInsts = funcSum->mallocInsts;
  heapAllocInsts.insert(funcSum->callocInsts.begin(),
                        funcSum->callocInsts.end());

  std::unordered_set<CallBase *> allocable;

  /*
   * 1. Only fixed size @malloc(), such as %1 = tail call i8* @malloc(i64 8),
   *    can be transformed to allocaInst.
   * 2. @malloc() insts in loops can't be transformed to allocaInst becuase it
   *    will be executed multiple times.
   * 3. @malloc() insts that may escape can't be transformed to allocaInst.
   * 4. Dest of @memcpy() shouldn't be transformed to allocaInst. Otherwise,
   *    -instCombine pass will incorrectly remove the memcpy.
   */
  for (auto heapAllocInst : heapAllocInsts) {
    if (!isFixedSizedHeapAllocation(heapAllocInst)) {
      continue;
    }
    if (cfgAnalysis.isIncludedInACycle(*heapAllocInst)) {
      continue;
    }
    if (mpa.mayEscape(heapAllocInst)
        || funcSum->isDestOfMemcpy(heapAllocInst)) {
      continue;
    }

    allocable.insert(heapAllocInst);
  }

  auto mayFreeNonAllocable =
      [&](std::unordered_set<Value *> mayBeFreed) -> bool {
    for (auto memobj : mayBeFreed) {
      if (!memobj) {
        return true;
      } else if (!isa<CallBase>(memobj)) {
        return true;
      } else {
        auto heapAlloc = dyn_cast<CallBase>(memobj);
        if (allocable.find(heapAlloc) == allocable.end()) {
          return true;
        }
      }
    }
    return false;
  };

  /*
   * Assume we have:
   *   %1 = tail call i8* @malloc(i64 8)
   *   %2 = tail call i8* @malloc(i64 8)
   *   %3 = tail call i8* @malloc(i64 8)
   *   call @free(%4);
   *   call @free(%5);
   * where %1, %2 are allocable; while %3 is not.
   * %4 may free memory object allocated by %1, %2.
   * %5 may free memory object allocated by %2, %3.
   *
   * It turns out we can't optimize anything.
   * Since %3 is not an allocable, we can't remove @free(%5).
   * This means %2 should not be transformed to allocaInst as well.
   * Since %2 can't be transformed to allocaInst, we can't remove @free(%4).
   * This means %1 also shouldn't be transformed to allocaInst.
   *
   * Therefore, if the memory object allocated by an allocable can be
   * freed by a @free() inst that may also free memory objects allocated
   * by a non-allocable, the allocable is not an allocable anymore.
   */
  bool fixedPoint = false;
  while (!fixedPoint) {
    fixedPoint = true;
    for (auto freeInst : funcSum->freeInsts) {
      auto mayBeFreed = mpa.getPointees(freeInst->getArgOperand(0), f);
      if (mayFreeNonAllocable(mayBeFreed)) {
        for (auto allocation : mayBeFreed) {
          if (allocation && isa<CallBase>(allocation)) {
            auto heapAllocInst = dyn_cast<CallBase>(allocation);
            if (allocable.find(heapAllocInst) != allocable.end()) {
              allocable.erase(heapAllocInst);
              fixedPoint = false;
            }
          }
        }
      }
    }
  }

  /*
   * If a @free() inst can only free memory objects allocated by the remaining
   * allocable, we can safely remove the @free() inst.
   */
  std::unordered_set<CallBase *> removable;
  for (auto freeInst : funcSum->freeInsts) {
    auto mayBeFreed = mpa.getPointees(freeInst->getArgOperand(0), f);
    if (!mayFreeNonAllocable(mayBeFreed)) {
      removable.insert(freeInst);
    }
  }

  LiveMemorySummary memSum = LiveMemorySummary();
  memSum.removable = removable;
  memSum.allocable = allocable;

  return memSum;
}

std::unordered_map<Function *, LiveMemorySummary> Privatizer::collectH2S(
    Noelle &noelle) {

  auto hotFuncs = hotFunctions(noelle);

  std::unordered_set<Function *> heapAllocUsers;
  for (auto &F : *M) {
    if (F.getName() != "malloc" && F.getName() != "calloc") {
      continue;
    }
    for (auto user : F.users()) {
      if (auto callInst = dyn_cast<CallBase>(user)) {
        auto userFunc = callInst->getFunction();
        if (hotFuncs.find(userFunc) != hotFuncs.end()) {
          heapAllocUsers.insert(userFunc);
        }
      }
    }
  }

  std::unordered_map<Function *, LiveMemorySummary> result;

  for (auto f : heapAllocUsers) {
    auto fname = f->getName();
    auto suffix = "in function " + fname + "\n";
    auto memSum = getLiveMemorySummary(noelle, f);

    if (memSum.allocable.empty()) {
      errs() << prefix << "@malloc or @calloc not allocable " << suffix;
      continue;
    }

    result[f] = memSum;
  }

  return result;
}

bool Privatizer::transformH2S(Noelle &noelle, LiveMemorySummary liveMemSum) {
  auto modified = false;

  auto heapAllocInsts = Utils::sort(liveMemSum.allocable);
  for (auto heapAllocInst : heapAllocInsts) {
    auto allocationSize = getAllocationSize(heapAllocInst);
    auto currentF = heapAllocInst->getParent()->getParent();
    auto funcSum = getFunctionSummary(currentF);
    auto suffix = "in function " + currentF->getName() + "\n";

    /*
     * Check if the stack of current function can hold the memory object of
     * the @malloc(). If @malloc() allocates an memobj too large for the stack,
     * it should not be transformed to allocaInst.
     */
    if (!funcSum->stackCanHoldNewAlloca(allocationSize)) {
      errs()
          << prefix
          << "Stack memory usage exceeds the limit, can't transfrom to allocaInst: "
          << *heapAllocInst << " " << suffix;
      continue;
    }

    modified = true;
    auto entryBlock = &currentF->getEntryBlock();
    auto firstInst = entryBlock->getFirstNonPHI();
    IRBuilder<> entryBuilder(firstInst);

    auto &context = noelle.getProgramContext();
    auto oneByteType = Type::getInt8Ty(context);
    auto arraySize =
        ConstantInt::get(Type::getInt64Ty(context), allocationSize);

    auto calleeFunc = heapAllocInst->getCalledFunction();

    if (calleeFunc && calleeFunc->getName() == "malloc") {
      AllocaInst *allocaInst =
          entryBuilder.CreateAlloca(oneByteType, arraySize, "malloc2alloca");

      errs() << prefix << "Replace @malloc: " << *heapAllocInst << "\n";
      errs() << emptyPrefix << "with allocaInst: " << *allocaInst << "\n";
      errs() << emptyPrefix << suffix;

      heapAllocInst->replaceAllUsesWith(allocaInst);
      heapAllocInst->eraseFromParent();

    } else if (calleeFunc && calleeFunc->getName() == "calloc") {
      ConstantInt *zeroVal = ConstantInt::get(Type::getInt8Ty(context), 0);

      AllocaInst *allocaInst =
          entryBuilder.CreateAlloca(oneByteType, arraySize, "calloc2alloca");

      CallInst *memSetInst = entryBuilder.CreateMemSet(allocaInst,
                                                       zeroVal,
                                                       allocationSize,
                                                       allocaInst->getAlign());

      errs() << prefix << "Replace @calloc: " << *heapAllocInst << "\n";
      errs() << emptyPrefix << "with allocaInst: " << *allocaInst << "\n";
      errs() << emptyPrefix << "and memset Inst: " << *memSetInst << "\n";
      errs() << emptyPrefix << suffix;

      heapAllocInst->replaceAllUsesWith(allocaInst);
      heapAllocInst->eraseFromParent();
    }
  }

  /*
   * Remove dead instructions.
   */
  for (auto freeInst : liveMemSum.removable) {
    freeInst->eraseFromParent();
  }

  return modified;
}

} // namespace arcana::noelle
