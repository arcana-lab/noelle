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

unordered_set<Function *> PrivatizerManager::getPrivatizableFunctions(
    Noelle &noelle,
    PointToSummary *ptSum,
    GlobalVariable *globalVar) {

  auto globalVarName = globalVar->getName();

  auto fm = noelle.getFunctionsManager();
  auto pcf = fm->getProgramCallGraph();

  auto insertMyCallees = [&](Function *caller,
                             queue<Function *> &funcToTraverse) {
    auto funcNode = pcf->getFunctionNode(caller);
    for (auto callEdge : funcNode->getOutgoingEdges()) {
      for (auto subEdge : callEdge->getSubEdges()) {
        auto calleeFunc = subEdge->getCallee()->getFunction();
        if (!calleeFunc || calleeFunc->empty()) {
          continue;
        }
        funcToTraverse.push(calleeFunc);
      }
    }
  };

  auto mayInvoke = [&](Function *caller, Function *callee) {
    unordered_set<Function *> funcSet;
    queue<Function *> funcToTraverse;
    insertMyCallees(caller, funcToTraverse);

    while (!funcToTraverse.empty()) {
      auto func = funcToTraverse.front();
      funcToTraverse.pop();
      if (funcSet.count(func) > 0) {
        continue;
      }
      funcSet.insert(func);
      insertMyCallees(func, funcToTraverse);
    }

    return funcSet.count(callee) > 0;
  };

  unordered_set<Function *> hotFunctions;
  for (auto &[f, _] : ptSum->functionSummaries) {
    hotFunctions.insert(f);
  }

  bool isGlobalVarWritten = false;
  for (auto f : hotFunctions) {
    auto funcSum = ptSum->functionSummaries.at(f);
    auto funcPtGraph = funcSum->functionPointToGraph;
    for (auto storeInst : funcSum->storeInsts) {
      auto pointerOperand = ptSum->getVariable(storeInst->getPointerOperand());
      auto memObjsMayBeWritten = funcPtGraph->getPointees(pointerOperand);
      auto globalMemObj = ptSum->getMemoryObject(globalVar);
      if (memObjsMayBeWritten.count(globalMemObj) > 0) {
        isGlobalVarWritten = true;
        break;
      }
    }
  }
  if (!isGlobalVarWritten) {
    errs() << prefix << "Global variable " << globalVarName
           << " is never written, no need to privatize it.\n";
    return {};
  }

  unordered_set<Function *> privatizableFunctions;
  for (auto user : globalVar->users()) {
    if (auto inst = dyn_cast<Instruction>(user)) {
      auto userFunction = inst->getParent()->getParent();
      if (hotFunctions.count(userFunction) > 0) {
        privatizableFunctions.insert(userFunction);
      }
    }
  }

  if (privatizableFunctions.empty()) {
    return {};
  } else if (privatizableFunctions.size() == 1) {
    auto currentF = *privatizableFunctions.begin();
    auto funcSum = ptSum->functionSummaries.at(currentF);
    auto globalMemObj = ptSum->getMemoryObject(globalVar);

    if (funcSum->memoryObjectsReachableFromReturnValue().count(globalMemObj)
        > 0) {
      return {};
    } else if (mayInvoke(currentF, currentF)) {
      return {};
    } else if (!globalVariableInitializedInFunction(noelle,
                                                    ptSum,
                                                    globalVar,
                                                    currentF)) {
      return {};
    }
    return privatizableFunctions;
  } else {
    auto privatizableCandidates = privatizableFunctions;
    for (auto funcA : privatizableCandidates) {
      for (auto funcB : privatizableCandidates) {
        if (mayInvoke(funcA, funcB)) {
          return {};
        }
      }
    }

    for (auto currentF : privatizableFunctions) {
      auto funcSum = ptSum->functionSummaries.at(currentF);
      auto globalMemObj = ptSum->getMemoryObject(globalVar);

      if (funcSum->memoryObjectsReachableFromReturnValue().count(globalMemObj)
          > 0) {
        return {};
      } else if (!globalVariableInitializedInFunction(noelle,
                                                      ptSum,
                                                      globalVar,
                                                      currentF)) {
        return {};
      }
    }

    return privatizableFunctions;
  }
}

