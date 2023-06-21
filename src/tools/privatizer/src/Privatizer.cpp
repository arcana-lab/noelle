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
#include <numeric>

namespace llvm::noelle {

bool PrivatizerManager::applyHeapToStack(
    Noelle &noelle,
    MayPointToAnalysis &mayPointToAnalysis) {
  auto fm = noelle.getFunctionsManager();
  auto funcSum = mayPointToAnalysis.getFunctionSummary();
  if (funcSum->mallocInsts.empty() && funcSum->callocInsts.empty()) {
    errs() << "PrivatizerManager: @malloc or @calloc not invoked in function "
           << funcSum->F->getName() << "\n";
    return false;
  }

  bool modified = false;
  auto ptSum = mayPointToAnalysis.getPointToSummary();
  auto memSum = mayPointToAnalysis.getLiveMemorySummary();
  auto dl = funcSum->M->getDataLayout();
  auto stackMemoryUsage = std::accumulate(
      funcSum->allocaInsts.begin(),
      funcSum->allocaInsts.end(),
      0,
      [&](uint64_t previous, AllocaInst *item) -> uint64_t {
        return previous + (item->getAllocationSizeInBits(dl).getValue() / 8);
      });

  auto getAllocationSize = [memSum](CallBase *heapAllocInst) -> uint64_t {
    assert(memSum->allocable.count(heapAllocInst) > 0
           && "Not a fixed-sized memory allocation instruction");
    if (getCalledFuncName(heapAllocInst) == "malloc") {
      return dyn_cast<ConstantInt>(heapAllocInst->getOperand(0))
          ->getZExtValue();
    } else if (getCalledFuncName(heapAllocInst) == "calloc") {
      auto elementCount =
          dyn_cast<ConstantInt>(heapAllocInst->getOperand(0))->getZExtValue();
      auto elementSizeInBytes =
          dyn_cast<ConstantInt>(heapAllocInst->getOperand(1))->getZExtValue();
      return elementCount * elementSizeInBytes;
    } else {
      assert(false && "Unsupported memory allocation function");
    }
  };

  for (auto heapAllocInst : memSum->allocable) {
    auto allocationSize = getAllocationSize(heapAllocInst);
    if (stackMemoryUsage + allocationSize >= STACK_SIZE_THRESHOLD) {
      errs()
          << "PrivatizerManager: Stack memory usage exceeds the limit, can't transfrom to allocaInst: "
          << *heapAllocInst << "\n";
      continue;
    } else {
      stackMemoryUsage += allocationSize;
    }

    auto entryBlock = &funcSum->F->getEntryBlock();
    auto firstInst = entryBlock->getFirstNonPHI();
    IRBuilder<> entryBuilder(firstInst);
    IRBuilder<> allocBuilder(heapAllocInst);

    LLVMContext &context = heapAllocInst->getContext();
    Type *oneByteType = Type::getInt8Ty(context);

    if (getCalledFuncName(heapAllocInst) == "malloc") {

      Value *arraySize = heapAllocInst->getOperand(0);
      AllocaInst *allocaInst =
          entryBuilder.CreateAlloca(oneByteType, arraySize, "");

      errs()
          << "PrivatizerManager: Replace @malloc: " << *heapAllocInst << "\n";
      errs() << "                   With allocaInst: " << *allocaInst << "\n";

      heapAllocInst->replaceAllUsesWith(allocaInst);
      heapAllocInst->eraseFromParent();

    } else if (getCalledFuncName(heapAllocInst) == "calloc") {
      Value *elementCount = heapAllocInst->getOperand(0);
      Value *elementSizeInBytes = heapAllocInst->getOperand(1);
      Value *arraySize =
          entryBuilder.CreateMul(elementCount, elementSizeInBytes);
      ConstantInt *zeroVal = ConstantInt::get(Type::getInt8Ty(context), 0);

      AllocaInst *allocaInst =
          entryBuilder.CreateAlloca(oneByteType, arraySize, "");
      // using @memset after @alloca to initialize the memory to zero
      CallInst *memSetInst =
          allocBuilder.CreateMemSet(allocaInst, zeroVal, arraySize, 1);

      errs()
          << "PrivatizerManager: Replace @calloc: " << *heapAllocInst << "\n";
      errs() << "                   With allocaInst: " << *allocaInst << "\n";

      heapAllocInst->replaceAllUsesWith(allocaInst);
      heapAllocInst->eraseFromParent();
    }
    modified = true;
  }
  for (auto freeInst : memSum->removable) {
    freeInst->eraseFromParent();
    modified = true;
  }

  return modified;
}

bool PrivatizerManager::applyGlobalToStack(
    Noelle &noelle,
    MayPointToAnalysis &mayPointToAnalysis) {

  auto funcSum = mayPointToAnalysis.getFunctionSummary();
  auto currentF = funcSum->F;
  auto M = funcSum->M;

  auto moveGlobalToStack = [&](GlobalVariable *globalVar,
                               MayPointToAnalysis &mayPointToAnalysis) {
    /*
     * If the global variable can't be safely cloned to stack,
     * or there is no need to clone it,
     * do nothing.
     */
    if (!this->canBeClonedToStack(globalVar, noelle, mayPointToAnalysis)) {
      return false;
    }

    /*
     * Replace all uses of the global variable in the entry function with an
     * allocaInst. The allocaInst is placed at the beginning of
     * the entry block and is initialized with the global variable's
     * initializer.
     */
    unordered_set<Instruction *> instsToReplace;
    for (auto user : globalVar->users()) {
      if (auto inst = dyn_cast<Instruction>(user)) {
        if (inst->getParent()->getParent() == currentF) {
          instsToReplace.insert(inst);
        }
      }
    }

    auto &entryBlock = currentF->getEntryBlock();
    IRBuilder<> entryBuilder(entryBlock.getFirstNonPHI());
    Type *globalVarType = globalVar->getValueType();
    AllocaInst *allocaInst =
        entryBuilder.CreateAlloca(globalVarType, nullptr, "");

    if (globalVar->hasInitializer()) {
      auto initializer = globalVar->getInitializer();
      if (isa<ConstantAggregateZero>(initializer)
          && globalVarType->isArrayTy()) {
        auto typesManager = noelle.getTypesManager();
        auto sizeInByte = typesManager->getSizeOfType(globalVarType);
        auto zeroVal = ConstantInt::get(Type::getInt8Ty(M->getContext()), 0);
        entryBuilder.CreateMemSet(allocaInst, zeroVal, sizeInByte, 1);
      } else {
        entryBuilder.CreateStore(initializer, allocaInst);
      }
    }

    for (auto inst : instsToReplace) {
      inst->replaceUsesOfWith(globalVar, allocaInst);
    }

    errs() << "PrivatizerManager: Replace global variable "
           << globalVar->getName() << "\n"
           << "                   with allocaInst: " << *allocaInst << "\n";
    return true;
  };

  auto modified = false;
  for (auto &G : M->globals()) {
    modified |= moveGlobalToStack(&G, mayPointToAnalysis);
  }

  return modified;
}

bool PrivatizerManager::canBeClonedToStack(
    GlobalVariable *globalVar,
    Noelle &noelle,
    MayPointToAnalysis &mayPointToAnalysis) {
  /*
   * We only clone global variables to stack for main function.
   */
  auto funcSum = mayPointToAnalysis.getFunctionSummary();
  auto currentF = funcSum->F;
  auto mainF = noelle.getFunctionsManager()->getEntryFunction();
  if (currentF != mainF) {
    errs() << "PrivatizerManager: Function " << currentF->getName()
           << " is not main function,"
           << " global variable " << globalVar->getName()
           << " may be read or modified outside this function,"
           << " do not clone it to function " << currentF->getName() << ".\n";
    return false;
  }

  /*
   * Global variable shall not be accessed in another function.
   * If accessed, do not clone it.
   *
   * If some function @F() is called in current function,
   * we will conservatively assume the global
   * memory object is modified and choose not to clone it.
   */
  auto ptSum = mayPointToAnalysis.getPointToSummary();
  auto globalMemObj = ptSum->getMemoryObject(globalVar);
  if (funcSum->unknownFuntctionCalls.size() > 0) {
    errs() << "PrivatizerManager: Unknown function calls in function "
           << currentF->getName() << " may modify global variable "
           << globalVar->getName() << ", do not clone it to function "
           << currentF->getName() << ".\n";
    return false;
  }

  /*
   * If global variable is never written in a loop,
   * which means the global variable is read-only or never used in a loop,
   * cloning contributes nothing to parallelization, do not clone it.
   */
  unordered_set<Instruction *> instsWriteGlobalVar;
  for (auto storeInst : funcSum->storeInsts) {
    auto IN = ptSum->instIN.at(storeInst);
    auto ptr = storeInst->getPointerOperand();
    auto mayBeStored = ptSum->getPointees(IN, ptr);
    auto globalMemObj = ptSum->getMemoryObject(globalVar);
    if (mayBeStored.count(globalMemObj) > 0) {
      instsWriteGlobalVar.insert(storeInst);
    }
  }

  if (instsWriteGlobalVar.empty()) {
    errs() << "PrivatizerManager: Global variable " << globalVar->getName()
           << " is never modified in function " << currentF->getName()
           << ", no need clone it to function " << currentF->getName() << ".\n";
    return false;
  }

  errs() << "PrivatizerManager: Global variable " << globalVar->getName()
         << " can be cloned to function " << currentF->getName() << ".\n";
  return true;
}

} // namespace llvm::noelle