bool PrivatizerManager::globalVariableInitializedInFunction(
    Noelle &noelle,
    PointToSummary *ptSum,
    GlobalVariable *globalVar,
    Function *currentF) {
  unordered_set<StoreInst *> initCandidates;
  unordered_set<Instruction *> users;

  auto funcSum = ptSum->functionSummaries.at(currentF);
  auto funcPtGraph = funcSum->functionPointToGraph;

  for (auto storeInst : funcSum->storeInsts) {
    auto pointerOperand = ptSum->getVariable(storeInst->getPointerOperand());
    auto memObjsMayBeWritten = funcPtGraph->getPointees(pointerOperand);
    auto globalMemObj = ptSum->getMemoryObject(globalVar);
    if (memObjsMayBeWritten.count(globalMemObj) > 0) {
      initCandidates.insert(storeInst);
      users.insert(storeInst);
    }
  }

  for (auto loadInst : funcSum->loadInsts) {
    auto pointerOperand = ptSum->getVariable(loadInst->getPointerOperand());
    auto memObjsMayBeRead = funcPtGraph->getPointees(pointerOperand);
    auto globalMemObj = ptSum->getMemoryObject(globalVar);
    if (memObjsMayBeRead.count(globalMemObj) > 0) {
      users.insert(loadInst);
    }
  }

  for (auto callInst : funcSum->callInsts) {
    for (auto &arg : callInst->arg_operands()) {
      auto operand =
          ptSum->getVariable(callInst->getArgOperand(arg.getOperandNo()));
      auto memObjsMayBeAccessed = funcPtGraph->getPointees(operand);
      auto globalMemObj = ptSum->getMemoryObject(globalVar);
      if (memObjsMayBeAccessed.count(globalMemObj) > 0) {
        users.insert(callInst);
      }
    }
  }

  unordered_set<StoreInst *> storeInsts = initCandidates;

  auto initDominateAllUsers = [&](StoreInst *storeInst) {
    auto DS = noelle.getDominators(currentF);
    auto initProgramPoint =
        getProgramPointOfInitilization(noelle, globalVar, storeInst);
    if (!initProgramPoint) {
      return false;
    }

    for (auto user : users) {
      if ((storeInst == user) || DS->DT.dominates(initProgramPoint, user)) {
        continue;
      } else {
        return false;
      }
    }
    return true;
  };

  for (auto storeInst : storeInsts) {
    if (!initDominateAllUsers(storeInst)) {
      initCandidates.erase(storeInst);
    }
  }

  if (initCandidates.empty()) {
    return false;
  }

  assert(initCandidates.size() == 1);

  return true;
}

/*
 * Check whether the storeInst is used to initialize the global variable or not.
 * If so, try to find the program point where the global variable is
 * initialized. If not, return nullptr.
 */
Instruction *PrivatizerManager::getProgramPointOfInitilization(
    Noelle &noelle,
    GlobalVariable *globalVar,
    StoreInst *storeInst) {
  auto globalVarType = globalVar->getValueType();
  if (globalVarType->isSingleValueType()) {
    auto initGlobalVarDirectly = storeInst->getPointerOperand() == globalVar;
    return initGlobalVarDirectly ? storeInst : nullptr;
  }
  if (globalVar->getValueType()->isArrayTy()) {
    auto currentF = storeInst->getParent()->getParent();
    LoopDependenceInfo *LDI = nullptr;

    for (auto ldi : *noelle.getLoops(currentF)) {
      if (!ldi->getLoopStructure()->isIncluded(storeInst)) {
        continue;
      }
      if (!LDI) {
        LDI = ldi;
      } else if (LDI->getLoopStructure()->getNestingLevel()
                 < ldi->getLoopStructure()->getNestingLevel()) {
        LDI = ldi;
      }
    }
    if (!LDI) {
      return nullptr;
    }

    auto IVM = LDI->getInductionVariableManager();
    auto GIV = IVM->getLoopGoverningInductionVariable();

    if (GIV == nullptr) {
      return nullptr;
    }

    LLVMContext &C = noelle.getProgramContext();
    auto arrayType = dyn_cast<ArrayType>(globalVar->getValueType());
    auto arraySize =
        ConstantInt::get(Type::getInt32Ty(C), arrayType->getNumElements());

    auto IV = GIV->getInductionVariable();
    auto startValue = IV->getStartValue();
    auto stepValue = IV->getSingleComputedStepValue();
    auto exitConditionValue = GIV->getExitConditionValue();

    auto startFromZero = isa<ConstantInt>(startValue)
                         && dyn_cast<ConstantInt>(startValue)->isZero();
    auto stepIsOne = isa<ConstantInt>(stepValue)
                     && dyn_cast<ConstantInt>(stepValue)->isOne();
    auto exitArraySize = isa<ConstantInt>(exitConditionValue)
                         && dyn_cast<ConstantInt>(exitConditionValue)
                                ->equalsInt(arrayType->getNumElements());

    if (!(startFromZero && stepIsOne && exitArraySize)) {
      return nullptr;
    }

    auto gep = dyn_cast<GetElementPtrInst>(storeInst->getPointerOperand());
    auto gepOfGlobalVar = gep && gep->getOperand(0) == globalVar;
    auto accessByArrayIndex =
        gep && gep->getNumIndices() == 2 && isa<ConstantInt>(gep->getOperand(1))
        && dyn_cast<ConstantInt>(gep->getOperand(1))->isZero()
        && gep->getOperand(2) == *IV->getPHIs().begin();

    if (!(gepOfGlobalVar && accessByArrayIndex)) {
      return nullptr;
    }

    auto exitNodes = LDI->getLoopStructure()->getLoopExitBasicBlocks();
    if (exitNodes.size() != 1) {
      return nullptr;
    }
    auto exitNode = *exitNodes.begin();
    return exitNode->getFirstNonPHI();
  } else {
    return nullptr;
  }
};

unordered_map<GlobalVariable *, unordered_set<Function *>> PrivatizerManager::
    collectGlobalToStack(Noelle &noelle, PointToSummary *ptSum) {

  unordered_map<GlobalVariable *, unordered_set<Function *>> result;

  for (auto &G : ptSum->M.globals()) {

    auto privatizableFunctions = getPrivatizableFunctions(noelle, ptSum, &G);

    auto globalVarName = G.getName();

    if (privatizableFunctions.empty()) {
      errs() << prefix << globalVarName
             << " can't be privatized to any function.\n";
      continue;
    }

    result[&G] = privatizableFunctions;
  }

  return result;
}

bool PrivatizerManager::applyGlobalToStack(
    Noelle &noelle,
    GlobalVariable *globalVar,
    unordered_set<Function *> privatizableFunctions) {
  bool modified = false;

  for (auto currentF : privatizableFunctions) {
    auto fname = currentF->getName();
    auto suffix = " in function " + fname + "\n";
    auto globalVarName = globalVar->getName();
    errs() << prefix << "Try to privatize " << globalVarName << suffix;

    auto allocationSize = getAllocationSize(globalVar);
    if (!stackHasEnoughSpaceForNewAllocaInst(allocationSize, currentF)) {
      errs()
          << prefix << "Stack memory usage exceeds the limit, can't privatize "
          << globalVarName << suffix;
      return false;
    }

    modified = true;
    auto &context = noelle.getProgramContext();
    auto &entryBlock = currentF->getEntryBlock();
    IRBuilder<> entryBuilder(entryBlock.getFirstNonPHI());
    Type *globalVarType = globalVar->getValueType();
    AllocaInst *allocaInst =
        entryBuilder.CreateAlloca(globalVarType, nullptr, globalVarName);

    if (globalVar->hasInitializer()) {
      auto initializer = globalVar->getInitializer();
      if (isa<ConstantAggregateZero>(initializer)
          && globalVarType->isArrayTy()) {
        auto typesManager = noelle.getTypesManager();
        auto sizeInByte = typesManager->getSizeOfType(globalVarType);
        auto zeroVal = ConstantInt::get(Type::getInt8Ty(context), 0);
        entryBuilder.CreateMemSet(allocaInst, zeroVal, sizeInByte, 1);
      } else {
        entryBuilder.CreateStore(initializer, allocaInst);
      }
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
    assert(!instsToReplace.empty());
    for (auto inst : instsToReplace) {
      inst->replaceUsesOfWith(globalVar, allocaInst);
    }

    errs() << prefix << "Replace global variable " << globalVarName << "\n";
    errs() << emptyPrefix << "with allocaInst: " << *allocaInst << "\n";
  }

  return modified;
}

} // namespace llvm::noelle
